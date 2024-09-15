#include<ntddk.h>
#include<ntstrsafe.h>


#define log DbgPrint 
#define logerr DbgPrint 





PDEVICE_OBJECT global_filter_dev = NULL;
PDEVICE_OBJECT next_dev = NULL;

VOID DriverDetach(PDRIVER_OBJECT driver_obj);

NTSTATUS Dispatch(PDEVICE_OBJECT device_obj, PIRP pirp);

NTSTATUS AttachCom(PDRIVER_OBJECT driver_obj);

PDEVICE_OBJECT OpenCom(ULONG comid);

VOID DriverUnload(PDRIVER_OBJECT driver_obj);

NTSTATUS DriverEntry(PDRIVER_OBJECT driver_obj, UNICODE_STRING reg_path){

	driver_obj->DriverUnload = DriverDetach;

	for (ULONG i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		driver_obj->MajorFunction[i] = Dispatch;

	}


	NTSTATUS status = AttachCom(driver_obj);
	if (!NT_SUCCESS(status))
	{
		return STATUS_SUCCESS;
	}



	return STATUS_SUCCESS;

}

VOID DriverDetach(PDRIVER_OBJECT driver_obj){

	if (next_dev != NULL)
	{
		IoDetachDevice(next_dev);
	}

	LARGE_INTEGER delay_time;
	delay_time.QuadPart = 5 * 1000 * -10 * 1000;
	KeDelayExecutionThread(KernelMode, FALSE, &delay_time);

	if (global_filter_dev != NULL)
	{
		IoDeleteDevice(global_filter_dev);
	}
	DbgPrint("DETACH OK\n");

}

VOID DriverUnload(PDRIVER_OBJECT driver_obj){

	DbgPrint("UNLOAD\n");
}



PDEVICE_OBJECT OpenCom(ULONG comid){
	PFILE_OBJECT file_obj;
	PDEVICE_OBJECT real_dev;
	WCHAR name[32];
	RtlStringCchPrintfW(name, sizeof(WCHAR) * 32, L"\\Device\\Serial%d", comid);
	UNICODE_STRING uni_com_name;
	RtlInitUnicodeString(&uni_com_name, name);
	log("UNI_COM_NAME IS %wZ\n", &uni_com_name);

	NTSTATUS status;
	status = IoGetDeviceObjectPointer(&uni_com_name, FILE_ALL_ACCESS, &file_obj, &real_dev);
	if (!NT_SUCCESS(status)){
		logerr("FAIL IoGetDeviceObjectPointer\n");
		return NULL;
	}
	ObDereferenceObject(file_obj);
	return real_dev;

}

NTSTATUS AttachCom(PDRIVER_OBJECT driver_obj){
	//open com 
	PDEVICE_OBJECT real_dev;
	for (ULONG COMID = 0; COMID < 32; COMID++)
	{
		real_dev = OpenCom(COMID);
		if (NULL == real_dev)
		{
			break;
		}
	}


	if (NULL == real_dev)
	{
		return STATUS_UNSUCCESSFUL;

	}

	NTSTATUS status;
	PDEVICE_OBJECT filter_dev;

	//create device obj
	status = IoCreateDevice(driver_obj, 0, NULL, real_dev->DeviceType, 0, FALSE, &filter_dev);
	if (!NT_SUCCESS(status))
	{
		logerr("FAIL IoCreateDevice\n");
		return STATUS_UNSUCCESSFUL;
	}

	//set device obj
	if ((real_dev->Flags&DO_BUFFERED_IO) != 0)
	{
		filter_dev->Flags |= DO_BUFFERED_IO;
	}
	if ((real_dev->Flags&DO_DIRECT_IO) != 0)
	{
		filter_dev->Flags |= DO_DIRECT_IO;
	}
	filter_dev->Flags |= DO_POWER_PAGABLE;

	if ((real_dev->Characteristics &FILE_DEVICE_SECURE_OPEN) != 0)
	{
		filter_dev->Flags |= FILE_DEVICE_SECURE_OPEN;
	}


	//append device obj to device stack of com  
	PDEVICE_OBJECT top_dev = IoAttachDeviceToDeviceStack(filter_dev, real_dev);
	next_dev = top_dev;//set global var ; never write again;

	if (top_dev == NULL)
	{
		IoDeleteDevice(filter_dev);
		filter_dev = NULL;
		return STATUS_UNSUCCESSFUL;
	}
	filter_dev->Flags &= (~DO_DEVICE_INITIALIZING);

	global_filter_dev = filter_dev;

	return STATUS_SUCCESS;


}

NTSTATUS Dispatch(PDEVICE_OBJECT device_obj, PIRP pirp){
	ULONG length;
	PIO_STACK_LOCATION stack_location = IoGetCurrentIrpStackLocation(pirp);
	switch (stack_location->MajorFunction)
	{
	case IRP_MJ_POWER:
		PoStartNextPowerIrp(pirp);
		IoSkipCurrentIrpStackLocation(pirp);
		return PoCallDriver(next_dev, pirp);
		break;
	case IRP_MJ_WRITE:

		length = stack_location->Parameters.Write.Length;
		PCSCHAR buf;
		if (pirp->MdlAddress != NULL)
		{
			buf = MmGetSystemAddressForMdlSafe(pirp->MdlAddress, NormalPagePriority);
		}
		else if (pirp->UserBuffer != NULL)
		{
			buf = pirp->UserBuffer;
		}
		else
		{
			buf = pirp->AssociatedIrp.SystemBuffer;
		}
		for (ULONG i = 0; i < length; i++)
		{
			DbgPrint("send data %2x\n", buf[i]);
		}
		IoSkipCurrentIrpStackLocation(pirp);
		return IoCallDriver(next_dev, pirp);
		break;
	default:
		pirp->IoStatus.Information = 0;
		pirp->IoStatus.Status = STATUS_INVALID_PARAMETER;
		IoCompleteRequest(pirp, IO_NO_INCREMENT);
		return STATUS_SUCCESS;
		break;
	}


}
