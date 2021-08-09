/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ELIGIBLE_ADS_AD_NOTIFICATIONS_CANDIDATE_AD_NOTIFICATION_INFO
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ELIGIBLE_ADS_AD_NOTIFICATIONS_CANDIDATE_AD_NOTIFICATION_INFO

#include <cstdint>
#include <map>
#include <string>

#include "bat/ads/internal/ad_targeting/ad_targeting_segment.h"
#include "bat/ads/internal/bundle/creative_ad_notification_info.h"

namespace ads {
namespace ad_notifications {

// TODO(Moritz Haller): Name, maybe even `CandidateInfo`?
struct CandidateAdNotificationInfo {
  CandidateAdNotificationInfo();
  CandidateAdNotificationInfo(const CandidateAdNotificationInfo& info);
  ~CandidateAdNotificationInfo();

  std::string creative_instance_id;
  std::string advertiser_id;
  unsigned int priority = 0;
  double ptr = 0.0;
  SegmentList segments;
  // TODO(Moritz Haller): ptr instead of copy?
  CreativeAdNotificationInfo creative_ad_notification;
  bool matches_interest_child_segment = 0;
  bool matches_interest_parent_segment = 0;
  bool matches_intent_child_segment = 0;
  bool matches_intent_parent_segment = 0;
  unsigned int ad_last_seen_in_hours = 0;
  unsigned int advertiser_last_seen_in_hours = 0;
  double phi = 0.0;
};

using CandidateAdNotifications =
    std::map<std::string, CandidateAdNotificationInfo>;

}  // namespace ad_notifications
}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ELIGIBLE_ADS_AD_NOTIFICATIONS_CANDIDATE_AD_NOTIFICATION_INFO
