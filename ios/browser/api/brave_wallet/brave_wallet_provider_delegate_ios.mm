/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/ios/browser/api/brave_wallet/brave_wallet_provider_delegate_ios.h"

#include "base/strings/sys_string_conversions.h"
#include "brave/base/mac/conversions.h"
#include "brave/components/brave_wallet/common/brave_wallet.mojom.h"
#include "brave/ios/browser/api/brave_wallet/brave_wallet_provider_delegate_ios+private.h"
#include "brave/ios/browser/api/url/url_origin_ios+private.h"
#include "net/base/mac/url_conversions.h"

namespace brave_wallet {

bool BraveWalletProviderDelegateBridge::IsTabVisible() {
  return [bridge_ isTabVisible];
}

void BraveWalletProviderDelegateBridge::ShowPanel() {
  [bridge_ showPanel];
}

void BraveWalletProviderDelegateBridge::WalletInteractionDetected() {
  [bridge_ walletInteractionDetected];
}

url::Origin BraveWalletProviderDelegateBridge::GetOrigin() const {
  return url::Origin([[bridge_ getOrigin] underlyingOrigin]);
}

void BraveWalletProviderDelegateBridge::ShowWalletOnboarding() {
  [bridge_ showWalletOnboarding];
}

void BraveWalletProviderDelegateBridge::RequestPermissions(
    mojom::CoinType type,
    const std::vector<std::string>& accounts,
    RequestPermissionsCallback callback) {
  auto completion = [callback = std::make_shared<decltype(callback)>(std::move(
                         callback))](BraveWalletRequestPermissionsError error,
                                     NSArray<NSString*>* _Nullable results) {
    if (!callback) {
      return;
    }
    if (results == nil) {
      std::move(*callback).Run(
          static_cast<mojom::RequestPermissionsError>(error), absl::nullopt);
      return;
    }
    std::vector<std::string> v;
    for (NSString* result in results) {
      v.push_back(base::SysNSStringToUTF8(result));
    }
    std::move(*callback).Run(static_cast<mojom::RequestPermissionsError>(error),
                             v);
  };
  [bridge_ requestPermissions:static_cast<BraveWalletCoinType>(type)
                     accounts:brave::vector_to_ns(accounts)
                   completion:completion];
}

void BraveWalletProviderDelegateBridge::IsAccountAllowed(
    mojom::CoinType type,
    const std::string& account,
    IsAccountAllowedCallback callback) {
  auto completion = [callback = std::make_shared<decltype(callback)>(
                         std::move(callback))](bool allowed) {
    if (!callback) {
      return;
    }
    std::move(*callback).Run(allowed);
  };
  [bridge_ isAccountAllowed:static_cast<BraveWalletCoinType>(type)
                    account:base::SysUTF8ToNSString(account)
                 completion:completion];
}

void BraveWalletProviderDelegateBridge::GetAllowedAccounts(
    mojom::CoinType type,
    const std::vector<std::string>& accounts,
    GetAllowedAccountsCallback callback) {
  auto completion =
      [callback = std::make_shared<decltype(callback)>(std::move(callback))](
          bool success, NSArray<NSString*>* results) {
        if (!callback) {
          return;
        }
        std::vector<std::string> v;
        for (NSString* result in results) {
          v.push_back(base::SysNSStringToUTF8(result));
        }
        std::move(*callback).Run(success, v);
      };
  [bridge_ getAllowedAccounts:static_cast<BraveWalletCoinType>(type)
                     accounts:brave::vector_to_ns(accounts)
                   completion:completion];
}

}  // namespace brave_wallet
