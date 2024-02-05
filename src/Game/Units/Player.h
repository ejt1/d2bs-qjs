#pragma once

struct D2WaypointDataStrc;
struct D2QuestInfoStrc;

#pragma pack(push, 1)
struct D2PlayerDataStrc {
  char szName[0x10];                       // 0x00
  D2QuestInfoStrc* pNormalQuest;           // 0x10
  D2QuestInfoStrc* pNightmareQuest;        // 0x14
  D2QuestInfoStrc* pHellQuest;             // 0x18
  D2WaypointDataStrc* pNormalWaypoint;     // 0x1c
  D2WaypointDataStrc* pNightmareWaypoint;  // 0x20
  D2WaypointDataStrc* pHellWaypoint;       // 0x24
};
#pragma pack(pop)