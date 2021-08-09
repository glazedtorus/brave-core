/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/eligible_ads/ad_notifications/eligible_ad_notifications.h"

#include <algorithm>
#include <string>
#include <vector>

#include "base/rand_util.h"
#include "base/time/time.h"
#include "bat/ads/ad_notification_info.h"
#include "bat/ads/internal/ad_pacing/ad_pacing.h"
#include "bat/ads/internal/ad_priority/ad_priority.h"
#include "bat/ads/internal/ad_serving/ad_targeting/geographic/subdivision/subdivision_targeting.h"
#include "bat/ads/internal/ad_targeting/ad_targeting_segment_util.h"
#include "bat/ads/internal/ad_targeting/ad_targeting_values.h"
#include "bat/ads/internal/ads/ad_notifications/ad_notification_exclusion_rules.h"
#include "bat/ads/internal/ads_client_helper.h"
#include "bat/ads/internal/client/client.h"
#include "bat/ads/internal/database/tables/ad_events_database_table.h"
#include "bat/ads/internal/database/tables/creative_ad_notifications_database_table.h"
#include "bat/ads/internal/eligible_ads/ad_notifications/candidate_ad_notification_info.h"
#include "bat/ads/internal/eligible_ads/seen_ads.h"
#include "bat/ads/internal/eligible_ads/seen_advertisers.h"
#include "bat/ads/internal/features/ad_serving/ad_serving_features.h"
#include "bat/ads/internal/logging.h"
#include "bat/ads/internal/resources/frequency_capping/anti_targeting_resource.h"

namespace ads {
namespace ad_notifications {

namespace {

bool ShouldCapLastServedAd(const CreativeAdNotificationList& ads) {
  return ads.size() != 1;
}

}  // namespace

EligibleAds::EligibleAds(
    ad_targeting::geographic::SubdivisionTargeting* subdivision_targeting,
    resource::AntiTargeting* anti_targeting_resource)
    : subdivision_targeting_(subdivision_targeting),
      anti_targeting_resource_(anti_targeting_resource) {
  DCHECK(subdivision_targeting_);
  DCHECK(anti_targeting_resource_);
}

EligibleAds::~EligibleAds() = default;

void EligibleAds::SetLastServedAd(const CreativeAdInfo& creative_ad) {
  last_served_creative_ad_ = creative_ad;
}

void EligibleAds::GetForSegments(const SegmentList& segments,
                                 GetEligibleAdsCallback callback) {
  database::table::AdEvents database_table;
  database_table.GetAll([=](const Result result, const AdEventList& ad_events) {
    if (result != Result::SUCCESS) {
      BLOG(1, "Failed to get ad events");
      callback(/* was_allowed */ false, {});
      return;
    }

    const int max_count = features::GetBrowsingHistoryMaxCount();
    const int days_ago = features::GetBrowsingHistoryDaysAgo();
    AdsClientHelper::Get()->GetBrowsingHistory(
        max_count, days_ago, [=](const BrowsingHistoryList& history) {
          if (segments.empty()) {
            GetForUntargeted(ad_events, history, callback);
            return;
          }

          GetForParentChildSegments(segments, ad_events, history, callback);
        });
  });
}

void EligibleAds::Get(const SegmentList& interest_segments,
                      const SegmentList& intent_segments,
                      GetEligibleAdsCallback callback) {
  database::table::AdEvents database_table;
  database_table.GetAll([=](const Result result, const AdEventList& ad_events) {
    if (result != Result::SUCCESS) {
      BLOG(1, "Failed to get ad events");
      callback(/* was_allowed */ false, {});
      return;
    }

    const int max_count = features::GetBrowsingHistoryMaxCount();
    const int days_ago = features::GetBrowsingHistoryDaysAgo();
    AdsClientHelper::Get()->GetBrowsingHistory(
        max_count, days_ago, [=](const BrowsingHistoryList& history) {
          GetEligibleAds(interest_segments, intent_segments, ad_events,
                         history, callback);
        });
  });
}

///////////////////////////////////////////////////////////////////////////////

void EligibleAds::GetEligibleAds(
    const SegmentList& interest_segments,
    const SegmentList& intent_segments,
    const AdEventList& ad_events,
    const BrowsingHistoryList& browsing_history,
    GetEligibleAdsCallback callback) const {
  BLOG(1, "Get eligible ads");

  database::table::CreativeAdNotifications database_table;
  database_table.GetAll(
      [=](const Result result, const SegmentList& segments,
          const CreativeAdNotificationList& ads) {
        if (ads.empty()) {
          BLOG(1, "No ads");
          // TODO(Moritz Haller): Still need to callback to fail in serving.cc
          callback(/* was_allowed */ true, {});
        }

        CreativeAdNotificationList eligible_ads = ApplyFrequencyCapping(ads,
            ShouldCapLastServedAd(ads) ? last_served_creative_ad_ : CreativeAdInfo(),  // TODO(Moritz Haller): Keep?  NOLINT
            ad_events, browsing_history);

        if (eligible_ads.empty()) {
          BLOG(1, "No eligible ads");
          // TODO(Moritz Haller): Still need to callback to fail in serving.cc
          callback(/* was_allowed */ true, eligible_ads);
        }

        SampleFromEligibleAds(eligible_ads, ad_events, interest_segments,
            intent_segments, callback);
      });
}

void EligibleAds::SampleFromEligibleAds(
    const CreativeAdNotificationList& eligible_ads,
    const AdEventList& ad_events,
    const SegmentList& interest_segments,
    const SegmentList& intent_segments,
    GetEligibleAdsCallback callback) const {
  DCHECK(!eligible_ads.empty());

  // TODO(Moritz Haller): noop - ad_events are ordered desc

  CandidateAdNotifications candidate_ad_notifications;
  double Z = 0.0;

  for (const auto& eligible_ad : eligible_ads) {
    const auto iter = candidate_ad_notifications.find(
        eligible_ad.creative_instance_id);
    // eligible ads are "fan-out" in DB, i.e. multiple rows for the same
    // creative ad notification, one for each segment. Just append segment if
    // encountered before
    if (iter != candidate_ad_notifications.end()) {
      iter->second.segments.push_back(eligible_ad.segment);

      SegmentList interest_parent_segments =
          GetParentSegments(interest_segments);
      SegmentList intent_parent_segments =
          GetParentSegments(intent_segments);

      // TODO(Moritz Haller): else if or plain ifs?
      if (std::find(interest_segments.begin(), interest_segments.end(),
          eligible_ad.segment) != interest_segments.end()) {
        iter->second.matches_interest_child_segment = true;
      } else if (std::find(interest_parent_segments.begin(),
          interest_parent_segments.end(), eligible_ad.segment) !=
              interest_parent_segments.end()) {
        iter->second.matches_interest_parent_segment = true;
      } else if (std::find(intent_segments.begin(), intent_segments.end(),
          eligible_ad.segment) != intent_segments.end()) {
        iter->second.matches_intent_child_segment = true;
      } else if (std::find(intent_parent_segments.begin(),
          intent_parent_segments.end(), eligible_ad.segment) !=
              intent_parent_segments.end()) {
        iter->second.matches_intent_parent_segment = true;
      }

      continue;
    }

    CandidateAdNotificationInfo candidate_ad_notification;
    candidate_ad_notification.creative_instance_id =
        eligible_ad.creative_instance_id;
    candidate_ad_notification.advertiser_id =
        eligible_ad.advertiser_id;
    candidate_ad_notification.priority = eligible_ad.priority;
    candidate_ad_notification.ptr = eligible_ad.ptr;
    candidate_ad_notification.segments = { eligible_ad.segment };
    candidate_ad_notification.creative_ad_notification = eligible_ad;

    const base::Time now = base::Time::Now();

    const auto iter2 = std::find_if(ad_events.begin(), ad_events.end(),
      [&candidate_ad_notification](const AdEventInfo& ad_event) -> bool {
        return (ad_event.creative_instance_id ==
            candidate_ad_notification.creative_instance_id &&
          ad_event.confirmation_type == ConfirmationType::kViewed);
      });

    if (iter2 != ad_events.end()) {
      const base::Time timestamp = base::Time::FromDoubleT(iter2->timestamp);
      candidate_ad_notification.ad_last_seen_in_hours =
          (now - timestamp).InHours();
    }

    const auto iter3 = std::find_if(ad_events.begin(), ad_events.end(),
      [&candidate_ad_notification](const AdEventInfo& ad_event) -> bool {
        return (ad_event.advertiser_id ==
            candidate_ad_notification.advertiser_id &&
          ad_event.confirmation_type == ConfirmationType::kViewed);
      });

    if (iter3 != ad_events.end()) {
      const base::Time timestamp2 = base::Time::FromDoubleT(iter3->timestamp);
      candidate_ad_notification.advertiser_last_seen_in_hours =
        (now - timestamp2).InHours();
    }

    // TODO(Moritz Haller): weights via griffin
    std::vector<double> weights = {1, 1, 1, 1, 99, 1, 1};

    double phi = 0;
    if (candidate_ad_notification.matches_intent_child_segment) {
      phi += weights[0];
    } else if (candidate_ad_notification.matches_intent_parent_segment) {
      phi += weights[1];
    } else if (candidate_ad_notification.matches_interest_child_segment) {
      phi += weights[2];
    } else if (candidate_ad_notification.matches_interest_parent_segment) {
      phi += weights[3];
    }

    phi += (candidate_ad_notification.ad_last_seen_in_hours > 24) ? 0 :
        weights[4]*candidate_ad_notification.ad_last_seen_in_hours/24;

    phi += (candidate_ad_notification.advertiser_last_seen_in_hours > 24) ? 0 :
        weights[5]*candidate_ad_notification.advertiser_last_seen_in_hours/24;

    phi += weights[6]*1 / candidate_ad_notification.priority;

    phi *= candidate_ad_notification.ptr;
    Z += candidate_ad_notification.phi;

    candidate_ad_notification.phi = phi;

    candidate_ad_notifications.insert({
      candidate_ad_notification.creative_instance_id, candidate_ad_notification
    });
  }

  const double rand = base::RandDouble();
  double sum = 0;
  for (const auto& candidate : candidate_ad_notifications) {
    double probability = candidate.second.phi / Z;
    sum += probability;

    if (rand < sum) {
        CreativeAdNotificationList sampled_ads;
        sampled_ads.push_back(candidate.second.creative_ad_notification);
        callback(/* was_allowed */ true, sampled_ads);
    }
  }

  // TODO(Moritz Haller): defensive code, do we need?
  callback(/* was_allowed */ true, {});
}

void EligibleAds::GetForParentChildSegments(
    const SegmentList& segments,
    const AdEventList& ad_events,
    const BrowsingHistoryList& browsing_history,
    GetEligibleAdsCallback callback) const {
  DCHECK(!segments.empty());

  BLOG(1, "Get eligible ads for parent-child segments:");
  for (const auto& segment : segments) {
    BLOG(1, "  " << segment);
  }

  database::table::CreativeAdNotifications database_table;
  database_table.GetForSegments(
      segments, [=](const Result result, const SegmentList& segments,
                    const CreativeAdNotificationList& ads) {
        CreativeAdNotificationList eligible_ads =
            FilterIneligibleAds(ads, ad_events, browsing_history);

        if (eligible_ads.empty()) {
          BLOG(1, "No eligible ads for parent-child segments");
          GetForParentSegments(segments, ad_events, browsing_history, callback);
          return;
        }

        callback(/* was_allowed */ true, eligible_ads);
      });
}

void EligibleAds::GetForParentSegments(
    const SegmentList& segments,
    const AdEventList& ad_events,
    const BrowsingHistoryList& browsing_history,
    GetEligibleAdsCallback callback) const {
  DCHECK(!segments.empty());

  const SegmentList parent_segments = GetParentSegments(segments);
  if (parent_segments == segments) {
    callback(/* was_allowed */ false, {});
    return;
  }

  BLOG(1, "Get eligible ads for parent segments:");
  for (const auto& parent_segment : parent_segments) {
    BLOG(1, "  " << parent_segment);
  }

  database::table::CreativeAdNotifications database_table;
  database_table.GetForSegments(
      parent_segments, [=](const Result result, const SegmentList& segments,
                           const CreativeAdNotificationList& ads) {
        CreativeAdNotificationList eligible_ads =
            FilterIneligibleAds(ads, ad_events, browsing_history);

        if (eligible_ads.empty()) {
          BLOG(1, "No eligible ads for parent segments");
          GetForUntargeted(ad_events, browsing_history, callback);
          return;
        }

        callback(/* was_allowed */ true, eligible_ads);
      });
}

void EligibleAds::GetForUntargeted(const AdEventList& ad_events,
                                   const BrowsingHistoryList& browsing_history,
                                   GetEligibleAdsCallback callback) const {
  BLOG(1, "Get eligble ads for untargeted segment");

  const std::vector<std::string> segments = {ad_targeting::kUntargeted};

  database::table::CreativeAdNotifications database_table;
  database_table.GetForSegments(
      segments, [=](const Result result, const SegmentList& segments,
                    const CreativeAdNotificationList& ads) {
        CreativeAdNotificationList eligible_ads =
            FilterIneligibleAds(ads, ad_events, browsing_history);

        if (eligible_ads.empty()) {
          BLOG(1, "No eligible ads for untargeted segment");
        }

        callback(/* was_allowed */ true, eligible_ads);
      });
}

CreativeAdNotificationList EligibleAds::FilterIneligibleAds(
    const CreativeAdNotificationList& ads,
    const AdEventList& ad_events,
    const BrowsingHistoryList& browsing_history) const {
  if (ads.empty()) {
    return {};
  }

  CreativeAdNotificationList eligible_ads = ads;

  eligible_ads = FilterSeenAdvertisersAndRoundRobinIfNeeded(
      eligible_ads, AdType::kAdNotification);

  eligible_ads =
      FilterSeenAdsAndRoundRobinIfNeeded(eligible_ads, AdType::kAdNotification);

  eligible_ads = ApplyFrequencyCapping(
      eligible_ads,
      ShouldCapLastServedAd(ads) ? last_served_creative_ad_ : CreativeAdInfo(),
      ad_events, browsing_history);

  eligible_ads = PaceAds(eligible_ads);

  eligible_ads = PrioritizeAds(eligible_ads);

  return eligible_ads;
}

CreativeAdNotificationList EligibleAds::ApplyFrequencyCapping(
    const CreativeAdNotificationList& ads,
    const CreativeAdInfo& last_served_creative_ad,
    const AdEventList& ad_events,
    const BrowsingHistoryList& browsing_history) const {
  CreativeAdNotificationList eligible_ads = ads;

  frequency_capping::ExclusionRules exclusion_rules(
      subdivision_targeting_, anti_targeting_resource_, ad_events,
      browsing_history);

  const auto iter = std::remove_if(
      eligible_ads.begin(), eligible_ads.end(),
      [&exclusion_rules, &last_served_creative_ad](CreativeAdInfo& ad) {
        return exclusion_rules.ShouldExcludeAd(ad) ||
               ad.creative_instance_id ==
                   last_served_creative_ad.creative_instance_id;
      });

  eligible_ads.erase(iter, eligible_ads.end());

  return eligible_ads;
}

}  // namespace ad_notifications
}  // namespace ads
