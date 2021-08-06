/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_UI_WEBUI_BRAVE_VPN_PANEL_UI_H_
#define BRAVE_BROWSER_UI_WEBUI_BRAVE_VPN_PANEL_UI_H_

#include "ui/webui/mojo_bubble_web_ui_controller.h"

class VPNPanelUI : public ui::MojoBubbleWebUIController {
  public:
    explicit VPNPanelUI(content::WebUI* web_ui);
    VPNPanelUI(const VPNPanelUI&) = delete;
    VPNPanelUI& operator=(const VPNPanelUI&) = delete;
    ~VPNPanelUI() override;
};

#endif  // BRAVE_BROWSER_UI_WEBUI_BRAVE_VPN_PANEL_UI_H_
