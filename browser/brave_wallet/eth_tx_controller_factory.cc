/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/brave_wallet/eth_tx_controller_factory.h"

#include <memory>
#include <utility>

#include "brave/browser/brave_wallet/brave_wallet_context_utils.h"
#include "brave/components/brave_wallet/browser/brave_wallet_constants.h"
#include "brave/components/brave_wallet/browser/eth_tx_controller.h"
#include "chrome/browser/profiles/incognito_helpers.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "content/public/browser/storage_partition.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"

namespace brave_wallet {

// static
EthTxControllerFactory* EthTxControllerFactory::GetInstance() {
  return base::Singleton<EthTxControllerFactory>::get();
}

// static
mojo::PendingRemote<mojom::EthTxController>
EthTxControllerFactory::GetForContext(content::BrowserContext* context) {
  if (!IsAllowedForContext(context)) {
    return mojo::PendingRemote<mojom::EthTxController>();
  }

  return static_cast<EthTxController*>(
             GetInstance()->GetServiceForBrowserContext(context, true))
      ->MakeRemote();
}

// static
EthTxController* EthTxControllerFactory::GetControllerForContext(
    content::BrowserContext* context) {
  if (!IsAllowedForContext(context)) {
    return nullptr;
  }
  return static_cast<EthTxController*>(
      GetInstance()->GetServiceForBrowserContext(context, true));
}

EthTxControllerFactory::EthTxControllerFactory()
    : BrowserContextKeyedServiceFactory(
          "EthTxController",
          BrowserContextDependencyManager::GetInstance()) {}

EthTxControllerFactory::~EthTxControllerFactory() {}

KeyedService* EthTxControllerFactory::BuildServiceInstanceFor(
    content::BrowserContext* context) const {
  auto eth_nonce_tracker = std::make_unique<EthNonceTracker>(
      tx_state_manager_.get(), rpc_controller_);
  auto eth_pending_tx_tracker = std::make_unique<EthPendingTxTracker>(
      tx_state_manager_.get(),
      std::move(
          brave_wallet::EthJsonRpcController::GetInstance()->GetForContext(
              context)),
      eth_nonce_tracker.get());
  return new EthTxController(
      std::move(
          brave_wallet::EthJsonRpcController::GetInstance()->GetForContext(
              context)),
      std::move(brave_wallet::KeyringController::GetInstance()->GetForContext(
          context)),
      std::move(eth_nonce_tracker), std::move(eth_pending_tx_tracker),
      user_prefs::UserPrefs::Get(context));
}

content::BrowserContext* EthTxControllerFactory::GetBrowserContextToUse(
    content::BrowserContext* context) const {
  return chrome::GetBrowserContextRedirectedInIncognito(context);
}

}  // namespace brave_wallet
