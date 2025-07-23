#ifndef GAME_SERVER_GAMEMODES_INVASION_H
#define GAME_SERVER_GAMEMODES_INVASION_H
#include <game/server/gamecontroller.h>

#define MAX_ENEMIES 512

enum GroupTypes
{
	GROUP_ALIENS,
	GROUP_ROBOTS,
	GROUP_BUNNIES,
	GROUP_PYROS,
	GROUP_SKELETONS,
};


class CGameControllerInvasion : public IGameController
{
private:
	int m_LevelQuestsLeft;
	int m_QuestsCompleted;

	int m_Quest;
	int m_NextQuest;
	int m_QuestChangeTick;
	int m_QuestProgressCounter;
	
	int m_QuestWaveType;
	int m_QuestWaveEndTick;
	int m_QuestWaveEnemiesLeft;
	int m_QuestWaveSize;
	
	void ChangeQuest(int NextQuest, float QueueTimeInSeconds);
	void SendQuestStartMessage(int Quest);
	void SendQuestCompletedMessage(int Quest);
	void CompleteCurrentQuest();
	
	enum Enemies
	{
		ENEMY_ALIEN1,
		ENEMY_ROBOT1,
		ENEMY_ROBOT2,
		ENEMY_ALIEN2,
		ENEMY_BUNNY1,
		ENEMY_BUNNY2,
		ENEMY_PYRO1,
		ENEMY_PYRO2,
		NUM_ENEMIES
	};

	vec2 m_aEnemySpawnPos[MAX_ENEMIES];
	
	int m_Deaths;
	bool m_RoundWin;
	int m_RoundWinTick;
	int m_RoundOverTick;
	
	// enemy grouping
	vec2 m_GroupSpawnPos;
	int m_GroupType;
	
	void SpawnNewWave(bool AddBots = true);
	
	vec2 GetBotSpawnPos();
	void RandomGroupSpawnPos();
	int m_BotSpawnTick;
	
	// hordes of enemies
	int m_EnemyCount;
	int m_EnemiesLeft;
	
	int m_BossesLeft;
	
	int m_NumEnemySpawnPos;
	int m_SpawnPosRotation;
	
	int m_TriggerLevel;
	int m_TriggerTick;
	
	bool m_AutoRestart;
	
	void Trigger(bool IncreaseLevel);
	
	class CRadar *m_pDoor;
	class CRadar *m_pEnemySpawn;
	
public:
	CGameControllerInvasion(class CGameContext *pGameServer);
	
	virtual bool OnEntity(int Index, vec2 Pos);
	void OnCharacterSpawn(class CCharacter *pChr, bool RequestAI = false);
	int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);
	bool CanSpawn(int Team, vec2 *pPos, bool IsBot = false);
	void NextLevel(int CID = -1);
	bool GetSpawnPos(int Team, vec2 *pOutPos);
	virtual void Tick();
	virtual void Snap(int SnappingClient);
	
	void DisplayExit(vec2 Pos);
	
	enum GameState
	{
		STATE_STARTING,
		STATE_GAME,
		STATE_FAIL,
	};
};
#endif
