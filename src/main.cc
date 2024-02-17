#include "Engine.h"

#include <memory>

static std::unique_ptr<Engine> kEngineInstance;

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved) {
  UNREFERENCED_PARAMETER(lpReserved);

  switch (dwReason) {
    case DLL_PROCESS_ATTACH:
      DisableThreadLibraryCalls(hModule);

      kEngineInstance = std::make_unique<Engine>();
      if (!kEngineInstance->Initialize(hModule)) {
        return FALSE;
      }
      break;

    case DLL_PROCESS_DETACH:
      kEngineInstance->Shutdown();
      kEngineInstance.reset();
      break;
  }

  return TRUE;
}
