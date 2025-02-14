/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ADS_NOTIFICATION_AD_UTIL_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ADS_NOTIFICATION_AD_UTIL_H_

#include <string>

namespace ads {

struct NotificationAdInfo;

bool ShouldServe();
bool ShouldServeAtRegularIntervals();

void ShowNotification(const NotificationAdInfo& ad);
void CloseNotification(const std::string& placement_id);
void DismissNotification(const std::string& placement_id);
void NotificationTimedOut(const std::string& placement_id);

}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ADS_NOTIFICATION_AD_UTIL_H_
