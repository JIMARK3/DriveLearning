#include<ntifs.h>
#include<ntddk.h>
#define logerr DbgPrint
#define log DbgPrint
#define COPYFILE L"\\??\\C:\\tools.chm"
#define COPYDEST L"\\??\\C:\\back_tools.chm"

void DriveUnload(PDRIVER_OBJECT driveObj);
HANDLE CreateFile(UNICODE_STRING path);
BOOLEAN Copy(HANDLE src, HANDLE dest);


NTSTATUS DriverEntry(PDRIVER_OBJECT driveObj, PUNICODE_STRING regPath){

	driveObj->DriverUnload = DriveUnload;

	UNICODE_STRING src_path;
	RtlInitUnicodeString(&src_path, COPYFILE);
	HANDLE src = CreateFile(src_path);
	if (src == NULL)
	{
		return STATUS_SUCCESS;
	}

	UNICODE_STRING dest_path;
	RtlInitUnicodeString(&dest_path, COPYDEST);
	HANDLE dest = CreateFile(dest_path);
	if (dest == NULL)
	{
		ZwClose(src);
		return STATUS_SUCCESS;
	}

	if (Copy(src, dest) == FALSE){
		// do nothing;
	};


	ZwClose(src);
	ZwClose(dest);

	return STATUS_SUCCESS;


}

void DriveUnload(PDRIVER_OBJECT driveObj){
	DbgPrint("UNLOAD\n");
}

HANDLE CreateFile(UNICODE_STRING path){

	NTSTATUS status;
	HANDLE hd;
	OBJECT_ATTRIBUTES obj_attr = { 0 };
	IO_STATUS_BLOCK io_block = { 0 };
	InitializeObjectAttributes(&obj_attr, &path, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	status = ZwCreateFile(&hd, GENERIC_READ | GENERIC_WRITE, &obj_attr, &io_block, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	if (!NT_SUCCESS(status))
	{
		logerr("FAIL OPEN FILE %wZ\n", &path);
		return NULL;
	}
	log("SUCCESS CREATE FILE %wZ\n", &path);
	return hd;
}

LARGE_INTEGER FileSize(HANDLE hd){
	LARGE_INTEGER err_size = { 0 };
	err_size.QuadPart = -1;
	NTSTATUS status;
	IO_STATUS_BLOCK io_block = { 0 };
	FILE_STANDARD_INFORMATION fsi = { 0 };
	status = ZwQueryInformationFile(hd, &io_block, &fsi, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);
	if (!NT_SUCCESS(status))
	{
		logerr("ERR GET FILE SIZE\n");
		return err_size;
	}


	log("FILE SIZE %I64d\n", fsi.EndOfFile.QuadPart);


	return fsi.EndOfFile;


};

BOOLEAN Copy(HANDLE src, HANDLE dest){
	LARGE_INTEGER src_size;
	src_size = FileSize(src);
	if (src_size.QuadPart == -1)
	{
		logerr("FAIL COPY\n");
		return FALSE;
	}


	if (src_size.HighPart != 0)
	{
		logerr("FAIL SRC FILE TOO LARGE\n");
		return FALSE;
	}

	PVOID buffer = ExAllocatePool(NonPagedPool, src_size.LowPart);
	if (buffer == NULL)
	{
		logerr("FAIL ALLOC BUFFER\n");
		return FALSE;
	}


	NTSTATUS status;
	IO_STATUS_BLOCK io_block = { 0 };

	status = ZwReadFile(src, NULL, NULL, NULL, &io_block, buffer, src_size.LowPart, NULL, NULL);
	if (!NT_SUCCESS(status))
	{
		ExFreePool(buffer);
		logerr("FAIL READ FILE\n");
		return FALSE;
	}
	ULONG read_size = io_block.Information;
	log("READ SIZE %u\n", read_size);

	RtlZeroMemory(&io_block, sizeof(io_block));
	status = ZwWriteFile(dest, NULL, NULL, NULL, &io_block, buffer, src_size.LowPart, NULL, NULL);
	if (!NT_SUCCESS(status))
	{
		ExFreePool(buffer);
		logerr("FAIL WRITE FILE\n");
		return FALSE;
	}
	log("SUCCESS COPY\n");
	ExFreePool(buffer);
	return TRUE;
}