

#include <engine/textrender.h>
#include <engine/shared/config.h>
#include <generated/protocol.h>
#include <generated/game_data.h>

#include <game/client/customstuff.h>
#include <game/client/customstuff/playerinfo.h>

#include <game/client/gameclient.h>
#include "nameplates.h"
#include "controls.h"

void CNamePlates::RenderNameplate(
	const CNetObj_Character *pPrevChar,
	const CNetObj_Character *pPlayerChar,
	const CNetObj_PlayerInfo *pPlayerInfo
	)
{
	float IntraTick = Client()->IntraGameTick();

	vec2 Position = mix(vec2(pPrevChar->m_X, pPrevChar->m_Y), vec2(pPlayerChar->m_X, pPlayerChar->m_Y), IntraTick);
	Position.y -= 36;

	float FontSize = 18.0f + 20.0f * g_Config.m_ClNameplatesSize / 100.0f;
	
	// skip invisible players
	CPlayerInfo *pCustomPlayerInfo = &CustomStuff()->m_aPlayerInfo[pPlayerInfo->m_ClientID];
	
	float v = pCustomPlayerInfo->m_EffectIntensity[EFFECT_INVISIBILITY];
	
	if ((CustomStuff()->m_LocalTeam == pPlayerInfo->m_Team && m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_TEAMS) ||
		CustomStuff()->m_LocalTeam == TEAM_SPECTATORS)
		v = 0.0f;

	if (pPlayerInfo->m_Team == 0 && m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_COOP)
		v = 0.0f;
	
		
	//if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_INVISIBILITY] > 0.4f)
	//	return;
	
	// render name plate
	if(!pPlayerInfo->m_Local && (!CustomStuff()->IsBot(pPlayerInfo->m_ClientID) || !(m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_COOP)))
	{
		float a = 1;
		if(g_Config.m_ClNameplatesAlways == 0)
			a = clamp(1-powf(distance(m_pClient->m_pControls->m_TargetPos, Position)/200.0f,16.0f), 0.0f, 1.0f);

		const char *pName = m_pClient->m_aClients[pPlayerInfo->m_ClientID].m_aName;
		float tw = TextRender()->TextWidth(0, FontSize, pName, -1);

		a *= 1.0f - v;
		
		TextRender()->TextOutlineColor(0.0f, 0.0f, 0.0f, 0.5f*a);
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, a);
		
		if(g_Config.m_ClNameplatesTeamcolors && m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_TEAMS)
		{
			if (m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_INFECTION)
			{
				if(pPlayerInfo->m_Team == TEAM_RED)
					TextRender()->TextColor(255/255.0f, 200/255.0f, 200/255.0f, a);
				else if(pPlayerInfo->m_Team == TEAM_BLUE)
					TextRender()->TextColor(66/255.0f, 66/255.0f, 66/255.0f, a);
			}
			else
			{		
				if(pPlayerInfo->m_Team == TEAM_RED)
					TextRender()->TextColor(250/255.0f, 100/255.0f, 0, a);
				else if(pPlayerInfo->m_Team == TEAM_BLUE)
					TextRender()->TextColor(0/255.0f, 100/255.0f, 230/255.0f, a);
			}
		}

		TextRender()->Text(0, Position.x-tw/2.0f, Position.y-FontSize-38.0f, FontSize, pName, -1);

		if(g_Config.m_Debug) // render client id when in debug aswell
		{
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf),"%d", pPlayerInfo->m_ClientID);
			TextRender()->Text(0, Position.x, Position.y-90, 28.0f, aBuf, -1);
		}

		TextRender()->TextColor(1,1,1,1);
		TextRender()->TextOutlineColor(0.0f, 0.0f, 0.0f, 0.3f);
	}
	else
		CustomStuff()->m_LocalTeam = pPlayerInfo->m_Team;
}

void CNamePlates::OnRender()
{
	if (!g_Config.m_ClNameplates)
		return;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		// only render active characters
		if(!m_pClient->m_Snap.m_aCharacters[i].m_Active)
			continue;

		const void *pInfo = Client()->SnapFindItem(IClient::SNAP_CURRENT, NETOBJTYPE_PLAYERINFO, i);

		if(pInfo)
		{
			RenderNameplate(
				&m_pClient->m_Snap.m_aCharacters[i].m_Prev,
				&m_pClient->m_Snap.m_aCharacters[i].m_Cur,
				(const CNetObj_PlayerInfo *)pInfo);
		}
	}
}
