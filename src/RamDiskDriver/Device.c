/*++

Module Name:

    device.c - Device handling events for example driver.

Abstract:

   This file contains the device entry points and callbacks.
    
Environment:

    Kernel-mode Driver Framework

--*/

#include "driver.h"
#include "device.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, RamDiskDriverCreateDevice)
#endif

NTSTATUS RegisterMountMgr(PDEVICE_EXTENSION devext);
VOID WorkItemRegisterMountMgr(WDFWORKITEM WorkItem);

NTSTATUS
RamDiskDriverCreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
    )
{
    WDF_OBJECT_ATTRIBUTES   deviceAttributes;
    PDEVICE_EXTENSION devext;
    WDFDEVICE device;
    PWDFDEVICE_INIT devinit = DeviceInit;
    NTSTATUS status;
    KdPrint(("==[SmokyDrive] RamDiskDriverCreateDevice CALLed!\r\n"));

    PAGED_CODE();

    WDF_PNPPOWER_EVENT_CALLBACKS    power_callbacks;
    WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&power_callbacks);
    power_callbacks.EvtDevicePrepareHardware = RamDiskEvtDevicePrepareHardware;
    power_callbacks.EvtDeviceD0Entry = RamDiskEvtDeviceD0Entry;
    power_callbacks.EvtDeviceReleaseHardware = RamDiskEvtDeviceReleaseHardware;
    WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &power_callbacks);

    WdfDeviceInitSetDeviceType(DeviceInit, FILE_DEVICE_DISK);
    WdfDeviceInitSetIoType(DeviceInit, WdfDeviceIoDirect);
    WdfDeviceInitSetExclusive(DeviceInit, FALSE);

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_EXTENSION);
    deviceAttributes.EvtCleanupCallback = RamDiskDriverEvtDriverContextCleanup;
    //status = WdfDeviceInitAssignSDDLString(DeviceInit, &SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RWX_RES_RWX);
    //if (!NT_SUCCESS(status))
    //{
    //    KdPrint(("==[SmokyDrive] WdfDeviceInitAssignSDDLString() failed 0x%08X", status));
    //    return status;
    //}

    //不知為何DeviceInit會被清成NULL，但DeviceCreate是成功的...
    status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);
    if (NT_SUCCESS(status))
    {
        if(NULL == DeviceInit)
            DeviceInit = devinit;
        devext = DeviceGetExtension(device);

        status = RamDiskDriverQueueInitialize(device);
        if (!NT_SUCCESS(status))
            KdPrint(("==[SmokyDrive] RamDiskDriverQueueInitialize() failed 0x%08X", status));

        status = InitDeviceExtension(devext);
        if (!NT_SUCCESS(status))
            KdPrint(("==[SmokyDrive] InitDeviceExtension() failed 0x%08X", status));

        status = RegisterRamdiskDeviceName(device, DeviceInit);
    }
    else
        KdPrint(("==[SmokyDrive] WdfDeviceCreate() failed 0x%08X", status));

    return status;
}

NTSTATUS RamDiskEvtDeviceD0Entry(IN WDFDEVICE Device, IN WDF_POWER_DEVICE_STATE PreviousState)
{
    UNREFERENCED_PARAMETER(Device);
    UNREFERENCED_PARAMETER(PreviousState);
    KdPrint(("==[SmokyDrive] RamDiskEvtDeviceD0Entry CALLed!\r\n"));

    //devExt = DeviceGetExtension(Device);
    //status = CreateMountMgrWorkItem(Device);
    NTSTATUS status;
    WDF_WORKITEM_CONFIG config;
    WDF_OBJECT_ATTRIBUTES attributes;
    WDFWORKITEM workitem = NULL;

    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.ParentObject = Device;
    
    WDF_WORKITEM_CONFIG_INIT(&config, WorkItemRegisterMountMgr);
    
    status = WdfWorkItemCreate(&config, &attributes, &workitem);
    if (!NT_SUCCESS(status)) 
    {
        return status;
    }
    WdfWorkItemEnqueue(workitem);
    return status;

    //return STATUS_SUCCESS;
}

NTSTATUS RegisterMountMgr(PDEVICE_EXTENSION devext)
{
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING name;
    PFILE_OBJECT mountMgrFileObject = NULL;
    PDEVICE_OBJECT mountMgrDeviceObject = NULL;
    PMOUNTMGR_TARGET_NAME mntName = NULL;
    KEVENT event;
    ULONG mntNameLength = 0;

    RtlInitUnicodeString(&name, MOUNTMGR_DEVICE_NAME);
    status = IoGetDeviceObjectPointer(&name, FILE_READ_ATTRIBUTES, &mountMgrFileObject, &mountMgrDeviceObject);
    if (NT_SUCCESS(status)) {
        PIRP irp;
        IO_STATUS_BLOCK statusBlock;
        mntNameLength = sizeof(MOUNTMGR_TARGET_NAME) + devext->NTDeviceName.MaximumLength;
        mntName = ExAllocatePoolWithTag(NonPagedPool, mntNameLength, MY_POOLTAG);
        if (NULL == mntName) {
            KdPrint(("==[SmokyDrive] mntName == NULL!\r\n"));
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        mntName->DeviceNameLength = devext->NTDeviceName.Length;
        RtlZeroMemory(mntName->DeviceName, devext->NTDeviceName.MaximumLength);
        RtlCopyMemory(mntName->DeviceName, devext->NTDeviceName.Buffer, devext->NTDeviceName.Length);
        
        KeInitializeEvent(&event, NotificationEvent, FALSE);
        irp = IoBuildDeviceIoControlRequest(IOCTL_MOUNTMGR_VOLUME_ARRIVAL_NOTIFICATION, mountMgrDeviceObject, mntName, mntNameLength, NULL, 0, FALSE, &event, &statusBlock);
        if (irp) {
            status = IoCallDriver(mountMgrDeviceObject, irp);
            if (status == STATUS_PENDING) {
                status = KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
            }
            else
                KdPrint(("==[SmokyDrive] call to MountManager failed. status= 0x%08X\r\n", status));
        }
    }
    if (mntName) {
        ExFreePool(mntName);
    }
    return status;
}

VOID WorkItemRegisterMountMgr(WDFWORKITEM WorkItem)
{
    WDFDEVICE device;
    PDEVICE_EXTENSION devExt;
    device = WdfWorkItemGetParentObject(WorkItem);
    devExt = DeviceGetExtension(device);
    RegisterMountMgr(devExt);
    RegisterInterface(device);

    //status = RegisterInterface(device);
    //if (!NT_SUCCESS(status))
    //    KdPrint(("==[SmokyDrive] RegisterInterface() failed 0x%08X", status));
}

NTSTATUS RamDiskEvtDevicePrepareHardware(IN WDFDEVICE Device, IN WDFCMRESLIST ResourcesRaw, IN WDFCMRESLIST ResourcesTranslated)
{
    UNREFERENCED_PARAMETER(Device);
    UNREFERENCED_PARAMETER(ResourcesRaw);
    UNREFERENCED_PARAMETER(ResourcesTranslated);
    KdPrint(("==[SmokyDrive] RamDiskEvtDevicePrepareHardware CALLed!\r\n"));
    return STATUS_SUCCESS;
}
NTSTATUS RamDiskEvtDeviceReleaseHardware(IN WDFDEVICE Device, IN WDFCMRESLIST ResourceListTranslated)
{
    UNREFERENCED_PARAMETER(Device);
    UNREFERENCED_PARAMETER(ResourceListTranslated);
    KdPrint(("==[SmokyDrive] RamDiskEvtDeviceReleaseHardware CALLed!\r\n"));
    return STATUS_SUCCESS;
}
