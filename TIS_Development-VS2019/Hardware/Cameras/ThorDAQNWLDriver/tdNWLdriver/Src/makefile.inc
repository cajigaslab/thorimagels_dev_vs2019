#// -------------------------------------------------------------------------
#// 
#// PRODUCT:			DMA Driver
#// MODULE NAME:		makefile.inc
#// 
#// MODULE DESCRIPTION: 
#// 
#// Contains .inf make file instructions
#// 
#// $Revision:  $
#//
#// ------------------------- CONFIDENTIAL ----------------------------------
#// 
#//              Copyright (c) 2015 by Northwest Logic, Inc.   
#//                       All rights reserved. 
#// 
#// Trade Secret of Northwest Logic, Inc.  Do not disclose. 
#// 
#// Use of this source code in any form or means is permitted only 
#// with a valid, written license agreement with Northwest Logic, Inc. 
#// 
#// Licensee shall keep all information contained herein confidential  
#// and shall protect same in whole or in part from disclosure and  
#// dissemination to all third parties. 
#// 
#// 
#//                        Northwest Logic, Inc. 
#//                  1100 NW Compton Drive, Suite 100 
#//                      Beaverton, OR 97006, USA 
#//   
#//                        Ph:  +1 503 533 5800 
#//                        Fax: +1 503 533 5900 
#//                      E-Mail: info@nwlogic.com 
#//                           www.nwlogic.com 
#// 
#// -------------------------------------------------------------------------

_LNG=$(LANGUAGE)
_INX=.
STAMP=stampinf -f $@ -a $(_BUILDARCH) -d * -v $(VERSION)

$(OBJ_PATH)\$(O)\$(INF_NAME).inf: $(_INX)\$(INF_NAME).inx 
    copy $(_INX)\$(@B).inx $@
    $(STAMP)
