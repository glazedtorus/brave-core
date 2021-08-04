// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import { AppObjectType, AccountInfo } from '../../constants/types'

export type InitializedPayloadType = {
  isWalletCreated: boolean
  isWalletLocked: boolean
  favoriteApps: AppObjectType[]
  isWalletBackedUp: boolean
  accountInfos: AccountInfo[]
}

export type UnlockWalletPayloadType = {
  password: string
}

export type ChainChangedEventPayloadType = {
  chainId: string
}
