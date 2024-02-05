#pragma once

#include "DataTbls/ItemsTbls.h"
#include "DataTbls/LevelsTbls.h"
#include "DataTbls/MonsterTbls.h"
#include "DataTbls/ObjectsTbls.h"
#include "DataTbls/SkillsTbls.h"

#include <cstdint>

// rename D2DataTablesStrc
struct D2DataTablesStrc {
  void* pPlayerClass;
  uint32_t dwPlayerClassRecords;
  void* pBodyLocs;
  uint32_t dwBodyLocsRecords;
  void* pStorePage;
  uint32_t dwStorePageRecords;
  void* pElemTypes;
};