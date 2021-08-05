import * as React from 'react'
import * as S from './style'
import StatusIndicator from '../status-indicator'
import Toggle from '../toggle'

function Main () {
  const [isOn, setOn] = React.useState(true);
  const handleClick = () => setOn(state => !state);

  return (
    <S.Box>
      <S.PanelTitle>
        Brave Firewall + VPN
      </S.PanelTitle>
      <S.ToggleBox>
        <Toggle
          on={isOn}
          onClick={handleClick}
        />
      </S.ToggleBox>
      <S.StatusIndicatorBox>
        <StatusIndicator
          isConnected={isOn}
        />
      </S.StatusIndicatorBox>
      <S.Btn>
        <S.BtnText>United States</S.BtnText>
      </S.Btn>
    </S.Box>
  )
}

export default Main
