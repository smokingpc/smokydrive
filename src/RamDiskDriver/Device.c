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

    //DECLARE_CONST_UNICODE_STRING(nt_name, NT_DEVICE_NAME);
    //status = WdfDeviceInitAssignName(DeviceInit, &nt_name);
    //if (!NT_SUCCESS(status))
    //{
    //    KdPrint(("==[SmokyDrive] WdfDeviceInitAssignName() failed 0x%08X", status));
    //    return status;
    //}

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
    
    //不知為何DeviceInit會被清成NULL，但DeviceCreate是成功的...
    status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);
    if (NT_SUCCESS(status))
    {
        if(NULL == DeviceInit)
            DeviceInit = devinit;
        status = RegisterRamdiskDeviceName(device, DeviceInit);

        devext = DeviceGetExtension(device);
        status = RamDiskDriverQueueInitialize(device);
        if (!NT_SUCCESS(status))
            KdPrint(("==[SmokyDrive] RamDiskDriverQueueInitialize() failed 0x%08X", status));

        status = InitDeviceExtension(devext);
        if (!NT_SUCCESS(status))
            KdPrint(("==[SmokyDrive] InitDeviceExtension() failed 0x%08X", status));

        status = RegisterInterface(device);
        if (!NT_SUCCESS(status))
            KdPrint(("==[SmokyDrive] RegisterInterface() failed 0x%08X", status));
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

    return STATUS_SUCCESS;
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
