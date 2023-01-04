/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/


//
// Define an Interface Guid so that app can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_ThorDaqDrv,
    0x823691b8,0x66b9,0x4c5c,0xb8,0x9a,0x99,0xa1,0x1b,0xa3,0xf1,0xd5);
// {823691b8-66b9-4c5c-b89a-99a11ba3f1d5}
