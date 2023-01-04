// Licensee shall keep all information contained herein confidential  
// and shall protect same in whole or in part from disclosure and  
// dissemination to all third parties. 
// 
// 
//                        Thorlabs, Inc. 
// 
// -------------------------------------------------------------------------

#ifndef THORDAQGUID_H_
#define THORDAQGUID_H_
#include <guiddef.h>

/*A pointer to the GUID for a device setup class or a device interface class. This pointer is optional and can be NULL. 
If a GUID value is not used to select devices, set ClassGuid to NULL.*/
DEFINE_GUID (GUID_DEVINTERFACE_ThorDaqDrv,
    0x823691b8,0x66b9,0x4c5c,0xb8,0x9a,0x99,0xa1,0x1b,0xa3,0xf1,0xd5);

#endif /* THORDAQGUID_H_ */