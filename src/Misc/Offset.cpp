#define _DEFINE_VARS
#define __D2PTRS_ONCE_
#include "D2Ptrs.h"
#undef __D2PTRS_H__
#define __D2PTRS_LIST_
#include "D2Ptrs.h"
#include "Patch.h"
#include "Engine.h"

#ifndef ArraySize
#define ArraySize(x) (sizeof((x)) / sizeof((x)[0]))
#endif

void DefineOffsets() {
  uint32_t** p = (uint32_t**)d2ptrs_list;
  do {
    **p = GetDllOffset(**p);
  } while (ptrdiff_t(++p) < ((ptrdiff_t)d2ptrs_list) + sizeof(d2ptrs_list));
}

uint32_t GetDllOffset(const char* /*DllName*/, int Offset) {
  HMODULE hMod = GetModuleHandle(NULL);

  if (Offset < 0)
    return (uint32_t)GetProcAddress(hMod, (LPCSTR)(-Offset));

  return ((uint32_t)hMod) + Offset;
}

uint32_t GetDllOffset(int num) {
  static const char* dlls[] = {"D2Client.DLL", "D2Common.DLL", "D2Gfx.DLL",    "D2Lang.DLL", "D2Win.DLL", "D2Net.DLL",  "D2Game.DLL",
                               "D2Launch.DLL", "Fog.DLL",      "BNClient.DLL", "Storm.DLL",  "D2Cmp.DLL", "D2Multi.DLL"};
  if ((num & 0xff) > 12)
    return 0;
  return GetDllOffset(dlls[num & 0xff], num >> 8);
}

void InstallPatches() {
  for (int x = 0; x < ArraySize(Patches); x++) {
    Patches[x].bOldCode = new BYTE[Patches[x].dwLen];
    ::ReadProcessMemory(GetCurrentProcess(), (void*)Patches[x].dwAddr, Patches[x].bOldCode, Patches[x].dwLen, NULL);
    Patches[x].pFunc(Patches[x].dwAddr, Patches[x].dwFunc, Patches[x].dwLen);
  }
}

void RemovePatches() {
  for (int x = 0; x < ArraySize(Patches); x++) {
    WriteBytes((void*)Patches[x].dwAddr, Patches[x].bOldCode, Patches[x].dwLen);
    delete[] Patches[x].bOldCode;
  }
}

void InstallConditional() {
  for (int x = 0; x < ArraySize(Conditional); x++) {
    if (Conditional[x].enabled == NULL || *Conditional[x].enabled != TRUE) {
      continue;
    }
    Conditional[x].bOldCode = new BYTE[Conditional[x].dwLen];
    ::ReadProcessMemory(GetCurrentProcess(), (void*)Conditional[x].dwAddr, Conditional[x].bOldCode, Conditional[x].dwLen, NULL);
    Conditional[x].pFunc(Conditional[x].dwAddr, Conditional[x].dwFunc, Conditional[x].dwLen);
  }
}

void RemoveConditional() {
  for (int x = 0; x < ArraySize(Conditional); x++) {
    if (Conditional[x].enabled == NULL || *Conditional[x].enabled != TRUE)
      continue;
    WriteBytes((void*)Conditional[x].dwAddr, Conditional[x].bOldCode, Conditional[x].dwLen);
    delete[] Conditional[x].bOldCode;
  }
}

bool WriteBytes(void* pAddr, void* pData, uint32_t dwLen) {
  DWORD dwOld;

  if (!VirtualProtect(pAddr, dwLen, PAGE_READWRITE, &dwOld))
    return FALSE;

  ::memcpy(pAddr, pData, dwLen);
  return VirtualProtect(pAddr, dwLen, dwOld, &dwOld);
}

void FillBytes(void* pAddr, uint8_t bFill, uint32_t dwLen) {
  BYTE* bCode = new BYTE[dwLen];
  ::memset(bCode, bFill, dwLen);

  WriteBytes(pAddr, bCode, dwLen);

  delete[] bCode;
}

void InterceptLocalCode(uint8_t bInst, uint32_t pAddr, uint32_t pFunc, uint32_t dwLen) {
  BYTE* bCode = new BYTE[dwLen];
  ::memset(bCode, 0x90, dwLen);
  DWORD dwFunc = pFunc - (pAddr + 5);

  bCode[0] = bInst;
  *(DWORD*)&bCode[1] = dwFunc;
  WriteBytes((void*)pAddr, bCode, dwLen);

  delete[] bCode;
}

void PatchCall(uint32_t dwAddr, uint32_t dwFunc, uint32_t dwLen) {
  InterceptLocalCode(INST_CALL, dwAddr, dwFunc, dwLen);
}

void PatchJmp(uint32_t dwAddr, uint32_t dwFunc, uint32_t dwLen) {
  InterceptLocalCode(INST_JMP, dwAddr, dwFunc, dwLen);
}

void PatchBytes(uint32_t dwAddr, uint32_t dwValue, uint32_t dwLen) {
  uint8_t* bCode = new uint8_t[dwLen];
  ::memset(bCode, (uint8_t)dwValue, dwLen);

  WriteBytes((void*)dwAddr, bCode, dwLen);

  delete[] bCode;
}

PatchHook* RetrievePatchHooks(int* pBuffer) {
  *pBuffer = ArraySize(Patches);
  return &Patches[0];
}
