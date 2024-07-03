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
// This is a mess to try and have two kernel drivers which can share the same 0x4002 FPGA BAR ProdID
// but it's the only way to permit Legacy-debug testing with the newer dFLIM FPGA that permits 224MB 
// DMA sizes (original Win7 kernel driver/dFLIM FPGA (4001) was only 64 MB)
DEFINE_GUID (GUID_DEVINTERFACE_dFLIMLegacyDrv,
//    0x8b1991b8,0x66b9,0x4c5c,0xb8,0x9a,0x99,0xa1,0x1b,0xa3,0xf1,0xd5); // 2023 NWL dFLIM driver, use with PCI ProductID 0x4002
  0x822222b8,0x66b9,0x4c5c,0xb8,0x9a,0x99,0xa1,0x1b,0xa3,0xf1,0xd5); // 2023 Legacy dFLIM Win10 driver, use with PCI ProductID 0x4002 (224 MB DMA)
#endif /* THORDAQGUID_H_ */