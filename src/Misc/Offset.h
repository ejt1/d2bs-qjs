#ifndef _OFFSET_H
#define _OFFSET_H

#include <cstdint>

#define INST_INT3 0xCC
#define INST_CALL 0xE8
#define INST_NOP 0x90
#define INST_JMP 0xE9
#define INST_RET 0xC3

typedef struct PatchHook_t {
  void (*pFunc)(uint32_t, uint32_t, uint32_t);
  uint32_t dwAddr;
  uint32_t dwFunc;
  uint32_t dwLen;
  int* enabled;
  uint8_t* bOldCode;
} PatchHook;

void InitOffsets();

PatchHook* RetrievePatchHooks(int* pBuffer);
void PatchBytes(uint32_t dwAddr, uint32_t dwValue, uint32_t dwLen);
void PatchJmp(uint32_t dwAddr, uint32_t dwFunc, uint32_t dwLen);
void PatchCall(uint32_t dwAddr, uint32_t dwFunc, uint32_t dwLen);
void InterceptLocalCode(uint8_t bInst, uint32_t pAddr, uint32_t pFunc, uint32_t dwLen);
void FillBytes(void* pAddr, uint8_t bFill, uint32_t dwLen);
bool WriteBytes(void* pAddr, void* pData, uint32_t dwLen);
void RemovePatches();
void InstallPatches();
void InstallConditional();
void RemoveConditional();

#endif