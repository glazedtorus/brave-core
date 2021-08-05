// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import { WalletAPIHandler, APIProxyControllers } from '../constants/types'

export default class APIProxy implements APIProxyControllers {
  static getInstance: () => APIProxy
  showUI: () => {}
  closeUI: () => {}
  connectToSite: (accounts: string[], origin: string, tabId: number) => {}
  cancelConnectToSite: (origin: string, tabId: number) => {}
  walletHandler: WalletAPIHandler
  ethJsonRpcController: EthJsonRpcController
  swapController: SwapController
  assetRatioController: AssetRatioController
  keyringController: KeyringController
}
