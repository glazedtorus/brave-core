import * as React from 'react'
import * as S from './style'

interface Props {
 isConnected: boolean
}

function StatusIndicator (props: Props) {
  return (
    <S.Box>
      <S.Indicator isActive={props.isConnected} />
      <S.Text>{props.isConnected ? 'Connected' : 'Disconnected'}</S.Text>
    </S.Box>
  )
}

export default StatusIndicator
