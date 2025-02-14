/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/ads/notification_ad_util.h"

#include "bat/ads/internal/account/account_util.h"
#include "bat/ads/internal/ads_client_helper.h"
#include "bat/ads/internal/browser/browser_manager.h"
#include "bat/ads/internal/creatives/notification_ads/notification_ad_manager.h"
#include "bat/ads/internal/settings/settings.h"
#include "bat/ads/notification_ad_info.h"

namespace ads {

bool ShouldServe() {
  return ShouldRewardUser();
}

bool ShouldServeAtRegularIntervals() {
  return ShouldServe() &&
         (BrowserManager::GetInstance()->IsBrowserActive() ||
          AdsClientHelper::GetInstance()->CanShowBackgroundNotifications()) &&
         settings::GetAdsPerHour() > 0;
}

void ShowNotification(const NotificationAdInfo& ad) {
  NotificationAdManager::GetInstance()->PushBack(ad);
  AdsClientHelper::GetInstance()->ShowNotification(ad);
}

void DismissNotification(const std::string& placement_id) {
  NotificationAdManager::GetInstance()->Remove(placement_id);
}

void CloseNotification(const std::string& placement_id) {
  NotificationAdManager::GetInstance()->Remove(placement_id);
  AdsClientHelper::GetInstance()->CloseNotification(placement_id);
}

void NotificationTimedOut(const std::string& placement_id) {
  NotificationAdManager::GetInstance()->Remove(placement_id);
}

}  // namespace ads
