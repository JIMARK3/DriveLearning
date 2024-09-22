// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"

DWORD base;
DWORD* targetAddr;
DWORD orignCode;
DWORD hookCode;

extern "C"
__declspec(dllexport)
void Hook();

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Hook();
		break;
	case DLL_THREAD_ATTACH:
		Hook();
		break;
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}




int
WINAPI
HOOK_MessageBoxA(
_In_opt_ HWND hWnd,
_In_opt_ LPCSTR lpText,
_In_opt_ LPCSTR lpCaption,
_In_ UINT uType){
	MessageBoxW(NULL, L"into", L"into", MB_OK);
	DWORD oldProtect;
	VirtualProtect(targetAddr, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(targetAddr, &orignCode, 4);
	int i = MessageBoxA(hWnd, "hook success!", "hook success!", uType);
	memcpy(targetAddr, &hookCode, 4);
	VirtualProtect(targetAddr, 4, oldProtect, NULL);
	return i;

};
void Hook(){

	HMODULE hd = GetModuleHandleA(NULL);
	if (!hd)
	{
		MessageBoxW(NULL, L"GetModuleHandleA", L"GetModuleHandleA", MB_OK);
		return;
	}

	base = (DWORD)hd;

	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)base;
	PIMAGE_NT_HEADERS ntHeader = (PIMAGE_NT_HEADERS)(dosHeader->e_lfanew + base);
	IMAGE_DATA_DIRECTORY entryImport = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	PIMAGE_IMPORT_DESCRIPTOR importDescribs = (PIMAGE_IMPORT_DESCRIPTOR)(entryImport.VirtualAddress + base);

	while (importDescribs->Name != NULL&&strcmp("USER32.dll", (char *)(importDescribs->Name + base)) != 0)
	{
		importDescribs++;
	}
	if (!importDescribs->Name)
	{
		MessageBoxW(NULL, L"USER32.dll", L"USER32.dll", MB_OK);
		return;
	}
	//printf("%s\n", (char *)(importDescribs->Name + base));

	PIMAGE_THUNK_DATA thunkData = (PIMAGE_THUNK_DATA)(importDescribs->OriginalFirstThunk + base);
	DWORD iatEntry = importDescribs->FirstThunk + base;

	while (thunkData->u1.Function&&strcmp("MessageBoxA", ((PIMAGE_IMPORT_BY_NAME)(thunkData->u1.Function + base))->Name))
	{
		//printf("%s\n", ((PIMAGE_IMPORT_BY_NAME)(thunkData->u1.Function + base))->Name);
		thunkData++;
		iatEntry += 4;
	}
	if (!thunkData->u1.Function)
	{
		MessageBoxW(NULL, L"thunkData", L"thunkData", MB_OK);
		return;
	}

	targetAddr = (DWORD *)iatEntry;

	//MessageBoxA(NULL, "ok", "ok", MB_OK);

	DWORD oldProtect;
	VirtualProtect(targetAddr, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
	*(&orignCode) = *targetAddr;
	hookCode = (DWORD)HOOK_MessageBoxA;
	*targetAddr = (DWORD)HOOK_MessageBoxA;
	VirtualProtect(targetAddr, 4, oldProtect, NULL);

	//MessageBoxA(NULL, "ok2", "ok2", MB_OK);

}
