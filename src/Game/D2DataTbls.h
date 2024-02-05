#pragma once

#include "DataTbls/ItemsTbls.h"
#include "DataTbls/LevelsTbls.h"
#include "DataTbls/MonsterTbls.h"
#include "DataTbls/ObjectsTbls.h"
#include "DataTbls/SkillsTbls.h"

// rename D2DataTablesStrc
struct D2DataTablesStrc {
  void* pPlayerClass;
  DWORD dwPlayerClassRecords;
  void* pBodyLocs;
  DWORD dwBodyLocsRecords;
  void* pStorePage;
  DWORD dwStorePageRecords;
  void* pElemTypes;
};