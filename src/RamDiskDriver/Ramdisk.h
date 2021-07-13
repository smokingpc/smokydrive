
#include <ntdddisk.h>
#include <wdf.h>

NTSTATUS AssignRamdiskDeviceName(WDFDEVICE device, PWDFDEVICE_INIT DeviceInit);
NTSTATUS InitDeviceExtension(PDEVICE_EXTENSION devext);
void LoadSetting(PSMOKYDISK_SETTING setting);
BOOLEAN IsValidIoParams(IN PDEVICE_EXTENSION devExt, IN LARGE_INTEGER ByteOffset, IN size_t Length);
NTSTATUS RegisterInterface(IN WDFDEVICE dev);
//NTSTATUS RegisterRamdiskDeviceName(WDFDEVICE device, PWDFDEVICE_INIT devinit);
NTSTATUS RegisterRamdiskDeviceName(PWDFDEVICE_INIT devinit);

struct _IOCTL_FIELDS
{
    unsigned int Method : 2;
    unsigned int Function : 12;
    unsigned int Access : 2;
    unsigned int DeviceType : 16;
};

typedef union _IOCTL_CODE
{
    unsigned int Value;
    struct _IOCTL_FIELDS Fields;
} IOCTL_CODE, *PIOCTL_CODE;

//typedef struct _IOCTL_CODE
//{
//    union
//    {
//        unsigned int Value;
//        struct _IOCTL_FIELDS Fields;
//    };
//} IOCTL_CODE, *PIOCTL_CODE;
