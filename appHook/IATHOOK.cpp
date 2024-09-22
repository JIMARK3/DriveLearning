// IATHOOK.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Windows.h>
#include <hash_map>
void createmap_data(PIMAGE_IMPORT_DESCRIPTOR describes);
std::hash_map<char *, std::hash_map<char *, DWORD>>dllFuncAddrMap;
char * dllNameTemp;
DWORD base;

int
WINAPI
Hook_MessageBoxA(
_In_opt_ HWND hWnd,
_In_opt_ LPCSTR lpText,
_In_opt_ LPCSTR lpCaption,
_In_ UINT uType){

	printf("hookhookhook\n");
	return IDOK;
};



void IAThook(){

	HMODULE hd = GetModuleHandle(NULL);
	base = (DWORD)hd;
	PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)hd;
	LONG e_lfanew = dos_header->e_lfanew;
	PIMAGE_NT_HEADERS nt_header = (PIMAGE_NT_HEADERS)(base + (DWORD)e_lfanew);
	IMAGE_DATA_DIRECTORY data_dir =
		nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	PIMAGE_IMPORT_DESCRIPTOR describes = (PIMAGE_IMPORT_DESCRIPTOR)(data_dir.VirtualAddress + base);

	while (describes->Name != NULL)
	{
		dllNameTemp = (char *)((DWORD)(describes->Name) + base);
		createmap_data(describes);
		describes++;
	}




	//DWORD old_protect;
	//DWORD* target = (DWORD*)iat_entrys;
	//VirtualProtect(target, 4, PAGE_EXECUTE_READWRITE, &old_protect);
	//DWORD orign_addr;
	//orign_addr = *(DWORD*)iat_entrys;
	//*target = (DWORD)Hook_MessageBoxA;
	//VirtualProtect(target, 4, old_protect, NULL);



}


int _tmain(int argc, _TCHAR* argv[])
{
	MessageBoxA(NULL, "hello", "hello", MB_OK);
	IAThook();
	for (auto p :  dllFuncAddrMap)
	{
		for (auto p2: p.second)
		{
			printf("dll\t%s\tfunc\t%s\n", p.first, p2.first);
			printf("addr\t0x%x\n\n", p2.second);

		}
	}
	return 0;
}

void createmap_data(PIMAGE_IMPORT_DESCRIPTOR describes){
	PIMAGE_THUNK_DATA thunk_data = (PIMAGE_THUNK_DATA)(((DWORD)(describes->OriginalFirstThunk)) + base);
	DWORD iat_entrys = (DWORD)(describes->FirstThunk) + base;
	while (thunk_data->u1.Function != NULL)

	{
		PIMAGE_IMPORT_BY_NAME by_name = (PIMAGE_IMPORT_BY_NAME)((DWORD)thunk_data->u1.Function + base);
		char * func_name = (char *)((DWORD)by_name->Name);
		dllFuncAddrMap[dllNameTemp][func_name] = iat_entrys;

		thunk_data++;
		iat_entrys += 4;

	}

}
