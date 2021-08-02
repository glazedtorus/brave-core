/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import * as React from 'react'
import { bindActionCreators, Dispatch } from 'redux'
import { connect } from 'react-redux'

import { MainButton } from '../../../shared/components/onboarding/main_button'
import { TermsOfService } from '../../../shared/components/terms_of_service'
import * as style from './ads_enable.style'

import { CheckCircleIcon } from '../../../shared/components/icons/check_circle'
import { RewardsOnboardingCashbackIcon } from '../../../shared/components/icons/rewards_onboarding_cashback'

import * as rewardsPanelActions from '../actions/rewards_panel_actions'
import { getMessage } from '../background/api/locale_api'

interface Props extends RewardsExtension.ComponentProps {
}

interface State {
  showStartFreeCall: boolean
}

export class AdsEnablePanel extends React.Component<Props, State> {
  constructor (props: Props) {
    super(props)
    this.state = {
      showStartFreeCall: false
    }
  }

  componentDidMount () {
    chrome.braveRewards.isInitialized((initialized: boolean) => {
      if (initialized) {
        this.props.actions.initialized()
        this.startAdsEnable()
      }
    })
  }

  componentDidUpdate (prevProps: Props, prevState: State) {
    if (prevProps.rewardsPanelData.initializing &&
      !this.props.rewardsPanelData.initializing) {
      this.startAdsEnable()
    }
  }

  startAdsEnable = () => {
    chrome.braveRewards.getPrefs((prefs) => {
      this.actions.onGetPrefs(prefs)
    })

    chrome.braveRewards.getRewardsParameters((parameters: RewardsExtension.RewardsParameters) => {
      rewardsPanelActions.onRewardsParameters(parameters)
    })
  }

  get actions () {
    return this.props.actions
  }

  render () {
    const onEnable = () => {
      chrome.braveRewards.enableRewards()

      chrome.braveRewards.getPrefs((prefs) => {
        this.actions.onGetPrefs(prefs)
      })

      this.setState({ showStartFreeCall: true })
    }

    if (this.state.showStartFreeCall) {
      return (
        <style.root>
          <CheckCircleIcon />
          <style.header>
            {getMessage('adsEnableCanStartFreeCall')}
          </style.header>
          <style.text>
            {getMessage('adsEnableClickAnywhereToBraveTalk')}
          </style.text>
          <style.terms>
            <TermsOfService />
          </style.terms>
        </style.root>
      )
    }

    return (
      <style.root>
        <RewardsOnboardingCashbackIcon />
        <style.header>
          {getMessage('adsEnableTurnOnRewardsToStartCall')}
        </style.header>
        <style.text>
          {getMessage('adsEnableBraveRewardsDescription')}
        </style.text>
        <style.enable>
          <MainButton onClick={onEnable}>
            {getMessage('adsEnableTurnOnRewards')}
          </MainButton>
        </style.enable>
        <style.terms>
          <TermsOfService />
        </style.terms>
      </style.root>
    )
  }
}

export const mapStateToProps = (state: RewardsExtension.ApplicationState) => ({
  rewardsPanelData: state.rewardsPanelData
})

export const mapDispatchToProps = (dispatch: Dispatch) => ({
  actions: bindActionCreators(rewardsPanelActions, dispatch)
})

export default connect(
  mapStateToProps,
  mapDispatchToProps
)(AdsEnablePanel)
