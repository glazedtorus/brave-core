// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#include "brave/browser/ui/webui/brave_vpn_webui.h"

#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/webui/webui_util.h"
#include "chrome/browser/ui/webui/favicon_source.h"
#include "components/favicon_base/favicon_url_parser.h"
#include "components/grit/brave_components_resources.h"
#include "brave/components/brave_vpn_panel/resources/grit/brave_vpn_panel_generated_map.h"

VPNPanelUI::VPNPanelUI(content::WebUI* web_ui)
  : ui::MojoBubbleWebUIController(web_ui, false) {
  LOG(ERROR) << "CREATED" << "\n";
  LOG(ERROR) << web_ui << "\n";

  content::WebUIDataSource* source =
    content::WebUIDataSource::Create("vpn-panel.top-chrome");

  webui::SetupWebUIDataSource(source,
                              base::make_span(kBraveVpnPanelGenerated,
                                              kBraveVpnPanelGeneratedSize),
                              IDR_VPN_PANEL_HTML);
  content::WebUIDataSource::Add(web_ui->GetWebContents()->GetBrowserContext(),
                                source);
  Profile* profile = Profile::FromWebUI(web_ui);
  content::URLDataSource::Add(
      profile, std::make_unique<FaviconSource>(
                   profile, chrome::FaviconUrlFormat::kFavicon2));
}

VPNPanelUI::~VPNPanelUI() = default;
