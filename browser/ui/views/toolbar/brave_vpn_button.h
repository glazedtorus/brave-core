/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_UI_VIEWS_TOOLBAR_BRAVE_VPN_BUTTON_H_
#define BRAVE_BROWSER_UI_VIEWS_TOOLBAR_BRAVE_VPN_BUTTON_H_

#include "chrome/browser/ui/views/toolbar/toolbar_button.h"
#include "ui/base/metadata/metadata_header_macros.h"
#include "chrome/browser/ui/views/bubble/webui_bubble_manager.h"
#include "brave/browser/ui/webui/brave_vpn_webui.h"
class BraveVPNButton : public ToolbarButton {
 public:
  METADATA_HEADER(BraveVPNButton);

  explicit BraveVPNButton(Profile* profile);
  ~BraveVPNButton() override;

  BraveVPNButton(const BraveVPNButton&) = delete;
  BraveVPNButton& operator=(const BraveVPNButton&) = delete;

 private:
  void UpdateColorsAndInsets() override;

  // Update button based on connection state.
  void UpdateButtonState();
  bool IsConnected();

  void OnButtonPressed(const ui::Event& event);
  void ShowBraveVPNPanel();

  WebUIBubbleManagerT<VPNPanelUI> webui_bubble_manager_;
};

#endif  // BRAVE_BROWSER_UI_VIEWS_TOOLBAR_BRAVE_VPN_BUTTON_H_
