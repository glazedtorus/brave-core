// Copyright (c) 2020 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#include "brave/components/ntp_background_images/browser/view_counter_model.h"

#include "base/check_op.h"
#include "base/logging.h"

namespace ntp_background_images {

ViewCounterModel::ViewCounterModel() {
  Reset();
}

ViewCounterModel::~ViewCounterModel() = default;

bool ViewCounterModel::ShouldShowBrandedWallpaper() const {
  if (ignore_count_to_branded_wallpaper_)
    return true;

  return count_to_branded_wallpaper_ == 0;
}

void ViewCounterModel::ResetCurrentWallpaperImageIndex() {
  current_wallpaper_image_index_ = 0;
  current_branded_wallpaper_image_index_ = 0;
}

void ViewCounterModel::RegisterPageView() {
  // Do not add  DCHECK_NE for total_background_image_count_ here. RegisterPageView can happen before BI finished downloading.
  DCHECK_NE(-1, total_image_count_);
  DCHECK_NE(-1, total_branded_image_count_);

  if (ignore_count_to_branded_wallpaper_) {
    current_branded_wallpaper_image_index_++;
    current_branded_wallpaper_image_index_ %= total_branded_image_count_;
    return;
  }

  // When count is `0` then UI is free to show
  // the branded wallpaper, until the next time `RegisterPageView`
  // is called.
  // We select the appropriate image index for the scheduled
  // view of the branded wallpaper.
  count_to_branded_wallpaper_--;
  if (count_to_branded_wallpaper_ < 0) {
    // Reset count and increse image index for next time.
    count_to_branded_wallpaper_ = kRegularCountToBrandedWallpaper;
    current_branded_wallpaper_image_index_++;
    current_branded_wallpaper_image_index_ %= total_branded_image_count_;
  } else {
    // Increase background image index
    current_wallpaper_image_index_++;
    current_wallpaper_image_index_ %= total_image_count_;
  }
}

void ViewCounterModel::Reset(bool use_initial_count) {
  count_to_branded_wallpaper_ =
      use_initial_count ? kInitialCountToBrandedWallpaper
                        : kRegularCountToBrandedWallpaper;
  current_wallpaper_image_index_ = 0;
  current_branded_wallpaper_image_index_ = 0;
  total_image_count_ = -1;
  total_branded_image_count_ = -1;
  ignore_count_to_branded_wallpaper_ = false;
}

}  // namespace ntp_background_images
