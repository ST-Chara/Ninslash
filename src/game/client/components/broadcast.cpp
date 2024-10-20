

#include <engine/shared/config.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <generated/protocol.h>
#include <generated/game_data.h>

#include <game/client/gameclient.h>

#include <game/client/components/motd.h>
#include <game/client/components/scoreboard.h>


#include <game/client/customstuff.h>
#include <game/client/customstuff/playerinfo.h>

#include "broadcast.h"

void CBroadcast::OnReset()
{
	m_BroadcastTime = 0;
}

void CBroadcast::OnRender()
{
	if(m_pClient->m_pScoreboard->Active() || m_pClient->m_pMotd->IsActive())
		return;

	Graphics()->MapScreen(0, 0, 300*Graphics()->ScreenAspect(), 300);

	if(time_get() < m_BroadcastTime)
	{
		CTextCursor Cursor;
		TextRender()->SetCursor(&Cursor, m_BroadcastRenderOffset, 40.0f, 12.0f, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
		Cursor.m_LineWidth = 300*Graphics()->ScreenAspect()-m_BroadcastRenderOffset;
		TextRender()->TextEx(&Cursor, m_aBroadcastText, -1);
	}
}

void CBroadcast::OnMessage(int MsgType, void *pRawMsg)
{
	if(MsgType == NETMSGTYPE_SV_BROADCAST)
	{
		CNetMsg_Sv_Broadcast *pMsg = (CNetMsg_Sv_Broadcast *)pRawMsg;
		str_copy(m_aBroadcastText, pMsg->m_pMessage, sizeof(m_aBroadcastText));
		CTextCursor Cursor;
		TextRender()->SetCursor(&Cursor, 0, 0, 12.0f, TEXTFLAG_STOP_AT_END);
		Cursor.m_LineWidth = 300*Graphics()->ScreenAspect();
		TextRender()->TextEx(&Cursor, m_aBroadcastText, -1);
		m_BroadcastRenderOffset = 150*Graphics()->ScreenAspect()-Cursor.m_X/2;
		m_BroadcastTime = time_get()+time_freq()*10;
		
		if (m_aBroadcastText[0] == 'D' && m_aBroadcastText[1] == 'i' && m_aBroadcastText[2] == 's' && m_aBroadcastText[3] == 'a')
			m_BroadcastTime = time_get()+time_freq()*1.5f;
		if (m_aBroadcastText[0] == 'A' && m_aBroadcastText[1] == 'r' && m_aBroadcastText[2] == 'm' && m_aBroadcastText[3] == 'i')
			m_BroadcastTime = time_get()+time_freq()*1.5f;
	}
}

