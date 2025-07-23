


#include "gameworld.h"
#include "entity.h"
#include "gamecontext.h"
#include "entities/turret.h"
#include "entities/weapon.h"
#include "entities/ball.h"
#include "entities/building.h"
#include "entities/droid.h"

#include <game/weapons.h>
#include <engine/shared/config.h>

//////////////////////////////////////////////////
// game world
//////////////////////////////////////////////////
CGameWorld::CGameWorld()
{
	m_pGameServer = 0x0;
	m_pServer = 0x0;

	m_Paused = false;
	m_ResetRequested = false;
	for(int i = 0; i < NUM_ENTTYPES; i++)
		m_apFirstEntityTypes[i] = 0;
}

CGameWorld::~CGameWorld()
{
	// delete all entities
	for(int i = 0; i < NUM_ENTTYPES; i++)
		while(m_apFirstEntityTypes[i])
			delete m_apFirstEntityTypes[i];
}

void CGameWorld::SetGameServer(CGameContext *pGameServer)
{
	m_pGameServer = pGameServer;
	m_pServer = m_pGameServer->Server();
}

CEntity *CGameWorld::FindFirst(int Type)
{
	return Type < 0 || Type >= NUM_ENTTYPES ? 0 : m_apFirstEntityTypes[Type];
}

int CGameWorld::FindEntities(vec2 Pos, float Radius, CEntity **ppEnts, int Max, int Type)
{
	if(Type < 0 || Type >= NUM_ENTTYPES)
		return 0;

	// todo: scalable
	vec2 OPos = Pos;
	
	int Num = 0;
	for(CEntity *pEnt = m_apFirstEntityTypes[Type];	pEnt; pEnt = pEnt->m_pNextTypeEntity)
	{
		if (Type == CGameWorld::ENTTYPE_DROID)
			Pos = OPos + pEnt->m_Center;
		
		// circle body collision
		if(Radius <= 0.0f || distance(pEnt->m_Pos, Pos) < Radius+pEnt->m_ProximityRadius || 
			// head collision if character
			(Type == CGameWorld::ENTTYPE_CHARACTER && distance(pEnt->m_Pos + vec2(0, -27), Pos) < Radius+pEnt->m_ProximityRadius))
		{
			if(ppEnts)
				ppEnts[Num] = pEnt;
			Num++;
			if(Num == Max)
				break;
		}
		
	}

	return Num;
}


int CGameWorld::FindBlocks(vec2 Pos, ivec2 Radius, CEntity **ppEnts, int Max)
{
	int Num = 0;
	for(CEntity *pEnt = m_apFirstEntityTypes[CGameWorld::ENTTYPE_BLOCK];	pEnt; pEnt = pEnt->m_pNextTypeEntity)
	{
		if (abs(pEnt->m_Pos.x - Pos.x) < Radius.x && abs(pEnt->m_Pos.y - Pos.y) < Radius.y)
		{
			if(ppEnts)
				ppEnts[Num] = pEnt;
			Num++;
			if(Num == Max)
				break;
		}
	}

	return Num;
}


void CGameWorld::InsertEntity(CEntity *pEnt)
{
#ifdef CONF_DEBUG
	for(CEntity *pCur = m_apFirstEntityTypes[pEnt->m_ObjType]; pCur; pCur = pCur->m_pNextTypeEntity)
		dbg_assert(pCur != pEnt, "err");
#endif

	// insert it
	if(m_apFirstEntityTypes[pEnt->m_ObjType])
		m_apFirstEntityTypes[pEnt->m_ObjType]->m_pPrevTypeEntity = pEnt;
	pEnt->m_pNextTypeEntity = m_apFirstEntityTypes[pEnt->m_ObjType];
	pEnt->m_pPrevTypeEntity = 0x0;
	m_apFirstEntityTypes[pEnt->m_ObjType] = pEnt;
}

void CGameWorld::DestroyEntity(CEntity *pEnt)
{
	pEnt->m_MarkedForDestroy = true;
}

void CGameWorld::RemoveEntity(CEntity *pEnt)
{
	// not in the list
	if(!pEnt->m_pNextTypeEntity && !pEnt->m_pPrevTypeEntity && m_apFirstEntityTypes[pEnt->m_ObjType] != pEnt)
		return;

	// remove
	if(pEnt->m_pPrevTypeEntity)
		pEnt->m_pPrevTypeEntity->m_pNextTypeEntity = pEnt->m_pNextTypeEntity;
	else
		m_apFirstEntityTypes[pEnt->m_ObjType] = pEnt->m_pNextTypeEntity;
	if(pEnt->m_pNextTypeEntity)
		pEnt->m_pNextTypeEntity->m_pPrevTypeEntity = pEnt->m_pPrevTypeEntity;

	// keep list traversing valid
	if(m_pNextTraverseEntity == pEnt)
		m_pNextTraverseEntity = pEnt->m_pNextTypeEntity;

	pEnt->m_pNextTypeEntity = 0;
	pEnt->m_pPrevTypeEntity = 0;
}

//
void CGameWorld::Snap(int SnappingClient)
{
	for(int i = 0; i < NUM_ENTTYPES; i++)
		for(CEntity *pEnt = m_apFirstEntityTypes[i]; pEnt; )
		{
			m_pNextTraverseEntity = pEnt->m_pNextTypeEntity;
			pEnt->Snap(SnappingClient);
			pEnt = m_pNextTraverseEntity;
		}
}

void CGameWorld::Reset()
{
	// reset all entities
	for(int i = 0; i < NUM_ENTTYPES; i++)
		for(CEntity *pEnt = m_apFirstEntityTypes[i]; pEnt; )
		{
			m_pNextTraverseEntity = pEnt->m_pNextTypeEntity;
			pEnt->Reset();
			pEnt = m_pNextTraverseEntity;
		}
	RemoveEntities();

	GameServer()->m_pController->PostReset();
	RemoveEntities();

	m_ResetRequested = false;
}

void CGameWorld::RemoveEntities()
{
	// destroy objects marked for destruction
	for(int i = 0; i < NUM_ENTTYPES; i++)
		for(CEntity *pEnt = m_apFirstEntityTypes[i]; pEnt; )
		{
			m_pNextTraverseEntity = pEnt->m_pNextTypeEntity;
			if(pEnt->m_MarkedForDestroy)
			{
				RemoveEntity(pEnt);
				pEnt->Destroy();
			}
			pEnt = m_pNextTraverseEntity;
		}
}


int CGameWorld::CountEntities()
{
	int Entities = 0;
	
	for(int i = 0; i < NUM_ENTTYPES; i++)
		for(CEntity *pEnt = m_apFirstEntityTypes[i]; pEnt; )
		{
			m_pNextTraverseEntity = pEnt->m_pNextTypeEntity;
			Entities++;
			pEnt = m_pNextTraverseEntity;
		}
		
	return Entities;
}

void CGameWorld::Tick()
{
	if(m_ResetRequested)
		Reset();

	if(!m_Paused)
	{
		if(GameServer()->m_pController->IsForceBalanced())
			GameServer()->SendChatTarget(-1, "Teams have been balanced");
		// update all objects
		for(int i = 0; i < NUM_ENTTYPES; i++)
		{
			if (i == ENTTYPE_CHARACTER)
				m_Core.ClearDroidHookImpacts();

			for(CEntity *pEnt = m_apFirstEntityTypes[i]; pEnt; )
			{
				m_pNextTraverseEntity = pEnt->m_pNextTypeEntity;
				pEnt->Tick();
				pEnt = m_pNextTraverseEntity;
			}
		}

		for(int i = 0; i < NUM_ENTTYPES; i++)
			for(CEntity *pEnt = m_apFirstEntityTypes[i]; pEnt; )
			{
				m_pNextTraverseEntity = pEnt->m_pNextTypeEntity;
				pEnt->TickDefered();
				pEnt = m_pNextTraverseEntity;
			}
	}
	else
	{
		// update all objects
		for(int i = 0; i < NUM_ENTTYPES; i++)
			for(CEntity *pEnt = m_apFirstEntityTypes[i]; pEnt; )
			{
				m_pNextTraverseEntity = pEnt->m_pNextTypeEntity;
				pEnt->TickPaused();
				pEnt = m_pNextTraverseEntity;
			}
	}

	RemoveEntities();
}



bool CGameWorld::IsShielded(vec2 Pos0, vec2 Pos1, float Radius, int Team)
{
	CWeapon *w = (CWeapon *)FindFirst(ENTTYPE_WEAPON);
	for(; w; w = (CWeapon *)w->TypeNext())
 	{
		if (!w->m_Disabled && GetStaticType(w->GetWeaponType()) == SW_AREASHIELD)
		{
			
			vec2 IntersectPos = closest_point_on_line(Pos0, Pos1, w->m_Pos);
			
			float Len = distance(w->m_Pos + w->m_Center, IntersectPos);
			if (Len < 180+Radius && distance(w->m_Pos + w->m_Center, Pos0) >= 180+Radius)
			{
				return true;
			}	
		}
	}
	
	return false;
}
	
	

CBuilding *CGameWorld::IntersectBuilding(vec2 Pos0, vec2 Pos1, float Radius, vec2 &NewPos, int Team, CEntity *pNotThis)
{
	float ClosestLen = distance(Pos0, Pos1) * 100.0f;
	CBuilding *pClosest = 0;

	CBuilding *p = (CBuilding *)FindFirst(ENTTYPE_BUILDING);
	for(; p; p = (CBuilding *)p->TypeNext())
 	{
		if(p == pNotThis || !p->m_Collision)
			continue;
		
		//if (!GameServer()->m_pController->IsTeamplay())
		//	continue;
		
		/*
		if (GameServer()->m_pController->IsTeamplay())
		{
			if (Team == p->m_Team)
				continue;
		}
		else
		{
			if (Team == 
		}
		*/
		
		if (Team == p->m_Team)
			continue;
		
		//if (p->m_Team >= 0)
		//	continue;
		
		if (GameServer()->m_pController->IsCoop() && Team >= 0 && p->m_Team >= 0 && (p->m_Type == BUILDING_TURRET || p->m_Type == BUILDING_GENERATOR || p->m_Type == BUILDING_TESLACOIL || p->m_Type == BUILDING_REACTOR))
		{
			if (Team >= 0 && Team < MAX_CLIENTS)
			{
				CPlayer *pPlayer = GameServer()->m_apPlayers[Team];
				
				if(pPlayer && !pPlayer->m_IsBot)
					continue;
			}
		}
		
		/*
				// co-op player to player collisiong ignore
		if (g_Config.m_SvDisablePVP && !p->m_IsBot)
		{
			if (pNotThis && pNotThis->GetType() != CGameWorld::ENTTYPE_CHARACTER)
				continue;
			
			if (pNotThis && pNotThis->GetType() == CGameWorld::ENTTYPE_CHARACTER)
			{
				CCharacter *pOwnerChar = (CCharacter *)pNotThis;
				if (!pOwnerChar->m_IsBot)
					continue;
			}
		}
		*/
		
		vec2 IntersectPos = closest_point_on_line(Pos0, Pos1, p->m_Pos);
		float Len = distance(p->m_Pos + p->m_Center, IntersectPos);
		if(Len < p->m_ProximityRadius+Radius || (p->m_Type == BUILDING_GENERATOR && Len < 240+Radius && distance(p->m_Pos + p->m_Center, Pos0) >= 240+Radius))
		{
			Len = distance(Pos0, IntersectPos);
			if(Len < ClosestLen)
			{
				NewPos = IntersectPos;
				ClosestLen = Len;
				pClosest = p;
			}
		}
	}

	return pClosest;
}


CBall *CGameWorld::IntersectBall(vec2 Pos0, vec2 Pos1, float Radius, vec2 &NewPos)
{
	if (!GameServer()->m_pController->m_pBall)
		return NULL;
	
	CBall *pBall = GameServer()->m_pController->m_pBall;
	
	vec2 IntersectPos = closest_point_on_line(Pos0, Pos1, pBall->m_Pos);
	float Len = distance(pBall->m_Pos, IntersectPos);
	
	if(Len < pBall->m_ProximityRadius+Radius)
		return pBall;

	return NULL;
}


CDroid *CGameWorld::IntersectWalker(vec2 Pos0, vec2 Pos1, float Radius, vec2 &NewPos)
{
	float ClosestLen = distance(Pos0, Pos1) * 100.0f;
	CDroid *pClosest = 0;

	CDroid *p = (CDroid *)FindFirst(ENTTYPE_DROID);
	for(; p; p = (CDroid *)p->TypeNext())
 	{
		if (p->m_Health <= 0)
			continue;
		
		vec2 IntersectPos = closest_point_on_line(Pos0, Pos1, p->m_Pos);
		float Len = distance(p->m_Pos + p->m_Center, IntersectPos);
		if(Len < p->m_ProximityRadius+Radius)
		{
			Len = distance(Pos0, IntersectPos);
			if(Len < ClosestLen)
			{
				NewPos = IntersectPos;
				ClosestLen = Len;
				pClosest = p;
			}
		}
	}

	return pClosest;
}


bool CGameWorld::GetDroidPosChange(int ID)
{
	CDroid *p = (CDroid *)FindFirst(ENTTYPE_DROID);
	for(; p; p = (CDroid *)p->TypeNext())
 	{
		if (p->m_ID == ID)
		{
			return true;
		}
	}

	return false;
}



// TODO: should be more general
CCharacter *CGameWorld::IntersectCharacter(vec2 Pos0, vec2 Pos1, float Radius, vec2& NewPos, CEntity *pNotThis, bool IgnoreDeathrayed)
{
	// Find other players
	float ClosestLen = distance(Pos0, Pos1) * 100.0f;
	CCharacter *pClosest = 0;

	CCharacter *p = (CCharacter *)FindFirst(ENTTYPE_CHARACTER);
	for(; p; p = (CCharacter *)p->TypeNext())
 	{
		if(p == pNotThis)
			continue;
		
		if(p->IgnoreCollision())
			continue;
		
		if (IgnoreDeathrayed && p->Deathrayed())
			continue;
		
		// co-op player to player collisiong ignore
		if (g_Config.m_SvDisablePVP && !p->m_IsBot)
		{
			if (pNotThis && pNotThis->GetType() != CGameWorld::ENTTYPE_CHARACTER)
				continue;
			
			if (pNotThis && pNotThis->GetType() == CGameWorld::ENTTYPE_CHARACTER)
			{
				CCharacter *pOwnerChar = (CCharacter *)pNotThis;
				if (!pOwnerChar->m_IsBot)
					continue;
			}
		}

		vec2 IntersectPos = closest_point_on_line(Pos0, Pos1, p->m_Pos);
		float Len = distance(p->m_Pos, IntersectPos);
		if(Len < p->m_ProximityRadius+Radius+p->m_ShieldRadius)
		{
			Len = distance(Pos0, IntersectPos);
			if(Len < ClosestLen)
			{
				NewPos = IntersectPos;
				ClosestLen = Len;
				pClosest = p;
			}
		}
		// head shot
		Len = distance(p->m_Pos + vec2(0, -28), IntersectPos);
		if(Len < p->m_ProximityRadius+Radius)
		{
			Len = distance(Pos0, IntersectPos);
			if(Len < ClosestLen)
			{
				NewPos = IntersectPos;
				ClosestLen = Len;
				pClosest = p;
			}
		}
	}

	return pClosest;
}

CCharacter *CGameWorld::GetFriendlyCharacterInBox(vec2 TopLeft, vec2 BotRight, int Team)
{
	vec2 Center = (TopLeft+BotRight)/2;
	
	// Find other players
	CCharacter *p = (CCharacter *)FindFirst(ENTTYPE_CHARACTER);
	for(; p; p = (CCharacter *)p->TypeNext())
 	{
		if (!p->GetPlayer())
			continue;
		
		// team checks, assume team is clientID in dm
		if (g_Config.m_SvDisablePVP)
		{
			if ((Team < 0 && !p->m_IsBot) || (Team >= 0 && p->m_IsBot))
				continue;
		}
		else
		{
			if (GameServer()->m_pController->IsTeamplay())
			{
				if (Team != p->GetPlayer()->GetTeam())
					continue;
			}
			else
				if (Team != p->GetPlayer()->GetCID())
					continue;
		}
		
		if (abs(p->m_Pos.x - Center.x) < abs(TopLeft.x - BotRight.x) &&
			abs(p->m_Pos.y - Center.y) < abs(TopLeft.y - BotRight.y))
			return p;
	}

	return NULL;
}


CCharacter *CGameWorld::IntersectReflect(vec2 Pos0, vec2 Pos1, float Radius, vec2& NewPos, CEntity *pNotThis)
{
	// Find other players
	float ClosestLen = distance(Pos0, Pos1) * 100.0f;
	CCharacter *pClosest = 0;

	CCharacter *p = (CCharacter *)FindFirst(ENTTYPE_CHARACTER);
	for(; p; p = (CCharacter *)p->TypeNext())
 	{
		if(p == pNotThis)
			continue;
		
		if(p->IgnoreCollision())
			continue;
		
		// co-op player to player collisiong ignore
		if (g_Config.m_SvDisablePVP && !p->m_IsBot)
		{
			if (pNotThis && pNotThis->GetType() != CGameWorld::ENTTYPE_CHARACTER)
				continue;
			
			if (pNotThis && pNotThis->GetType() == CGameWorld::ENTTYPE_CHARACTER)
			{
				CCharacter *pOwnerChar = (CCharacter *)pNotThis;
				if (!pOwnerChar->m_IsBot)
					continue;
			}
		}

		int Reflect = p->Reflect();
		
		if (!Reflect)
			continue;
		
		vec2 IntersectPos = closest_point_on_line(Pos0, Pos1, p->m_Pos);
		
		// only reflect in some directions
		//if (abs(GetAngle(normalize(p->m_Pos - IntersectPos)) - GetAngle(normalize(p->GetVel()))) > pi/4.0f)
		//	continue;
		
		float Len = distance(p->m_Pos + vec2(0, -32), IntersectPos);
		if(Len < Reflect+Radius)
		{
			Len = distance(Pos0, IntersectPos);
			if(Len < ClosestLen)
			{
				NewPos = IntersectPos;
				ClosestLen = Len;
				pClosest = p;
			}
		}
	}

	return pClosest;
}


CCharacter *CGameWorld::ClosestCharacter(vec2 Pos, float Radius, CEntity *pNotThis)
{
	// Find other players
	float ClosestRange = Radius*2;
	CCharacter *pClosest = 0;

	CCharacter *p = (CCharacter *)GameServer()->m_World.FindFirst(ENTTYPE_CHARACTER);
	for(; p; p = (CCharacter *)p->TypeNext())
 	{
		if(p == pNotThis)
			continue;

		float Len = distance(Pos, p->m_Pos);
		if(Len < p->m_ProximityRadius+Radius)
		{
			if(Len < ClosestRange)
			{
				ClosestRange = Len;
				pClosest = p;
			}
		}
		// head collision
		Len = distance(Pos, p->m_Pos + vec2(0, -28));
		if(Len < p->m_ProximityRadius+Radius)
		{
			if(Len < ClosestRange)
			{
				ClosestRange = Len;
				pClosest = p;
			}
		}
	}

	return pClosest;
}
