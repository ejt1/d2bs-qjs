#pragma once

struct Waypoint;
struct QuestInfo;

// rename D2PlayerDataStrc
#pragma pack(push, 1)
struct PlayerData {
  char szName[0x10];             // 0x00
  QuestInfo* pNormalQuest;       // 0x10
  QuestInfo* pNightmareQuest;    // 0x14
  QuestInfo* pHellQuest;         // 0x18
  Waypoint* pNormalWaypoint;     // 0x1c
  Waypoint* pNightmareWaypoint;  // 0x20
  Waypoint* pHellWaypoint;       // 0x24
};
#pragma pack(pop)