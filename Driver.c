#include <ntifs.h>
#include <ntddk.h>

//function options 
#define QUERYINFO
//end options



BOOLEAN create_file(UNICODE_STRING path);
BOOLEAN delete_file(UNICODE_STRING path);

VOID DriverUnload(PDRIVER_OBJECT driver_obj){

	UNICODE_STRING path;
	RtlInitUnicodeString(&path, L"\\??\\C:\\from_driver");
	if (delete_file(path) == FALSE)
	{
		DbgPrint("fail delete file\n");
		return;
	}
	DbgPrint("success delete file\n");
	DbgPrint("unload good bye\n");
	return;



}

NTSTATUS DriverEntry(PDRIVER_OBJECT driver_obj, PUNICODE_STRING reg_path){


	driver_obj->DriverUnload = DriverUnload;
	DbgPrint("hello world\n");
	UNICODE_STRING path;
	RtlInitUnicodeString(&path, L"\\??\\C:\\from_driver");
	if (create_file(path) == FALSE){
		//do nothing

	}

	return STATUS_SUCCESS;
}


BOOLEAN create_file(UNICODE_STRING path){
	NTSTATUS ntstat;
	HANDLE hd;
	OBJECT_ATTRIBUTES obj_attr = { 0 };
	IO_STATUS_BLOCK ios_block = { 0 };
	InitializeObjectAttributes(&obj_attr, &path, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	ntstat = ZwCreateFile(&hd, GENERIC_READ, &obj_attr, &ios_block, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	if (!NT_SUCCESS(ntstat))
	{
		DbgPrint("fail create file\n");
		return FALSE;
	}
	DbgPrint("success create file\n");


#ifdef QUERYINFO

	FILE_STANDARD_INFORMATION fsi = { 0 };
	ntstat = ZwQueryInformationFile(hd, &ios_block, &fsi, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);
	if (!NT_SUCCESS(ntstat))
	{
		DbgPrint("fail query fileinfo\n");
	}

	DbgPrint("file type ");
	fsi.Directory == TRUE ? DbgPrint("dir\n") : DbgPrint("file\n");
	DbgPrint("file size ");
	fsi.EndOfFile.QuadPart == 0 ? DbgPrint("empty\n") : DbgPrint("%L\n", fsi.EndOfFile);

#endif

	ZwClose(hd);
	return TRUE;


}

BOOLEAN delete_file(UNICODE_STRING path){
	NTSTATUS ntstat;
	OBJECT_ATTRIBUTES obj_attr = { 0 };
	InitializeObjectAttributes(&obj_attr, &path, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	ntstat = ZwDeleteFile(&obj_attr);
	if (!NT_SUCCESS(ntstat))
	{
		return FALSE;
	}
	return TRUE;
}
