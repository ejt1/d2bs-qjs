#include "ScriptEngine.h"
#include "D2Helpers.h"
#include "Helpers.h"
#include "dde.h"

DWORD DdeSrvInst = 0;
HSZ hszD2BSns;

HDDEDATA CALLBACK DdeCallback(UINT uType, UINT, HCONV, HSZ, HSZ, HDDEDATA hdata, DWORD, DWORD) {
  char pszItem[65535] = "";
  wchar_t* pslzItem = AnsiToUnicode(pszItem);
  switch (uType) {
    case XTYP_CONNECT:
      return (HDDEDATA)TRUE;
    case XTYP_POKE:
      DdeGetData(hdata, (LPBYTE)pszItem, 255, 0);
      if (SwitchToProfile(pslzItem))
        Log(L"Switched to profile %s", pslzItem);
      else
        Log(L"Profile %s not found", pslzItem);
      break;
    case XTYP_EXECUTE:
      DdeGetData(hdata, (LPBYTE)pszItem, sizeof(pszItem), 0);
      sScriptEngine->RunCommand(pslzItem);
      break;
  }
  delete[] pslzItem;
  return (HDDEDATA)0;
}

DWORD CreateDdeServer() {
  char buf[1000];

  int ret = DdeInitialize(
      &DdeSrvInst, DdeCallback,
      APPCLASS_STANDARD | APPCMD_FILTERINITS | CBF_FAIL_ADVISES | CBF_FAIL_REQUESTS | CBF_SKIP_CONNECT_CONFIRMS | CBF_SKIP_REGISTRATIONS | CBF_SKIP_UNREGISTRATIONS, 0);
  if (ret != DMLERR_NO_ERROR)
    return 0;
  char handle[25];
  sprintf_s(handle, sizeof(handle), "d2bs-%d", GetProcessId(GetCurrentProcess()));
  hszD2BSns = DdeCreateStringHandle(DdeSrvInst, handle, CP_WINANSI);
  if (!hszD2BSns)
    return 0;
  if (!DdeNameService(DdeSrvInst, hszD2BSns, 0L, DNS_REGISTER | DNS_FILTERON)) {
    sprintf_s(buf, sizeof(buf), "DdeServer DdeNameService Error: %X", DdeGetLastError(DdeSrvInst));
    OutputDebugString(buf);
    return 0;
  }
  return GetLastError();
}

BOOL ShutdownDdeServer() {
  DdeFreeStringHandle(DdeSrvInst, hszD2BSns);
  return DdeUninitialize(DdeSrvInst);
}

BOOL SendDDE(int mode, const char* pszDDEServer, const char* pszTopic, const char* pszItem, const char* pszData, char** result, uint size) {
  DWORD pidInst = 0;
  HCONV hConv;
  DWORD dwTimeout = 5000;

  Log(L"SendDDE(%d, %hs, %hs, %hs, %hs, %hs, %u)", mode, pszDDEServer, pszTopic, pszItem, pszData, *result, size);

  int ret = DdeInitialize(&pidInst, (PFNCALLBACK)DdeCallback, APPCMD_CLIENTONLY, 0);
  if (ret != DMLERR_NO_ERROR) {
    Log(L"DdeInitialize Error: %X", ret);
    return FALSE;
  }

  HSZ hszDDEServer = DdeCreateStringHandle(pidInst, (strlen(pszDDEServer) == 0 ? "\"\"" : pszDDEServer), CP_WINANSI);
  HSZ hszTopic = DdeCreateStringHandle(pidInst, (strlen(pszTopic) == 0 ? "\"\"" : pszTopic), CP_WINANSI);
  HSZ hszCommand = DdeCreateStringHandle(pidInst, (strlen(pszItem) == 0 ? "\"\"" : pszItem), CP_WINANSI);

  if (!(hszDDEServer && hszTopic && hszCommand)) {
    UINT err = DdeGetLastError(pidInst);
    Log(L"Error creating DDE Handles: %d", err);
    return FALSE;
  }

  hConv = DdeConnect(pidInst, hszDDEServer, hszTopic, 0);

  switch (mode) {
    case 0: {
      HDDEDATA DdeSrvData = DdeClientTransaction(0, 0, hConv, hszCommand, CF_TEXT, XTYP_REQUEST, dwTimeout, 0);
      DdeGetData(DdeSrvData, (LPBYTE)result, size, 0);
      break;
    }
    case 1:
      DdeClientTransaction((LPBYTE)pszData, strlen(pszData) + 1, hConv, hszCommand, CF_TEXT, XTYP_POKE, dwTimeout, 0);
      break;
    case 2:
      DdeClientTransaction((LPBYTE)pszData, strlen(pszData) + 1, hConv, 0L, 0, XTYP_EXECUTE, dwTimeout, 0);
      break;
  }

  DdeFreeStringHandle(pidInst, hszDDEServer);
  DdeFreeStringHandle(pidInst, hszTopic);
  DdeFreeStringHandle(pidInst, hszCommand);
  DdeUninitialize(pidInst);

  return TRUE;
}
