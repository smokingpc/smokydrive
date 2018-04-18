/*++

Module Name:

    driver.h

Abstract:

    This file contains the driver definitions.

Environment:

    Kernel-mode Driver Framework

--*/

#define INITGUID

#include <ntddk.h>
#include <wdf.h>
#include <wdmsec.h> // for SDDLs

#include "device.h"
#include "queue.h"
#include "trace.h"
#include "ramdisk.h"
#include "CommonDefines.h"

//
// WDFDRIVER Events
//

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD RamDiskDriverEvtDeviceAdd;
EVT_WDF_OBJECT_CONTEXT_CLEANUP RamDiskDriverEvtDriverContextCleanup;
EVT_WDF_DRIVER_UNLOAD RamdiskDriverUnload;
