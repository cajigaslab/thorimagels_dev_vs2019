// ThorVCMdrv.cpp : Defines the initialization routines for the DLL.
//
#ifndef FLYBACK_CYCLE_SCALE
#define FLYBACK_CYCLE_SCALE 4
#endif


#include "stdafx.h"
#include "windows.h"
#include "AlazarApi.h"
#include "AlazarCmd.h"
#include "AlazarError.h"
#include "NIDAQmx.h"
#include "ThorVCMdrv.h"
#include <math.h>
#include <direct.h>
#include "process.h"



#ifdef _DEBUG
#endif

#define PI 3.1415926535


//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//


// CThorVCMdrvApp

BEGIN_MESSAGE_MAP(CThorVCMdrvApp, CWinApp)
END_MESSAGE_MAP()


// CThorVCMdrvApp construction

CThorVCMdrvApp::CThorVCMdrvApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CThorVCMdrvApp object

CThorVCMdrvApp theApp;


// CThorVCMdrvApp initialization

BOOL CThorVCMdrvApp::InitInstance()
{
	CWinApp::InitInstance();
	InitDAQ();
	return TRUE;
}

int CThorVCMdrvApp::ExitInstance()
{
	CloseDAQ();
	return CWinApp::ExitInstance();
}


EXPORTFUN int __stdcall ThorVCMAlignReg(int ZoomV, int AlignValue)
{
	int i, j;
	char buffer1[36], buffer2[36];
	if ((ZoomV>=256)||(ZoomV<=0))
		return 1;
	for (i=1;((ZoomV>cal_ZoomV[i])&&(cal_ZoomV[i]!=0));i++); //seek through untill ZoomV <= calibration ZoomV[i];
	//_itoa(i, buffer1, 10);
	_itoa_s(i, buffer1, 36, 10);
	MessageBox(NULL, L"Current Regi Index:"+CString(buffer1), L"Calibration Information", MB_OK);
	if ((ZoomV==cal_ZoomV[i])||(cal_ZoomV[i]==0))
	{
		//directly replace the calibration value
		cal_ZoomV[i]=ZoomV;
		cal_Align[i]=AlignValue;
	}
	//else if (cal_ZoomV[i+1]==0)
	//{
	//	//simply append to the end of calibration value
	//	cal_ZoomV[i+1]=ZoomV;
	//	cal_Align[i+1]=AlignValue;
	//}
	else
	{
		//insert the calibration value into current position
		//and move data after that by one index
		for (j=255; j>i;j--)
		{
			cal_ZoomV[j]=cal_ZoomV[j-1];
			cal_Align[j]=cal_Align[j-1];
		}
		cal_ZoomV[i]=ZoomV;
		cal_Align[i]=AlignValue;
		//_itoa(i, buffer1, 10);
		_itoa_s(i, buffer1, 36, 10);
		//_itoa(cal_ZoomV[i], buffer2, 10);
		_itoa_s(cal_ZoomV[i], buffer2, 36, 10);
		MessageBox(NULL, L"Cal Align Registed, index:"+CString(buffer1)+L" zoomV:"+CString(buffer2), L"Calibration Information", MB_OK);
		//_itoa(i+1, buffer1, 10);
		_itoa_s(i+1, buffer1, 36, 10);

		//_itoa(cal_ZoomV[i+1], buffer2, 10);
		_itoa_s(cal_ZoomV[i+1], buffer2, 36, 10);
		MessageBox(NULL, L"Cal Align Registed, index:"+CString(buffer1)+L" zoomV:"+CString(buffer2), L"Calibration Information", MB_OK);

	}
	return 0;
}

EXPORTFUN void __stdcall ThorVCMFrmDisplayRGB(unsigned char* frmData, unsigned char* dispBuffer, int numChan, int numPixel)
{
	int i;
	index=0;
	switch (numChan)
	{
	case 1:
		for (i=0;i<numPixel; i++)
		{
			dispBuffer[index++]	=frmData[i];
			dispBuffer[index++]	=frmData[i];
			dispBuffer[index++]	=frmData[i];
		}
		break;
	case 2:
		for (i=0;i<numPixel; i++)
		{
			dispBuffer[index++]=0;
			dispBuffer[index++]=frmData[2*i];
			dispBuffer[index++]=frmData[2*i+1];
		}
		break;
	case 3:
		for (i=0;i<numPixel; i++)
		{
			dispBuffer[index++]=frmData[3*i];
			dispBuffer[index++]=frmData[3*i+1];
			dispBuffer[index++]=frmData[3*i+2];
		}
		break;
	}
}

EXPORTFUN int __stdcall ThorVCMGenAlignTabl()
{
	int i, j;
	if (cal_ZoomV[2]==0) //need to have at least 2 significant calibration data (First and Second), 
		//the 0th data is always 0
	{
		MessageBox(NULL, L"At least 3 calibration data points required to create Alignment table", NULL, MB_OK);
		return 1;
	}
	for (i=0;i<256;i++)
	{
		for (j=1;((i>cal_ZoomV[j])&&(cal_ZoomV[j]!=0));j++);  //search where the index sits between two calibration zoom values
		if (cal_ZoomV[j]==0) //outside of given calibration data, use exterporlation
		{//linear exterporlation
			shift_array[i]=cal_Align[j-1]
						  -(cal_ZoomV[j-1]-i)*(cal_Align[j-1]-cal_Align[j-2])
						  /(cal_ZoomV[j-1]-cal_ZoomV[j-2]);
		}
		else
		{// linear interporlation
			shift_array[i]=cal_Align[j]
						   -(cal_ZoomV[j]-i)*(cal_Align[j]-cal_Align[j-1])
							   /(cal_ZoomV[j]-cal_ZoomV[j-1]);
		}
	}
	MessageBox(NULL, L"Align Table Created", L"Calibration Information", MB_OK);
	return 0;
}

EXPORTFUN int __stdcall ThorVCMGetAlign(int ZoomV)
{
	if (ZoomV<0 ||ZoomV>255)
		return 0;
	else
		return shift_array[ZoomV];
}



EXPORTFUN int __stdcall ThorVCMGetAlignCal(int *cal_ZoomVOut, int *cal_AlignOut, int *shift_cal)
{
	int i;
	for (i=0; i<257; i++)
	{
		cal_ZoomVOut[i]=cal_ZoomV[i];
		cal_AlignOut[i]=cal_Align[i];
		shift_cal[i]=shift_array[i];
	}
	return 0;
}



EXPORTFUN void __stdcall ThorVCMGetErrorString(int32 errorCode, U32 bufferSize, char *errorString) 
{ 
    if (errorCode < -40000) 
	{
        DAQmxGetExtendedErrorInfo(errorString,bufferSize);
    }
	else if (errorCode >(-500)) 
	{
       errorCode = errorCode +1000;
       switch (errorCode) 
	   {
           case 514: strcpy_s(errorString, bufferSize, "Access Denied"); break;       
           case 515: strcpy_s(errorString, bufferSize, "Dma Channel Unavailable");break;
           case 516: strcpy_s(errorString, bufferSize, "Dma Channel Invalid");break;
           case 517: strcpy_s(errorString, bufferSize, "Dma Channel Type Error");break;
           case 518: strcpy_s(errorString, bufferSize, "Dma InProgess");break;
           case 519: strcpy_s(errorString, bufferSize, "Dma Done");break;
           case 520: strcpy_s(errorString, bufferSize, "Dma Paused");break;
           case 521: strcpy_s(errorString, bufferSize, "Dma Not Paused");break;
           case 522: strcpy_s(errorString, bufferSize, "Dma Command Invalid");break;
           case 523: strcpy_s(errorString, bufferSize, "Dma ManReady");break;
           case 524: strcpy_s(errorString, bufferSize, "Dma ManNotReady");break;
           case 525: strcpy_s(errorString, bufferSize, "Dma Invalid Channel Priority");break;
           case 526: strcpy_s(errorString, bufferSize, "Dma ManCorrupted");break;
           case 527: strcpy_s(errorString, bufferSize, "Dma Invalid Element Index");break;
           case 528: strcpy_s(errorString, bufferSize, "Dma Nomore Elements");break;
           case 529: strcpy_s(errorString, bufferSize, "Dma Signal Invalid");break;
           case 530: strcpy_s(errorString, bufferSize, "Dma Signal Queue Full");break;
           case 531: strcpy_s(errorString, bufferSize, "Null Parameter");break;
           case 532: strcpy_s(errorString, bufferSize, "Invalid Bm Index");break;
           case 533: strcpy_s(errorString, bufferSize, "Unsupported Function");break;
           case 534: strcpy_s(errorString, bufferSize, "Invalid PciSpace");break;
           case 535: strcpy_s(errorString, bufferSize, "Invalid IopSpace");break;
           case 536: strcpy_s(errorString, bufferSize, "Invalid Size");break;
           case 537: strcpy_s(errorString, bufferSize, "Invalid Address");break;
           case 538: strcpy_s(errorString, bufferSize, "Invalid Access Type");break;
           case 539: strcpy_s(errorString, bufferSize, "Invalid Index");break;
           case 540: strcpy_s(errorString, bufferSize, "Mu NotReady");break;
           case 541: strcpy_s(errorString, bufferSize, "Mu FIFO Empty");break;
           case 542: strcpy_s(errorString, bufferSize, "Mu FIFO Full");break;
           case 543: strcpy_s(errorString, bufferSize, "Invalid Register");break;
           case 544: strcpy_s(errorString, bufferSize, "Doorbell Clear Failed");break;
           case 545: strcpy_s(errorString, bufferSize, "Invalid UserPin");break;
           case 546: strcpy_s(errorString, bufferSize, "Invalid UserState");break;
           case 547: strcpy_s(errorString, bufferSize, "Eeprom Not Present");break;
           case 548: strcpy_s(errorString, bufferSize, "Eeprom Type Not Supported");break;
           case 549: strcpy_s(errorString, bufferSize, "Eeprom Blank");break;
           case 550: strcpy_s(errorString, bufferSize, "Config Access Failed");break;
           case 551: strcpy_s(errorString, bufferSize, "Invalid Device Info");break;
           case 552: strcpy_s(errorString, bufferSize, "No Active Driver");break;
           case 553: strcpy_s(errorString, bufferSize, "Insufficient Resources");break;
           case 554: strcpy_s(errorString, bufferSize, "Object Already Allocated");break;
           case 555: strcpy_s(errorString, bufferSize, "Already Initialized");break;
           case 556: strcpy_s(errorString, bufferSize, "Not Initialized");break;
           case 557: strcpy_s(errorString, bufferSize, "Bad ConfigRegEndian Mode");break;
           case 558: strcpy_s(errorString, bufferSize, "Invalid PowerState");break;
           case 559: strcpy_s(errorString, bufferSize, "Power Down");break;
           case 560: strcpy_s(errorString, bufferSize, "Flyby Not Supported");break;
           case 561: strcpy_s(errorString, bufferSize, "Not Support this Channel");break;
           case 562: strcpy_s(errorString, bufferSize, "No Action");break;
           case 563: strcpy_s(errorString, bufferSize, "HS Not Supported");break;
           case 564: strcpy_s(errorString, bufferSize, "VPD Not Supported");break;
           case 565: strcpy_s(errorString, bufferSize, "VPD Not Enabled");break;
           case 566: strcpy_s(errorString, bufferSize, "Nomore Cap");break;
           case 567: strcpy_s(errorString, bufferSize, "Invalid Offset");break;
           case 568: strcpy_s(errorString, bufferSize, "Bad Pin Direction");break;
           case 569: strcpy_s(errorString, bufferSize, "Pci Timeout");break;
           case 570: strcpy_s(errorString, bufferSize, "Dma Channel Closed");break;
           case 571: strcpy_s(errorString, bufferSize, "Dma Channel Error");break;
           case 572: strcpy_s(errorString, bufferSize, "Invalid Handle");break;
           case 573: strcpy_s(errorString, bufferSize, "Buffer Not Ready");break;
           case 574: strcpy_s(errorString, bufferSize, "Invalid Data");break;
           case 575: strcpy_s(errorString, bufferSize, "Do Nothing");break;
           case 576: strcpy_s(errorString, bufferSize, "Dma Signal Build Failed");break;
           case 577: strcpy_s(errorString, bufferSize, "PM Not Supported");break;
           case 578: strcpy_s(errorString, bufferSize, "Invalid Driver Version");break;
           case 579: strcpy_s(errorString, bufferSize, "Wait Timeout");break;
           case 580: strcpy_s(errorString, bufferSize, "Wait Canceled");break;
           case 581: strcpy_s(errorString, bufferSize, "Last Error");break;
           case 603: strcpy_s(errorString, bufferSize, "Internal DMA Trigger Buffer Not Allocated");break;
           case 604: strcpy_s(errorString, bufferSize, "AutoDMA mode is Active");break;
           case 606: strcpy_s(errorString, bufferSize, "The Record Number is Invalid");break;
           case 607: strcpy_s(errorString, bufferSize, "Record Count is too Large");break;
           case 620: strcpy_s(errorString, bufferSize, "Failed to find the First Image");break;
           case 621: strcpy_s(errorString, bufferSize, "Failed to find an Image");break;
           case 622: strcpy_s(errorString, bufferSize, "No other FPGA exists");break;
           case 624: strcpy_s(errorString, bufferSize, "Invalid Working Directory");break;
           case 626: strcpy_s(errorString, bufferSize, "Invalid Filename");break;
           case 630: strcpy_s(errorString, bufferSize, "NULL Pointer for Path");break;
           case 670: strcpy_s(errorString, bufferSize, "NULL Pointer for Handle");break;
           case 671: strcpy_s(errorString, bufferSize, "Events are not Supported");break;
           case 672: strcpy_s(errorString, bufferSize, "Events are not Active");break;
           //case 701: strcpy_s(errorString, bufferSize, "AlazarBoard not Detected"); break;  
		   case 801: strcpy_s(errorString, bufferSize, "Buffer1 is Invalid");break;
           case 802: strcpy_s(errorString, bufferSize, "Buffer2 is Invalid");break;
           case 803: strcpy_s(errorString, bufferSize, "Internal Buffer1 is Invalid");break;
           case 804: strcpy_s(errorString, bufferSize, "Internal Buffer2 is Invalid");break;
           case 805: strcpy_s(errorString, bufferSize, "Channel is Invalid");break;
           case 806: strcpy_s(errorString, bufferSize, "Header is not set");break;
           case 807: strcpy_s(errorString, bufferSize, "Header is Invalid");break;
           case 808: strcpy_s(errorString, bufferSize, "Records Per Buffer is Invalid");break;
           case 809: strcpy_s(errorString, bufferSize, "Transfer Offset is Invalid");break;
           case 810: strcpy_s(errorString, bufferSize, "CFlags is Invalid");break;
		   case 811: strcpy_s(errorString, bufferSize, "Timeout: Make sure scanner is properly connected and enabled.");break;
        }//switch
    }
	else 
	{
       errorCode = errorCode + 3000; 
       switch (errorCode) 
	   {
		   case 101: strcpy_s(errorString, bufferSize, "Voltage sent to Z stage is greater than 10"); break;
		   case 102: strcpy_s(errorString, bufferSize, "Voltage sent to Z stage is lesser than -10"); break;
		   
       }//switch
	   if (errorCode > 130 && errorCode<150) 
	   {
		   //ThorGetLastErrorMessage(errorString);
	   }
    }
	////getcwd(curDir, 256);     
}







EXPORTFUN int __stdcall ThorVCMGetFrm(unsigned char *pVCMFrmDataRGB24, unsigned char *pVCMFrmDataSave, bool frmSave)
{
	int32 error = 0;
	AUTODMA_STATUS err;
	U32	transferLength=VCMbd.RecLength;
	WhichOne=0;
	index=0;
	frmDataIndex=0;
	RecsTransferred=0;
	looping=1;
	loop_count=0;
	int i,j;
	
	try 
	{
		if (!SynDMA_In_Process)
		{
			AlazarStartAutoDMA(
				h,				// board handle
				data[0],		// data buffer
				0,				//headear or not  (0, no header)
				mode,			// Channel mode, Channel_A, Channel_B or Channel_A|Channel_B 
				VCMbd.PreDepth, // Pretrigger length, in # of samples
				transferLength, //the amount to transfer for each record
				OveralLines,	//The number of records that will be transferred into Buffer1.
				OveralLines,	//The number of records to be captured during this acquisition.
				&err,			//error info returned
				CFlags,			//Control Flags
				in1,			//reserved
				&r3,			//reserved
				&r4				//reserved
				);
			if (err !=ADMA_Completed && err!= ADMA_DMAInProgress) {
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				return ThorGetAutoDMAErrorCode(err);
			}

			AlzErrChk(AlazarStartCapture(h));
			SynDMA_In_Process=true;
		}
		while (looping == 1)
		{
			if (WhichOne==0||WhichOne==1)
			{	
				line_start_index=10;
				if (frmSave)
				{
					for (i=0; i<(int)ForwardLines; i++)
					{
						switch (mode)
						{
						case CHANNEL_A:
							for(j=line_start_index;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++)
							{
								pVCMFrmDataSave[frmDataIndex++]=
								pVCMFrmDataRGB24[index++]=clmapG[data[WhichOne][j]];
								pVCMFrmDataRGB24[index++]=clmapG[data[WhichOne][j]];
								pVCMFrmDataRGB24[index++]=clmapG[data[WhichOne][j]];
				
							}
							//back scan
							line_start_index+=VCMbd.RecLength-17;
							for(j=line_start_index;j>(line_start_index-(int)ImgPtyDll.width_pixel);j--)
							{
								pVCMFrmDataSave[frmDataIndex++]=
								pVCMFrmDataRGB24[index++]=clmapG[data[WhichOne][j]];
								pVCMFrmDataRGB24[index++]=clmapG[data[WhichOne][j]];
								pVCMFrmDataRGB24[index++]=clmapG[data[WhichOne][j]];
							}
							line_start_index+=17;
							break;
						case CHANNEL_B:

							for(j=line_start_index;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++)
							{
								pVCMFrmDataSave[frmDataIndex++]=
								pVCMFrmDataRGB24[index++]=clmapR[data[WhichOne][j]];
								pVCMFrmDataRGB24[index++]=clmapR[data[WhichOne][j]];
								pVCMFrmDataRGB24[index++]=clmapR[data[WhichOne][j]];
				
							}
							//back scan
							line_start_index+=VCMbd.RecLength-17;
							for(j=line_start_index;j>(line_start_index-(int)ImgPtyDll.width_pixel);j--)
							{
								pVCMFrmDataSave[frmDataIndex++]=
								pVCMFrmDataRGB24[index++]=clmapR[data[WhichOne][j]];
								pVCMFrmDataRGB24[index++]=clmapR[data[WhichOne][j]];
								pVCMFrmDataRGB24[index++]=clmapR[data[WhichOne][j]];
							}
							line_start_index+=17;
							break;

						case CHANNEL_A|CHANNEL_B:

							for(j=line_start_index;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++)
							{
								pVCMFrmDataRGB24[index++]=0;  //Blue
								pVCMFrmDataSave[frmDataIndex++]=
								pVCMFrmDataRGB24[index++]=clmapG[data[WhichOne][j]];  //Green
								pVCMFrmDataSave[frmDataIndex++]=
								pVCMFrmDataRGB24[index++]=clmapR[data[WhichOne][VCMbd.RecLength+j]];  //Red
				
							}
							//back scan
							line_start_index+=transferLength-17;
							for(j=line_start_index;j>(line_start_index-(int)ImgPtyDll.width_pixel);j--)
							{
								pVCMFrmDataRGB24[index++]=0;  //Blue;
								pVCMFrmDataSave[frmDataIndex++]=
								pVCMFrmDataRGB24[index++]=clmapG[data[WhichOne][j]]; //Green
								pVCMFrmDataSave[frmDataIndex++]=
								pVCMFrmDataRGB24[index++]=clmapR[data[WhichOne][VCMbd.RecLength+j]];  //Red
							}
							line_start_index+=transferLength+17;
							break;
						}
					}
				}
				else
				{
					for (i=0; i<(int)ForwardLines; i++)
					{
						switch (mode)
						{
						case CHANNEL_A:
						case CHANNEL_B:

							for(j=line_start_index;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++)
							{
								pVCMFrmDataRGB24[index++]=clmapB[data[WhichOne][j]];
								pVCMFrmDataRGB24[index++]=clmapG[data[WhichOne][j]];
								pVCMFrmDataRGB24[index++]=clmapR[data[WhichOne][j]];
							}
							//back scan
							line_start_index+=VCMbd.RecLength-17;
							for(j=line_start_index;j>(line_start_index-(int)ImgPtyDll.width_pixel);j--)
							{
								pVCMFrmDataRGB24[index++]=clmapB[data[WhichOne][j]];
								pVCMFrmDataRGB24[index++]=clmapG[data[WhichOne][j]];
								pVCMFrmDataRGB24[index++]=clmapR[data[WhichOne][j]];
							}
							line_start_index+=17;
							break;
						case CHANNEL_A|CHANNEL_B:

							for(j=line_start_index;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++)
							{
								pVCMFrmDataRGB24[index++]=0;  //Blue
								pVCMFrmDataRGB24[index++]=clmapG[data[WhichOne][j]];  //Green
								pVCMFrmDataRGB24[index++]=clmapR[data[WhichOne][VCMbd.RecLength+j]];  //Red
				
							}
							//back scan
							line_start_index+=transferLength-17;
							for(j=line_start_index;j>(line_start_index-(int)ImgPtyDll.width_pixel);j--)
							{
								pVCMFrmDataRGB24[index++]=0;  //Blue;
								pVCMFrmDataRGB24[index++]=clmapG[data[WhichOne][j]]; //Green
								pVCMFrmDataRGB24[index++]=clmapR[data[WhichOne][VCMbd.RecLength+j]];  //Red
							}
							line_start_index+=transferLength+17;
							break;
						}
					}
				}
			}
			AlazarGetNextAutoDMABuffer(h, data[0], data[0], &WhichOne, &RecsTransferred, &err, in1, in1, &TriggersOccurred, &r4);
			loop_count++;
			if (loop_count > 5000)
			{
				looping = 0;
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				SynDMA_In_Process=false;				
				return (811 - 1000);
			}
			if (WhichOne==0||WhichOne==1)
			{
				looping=0;

			}
			//An AUTODMA error has happened so abort the current acquisitiion
			/*if (err == ADMA_OverFlow)
			{
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				SynDMA_In_Process=false;
				looping = 0;
				error = 102;
			}*/
			if (err !=ADMA_Completed && err!= ADMA_DMAInProgress) {
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				SynDMA_In_Process=false;
				looping = 0;
				return ThorGetAutoDMAErrorCode(err);
			}
		}
		AlazarStartAutoDMA(
			h,				// board handle
			data[0],		// data buffer
			0,				//headear or not  (0, no header)
			mode,			// Channel mode, Channel_A, Channel_B or Channel_A|Channel_B 
			VCMbd.PreDepth, // Pretrigger length, in # of samples
			transferLength, //the amount to transfer for each record
			OveralLines,	//The number of records that will be transferred into Buffer1.
			OveralLines,	//The number of records to be captured during this acquisition.
			&err,			//error info returned
			CFlags,			//Control Flags
			in1,			//reserved
			&r3,			//reserved
			&r4				//reserved
			);
		if (err !=ADMA_Completed && err!= ADMA_DMAInProgress) {
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				return ThorGetAutoDMAErrorCode(err);
		}
		AlzErrChk(AlazarStartCapture(h));
		SynDMA_In_Process=true;
	} catch (...) {
		if (AlazarFailed(error))  
			return (error - 1000); 
	}
	return (error-512); //if success return Zero instead of 512
}


EXPORTFUN int __stdcall ThorVCMGetFrmU16(unsigned char *pVCMFrmDataRGB24, unsigned short *pVCMFrmDataU16)
{
	int32 error = 0;
	AUTODMA_STATUS err;
	U32	transferLength=VCMbd.RecLength;
	WhichOne=0;
	index=0;
	frmDataIndex=0;
	RecsTransferred=0;
	looping=1;
	loop_count=0;
	int i,j;
	
	try 
	{
		if (!SynDMA_In_Process)
		{
			AlazarStartAutoDMA(
				h,				// board handle
				data[0],		// data buffer
				0,				//headear or not  (0, no header)
				mode,			// Channel mode, Channel_A, Channel_B or Channel_A|Channel_B 
				VCMbd.PreDepth, // Pretrigger length, in # of samples
				transferLength, //the amount to transfer for each record
				OveralLines,	//The number of records that will be transferred into Buffer1.
				OveralLines,	//The number of records to be captured during this acquisition.
				&err,			//error info returned
				CFlags,			//Control Flags
				in1,			//reserved
				&r3,			//reserved
				&r4				//reserved
				);
			if (err !=ADMA_Completed && err!= ADMA_DMAInProgress) {
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				return ThorGetAutoDMAErrorCode(err);
			}

			AlzErrChk(AlazarStartCapture(h));
			SynDMA_In_Process=true;
		}
		while (looping == 1)
		{
			if (WhichOne==0||WhichOne==1)
			{	
				line_start_index=10;
				for (i=0; i<(int)ForwardLines; i++)
				{
					switch (mode)
					{
					case CHANNEL_A:
						for(j=line_start_index;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++)
						{
							pVCMFrmDataU16[frmDataIndex++]=data[0][j];
							pVCMFrmDataRGB24[index++]=clmapG[data[0][j]];
							pVCMFrmDataRGB24[index++]=clmapG[data[0][j]];
							pVCMFrmDataRGB24[index++]=clmapG[data[0][j]];
			
						}
						//back scan
						line_start_index+=VCMbd.RecLength-17;
						for(j=line_start_index;j>(line_start_index-(int)ImgPtyDll.width_pixel);j--)
						{
							pVCMFrmDataU16[frmDataIndex++]=data[0][j];
							pVCMFrmDataRGB24[index++]=clmapG[data[0][j]];
							pVCMFrmDataRGB24[index++]=clmapG[data[0][j]];
							pVCMFrmDataRGB24[index++]=clmapG[data[0][j]];
						}
						line_start_index+=17;
						break;						
					case CHANNEL_B:

						for(j=line_start_index;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++)
						{
							pVCMFrmDataU16[frmDataIndex++]=data[0][j];
							pVCMFrmDataRGB24[index++]=clmapR[data[0][j]];
							pVCMFrmDataRGB24[index++]=clmapR[data[0][j]];
							pVCMFrmDataRGB24[index++]=clmapR[data[0][j]];
			
						}
						//back scan
						line_start_index+=VCMbd.RecLength-17;
						for(j=line_start_index;j>(line_start_index-(int)ImgPtyDll.width_pixel);j--)
						{
							pVCMFrmDataU16[frmDataIndex++]=data[0][j];
							pVCMFrmDataRGB24[index++]=clmapR[data[0][j]];
							pVCMFrmDataRGB24[index++]=clmapR[data[0][j]];
							pVCMFrmDataRGB24[index++]=clmapR[data[0][j]];
						}
						line_start_index+=17;
						break;

					case CHANNEL_A|CHANNEL_B:

						for(j=line_start_index;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++)
						{
							pVCMFrmDataRGB24[index++]=0;  //Blue
							pVCMFrmDataU16[frmDataIndex++]=data[0][j];
							pVCMFrmDataRGB24[index++]=clmapG[data[0][j]];  //Green
							pVCMFrmDataU16[frmDataIndex++]=data[0][VCMbd.RecLength+j];
							pVCMFrmDataRGB24[index++]=clmapR[data[0][VCMbd.RecLength+j]];  //Red
			
						}
						//back scan
						line_start_index+=transferLength-17;
						for(j=line_start_index;j>(line_start_index-(int)ImgPtyDll.width_pixel);j--)
						{
							pVCMFrmDataRGB24[index++]=0;  //Blue;
							pVCMFrmDataU16[frmDataIndex++]=data[0][j];
							pVCMFrmDataRGB24[index++]=clmapG[data[0][j]]; //Green
							pVCMFrmDataU16[frmDataIndex++]=data[0][VCMbd.RecLength+j];
							pVCMFrmDataRGB24[index++]=clmapR[data[0][VCMbd.RecLength+j]];  //Red
						}
						line_start_index+=transferLength+17;
						break;
					}
				}
			}
			AlazarGetNextAutoDMABuffer(h, data[0], data[0], &WhichOne, &RecsTransferred, &err, in1, in1, &TriggersOccurred, &r4);
			loop_count++;
			if (loop_count > 5000)
			{
				looping = 0;
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				SynDMA_In_Process=false;				
				return (811 - 1000);
			}
			if (WhichOne==0||WhichOne==1)
			{
				looping=0;

			}
			//An AUTODMA error has happened so abort the current acquisitiion
			/*if (err == ADMA_OverFlow)
			{
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				SynDMA_In_Process=false;
				looping = 0;
				error = 102;
			}*/
			if (err !=ADMA_Completed && err!= ADMA_DMAInProgress) {
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				SynDMA_In_Process=false;
				looping = 0;
				return ThorGetAutoDMAErrorCode(err);
			}
		}
		AlazarStartAutoDMA(
			h,				// board handle
			data[0],		// data buffer
			0,				//headear or not  (0, no header)
			mode,			// Channel mode, Channel_A, Channel_B or Channel_A|Channel_B 
			VCMbd.PreDepth, // Pretrigger length, in # of samples
			transferLength, //the amount to transfer for each record
			OveralLines,	//The number of records that will be transferred into Buffer1.
			OveralLines,	//The number of records to be captured during this acquisition.
			&err,			//error info returned
			CFlags,			//Control Flags
			in1,			//reserved
			&r3,			//reserved
			&r4				//reserved
			);
		if (err !=ADMA_Completed && err!= ADMA_DMAInProgress) {
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				return ThorGetAutoDMAErrorCode(err);
		}
		AlzErrChk(AlazarStartCapture(h));
		SynDMA_In_Process=true;
	} catch (...) {
		if (AlazarFailed(error))  
			return (error - 1000); 
	}
	return (error-512);
}


EXPORTFUN int __stdcall ThorVCMGetFrmUniDirect(unsigned char *pVCMFrmDataRGB24, unsigned short *pVCMFrmDataU16)
{
	int32 error = 0;
	AUTODMA_STATUS err;
	U32	transferLength=VCMbd.RecLength;
	WhichOne=0;
	index=0;
	frmDataIndex=0;
	RecsTransferred=0;
	looping=1;
	loop_count=0;
	int i,j;
	
	try 
	{
		if (!SynDMA_In_Process)
		{
			AlazarStartAutoDMA(
				h,				// board handle
				data[0],		// data buffer
				0,				//headear or not  (0, no header)
				mode,			// Channel mode, Channel_A, Channel_B or Channel_A|Channel_B 
				VCMbd.PreDepth, // Pretrigger length, in # of samples
				transferLength, //the amount to transfer for each record
				OveralLines,	//The number of records that will be transferred into Buffer1.
				OveralLines,	//The number of records to be captured during this acquisition.
				&err,			//error info returned
				CFlags,			//Control Flags
				in1,			//reserved
				&r3,			//reserved
				&r4				//reserved
				);
			if (err !=ADMA_Completed && err!= ADMA_DMAInProgress) {
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				return ThorGetAutoDMAErrorCode(err);
			}

			AlzErrChk(AlazarStartCapture(h));
			SynDMA_In_Process=true;
		}
		while (looping == 1)
		{
			if (WhichOne==0||WhichOne==1)
			{	
				line_start_index=10;
				for (i=0; i<(int)ForwardLines; i++)
				{
					switch (mode)
					{
					case CHANNEL_A:

						for(j=line_start_index;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++)
						{
							pVCMFrmDataU16[frmDataIndex++]=data[WhichOne][j];
							pVCMFrmDataRGB24[index++]=clmapG[data[WhichOne][j]];
							pVCMFrmDataRGB24[index++]=clmapG[data[WhichOne][j]];
							pVCMFrmDataRGB24[index++]=clmapG[data[WhichOne][j]];
			
						}
						line_start_index+=VCMbd.RecLength;
						break;
					case CHANNEL_B:

						for(j=line_start_index;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++)
						{
							pVCMFrmDataU16[frmDataIndex++]=data[WhichOne][j];
							pVCMFrmDataRGB24[index++]=clmapR[data[WhichOne][j]];
							pVCMFrmDataRGB24[index++]=clmapR[data[WhichOne][j]];
							pVCMFrmDataRGB24[index++]=clmapR[data[WhichOne][j]];
			
						}
						line_start_index+=VCMbd.RecLength;
						break;

					case CHANNEL_A|CHANNEL_B:

						for(j=line_start_index;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++)
						{
							pVCMFrmDataRGB24[index++]=0;  //Blue
							pVCMFrmDataU16[frmDataIndex++]=data[WhichOne][j];
							pVCMFrmDataRGB24[index++]=clmapG[data[WhichOne][j]];  //Green
							pVCMFrmDataU16[frmDataIndex++]=data[WhichOne][VCMbd.RecLength+j];
							pVCMFrmDataRGB24[index++]=clmapR[data[WhichOne][VCMbd.RecLength+j]];  //Red
			
						}
						line_start_index+=transferLength*2;
						break;
					}
				}
			}
			AlazarGetNextAutoDMABuffer(h, data[0], data[0], &WhichOne, &RecsTransferred, &err, in1, in1, &TriggersOccurred, &r4);
			loop_count++;
			if (loop_count > 10000)
			{
				looping = 0;
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				SynDMA_In_Process=false;				
				return (811 - 1000);
			}
			if (WhichOne==0||WhichOne==1)
			{
				looping=0;

			}
			//An AUTODMA error has happened so abort the current acquisitiion
			/*if (err == ADMA_OverFlow)
			{
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				SynDMA_In_Process=false;
				looping = 0;
				error = 102;
			}*/
			if (err !=ADMA_Completed && err!= ADMA_DMAInProgress) {
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				SynDMA_In_Process=false;
				looping = 0;
				return ThorGetAutoDMAErrorCode(err);
			}
		}
		AlazarStartAutoDMA(
			h,				// board handle
			data[0],		// data buffer
			0,				//headear or not  (0, no header)
			mode,			// Channel mode, Channel_A, Channel_B or Channel_A|Channel_B 
			VCMbd.PreDepth, // Pretrigger length, in # of samples
			transferLength, //the amount to transfer for each record
			OveralLines,	//The number of records that will be transferred into Buffer1.
			OveralLines,	//The number of records to be captured during this acquisition.
			&err,			//error info returned
			CFlags,			//Control Flags
			in1,			//reserved
			&r3,			//reserved
			&r4				//reserved
			);
		if (err !=ADMA_Completed && err!= ADMA_DMAInProgress) {
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				return ThorGetAutoDMAErrorCode(err);
		}
		AlzErrChk(AlazarStartCapture(h));
		SynDMA_In_Process=true;
	} catch (...) {
		if (AlazarFailed(error))  
			return (error - 1000); 
	}
	return (error-512);
}


EXPORTFUN int __stdcall ThorVCMGetFrmFrmAve(unsigned char *pVCMFrmDataRGB24, unsigned char *pVCMFrmDataSave, bool frmSave, int numFrmAve)
{
	int32 error = 0;
	AUTODMA_STATUS err;
	U32	transferLength=VCMbd.RecLength;
	WhichOne=0;
	index=0;
	frmDataIndex=0;
	RecsTransferred=0;
	looping=1;
	loop_count=0;
	try
	{
		if (!SynDMA_In_Process)
		{
			AlazarStartAutoDMA(
				h,				// board handle
				data[0],		// data buffer
				0,				//headear or not  (0, no header)
				mode,			// Channel mode, Channel_A, Channel_B or Channel_A|Channel_B 
				VCMbd.PreDepth, // Pretrigger length, in # of samples
				transferLength, //the amount to transfer for each record
				OveralLines,	//The number of records that will be transferred into Buffer1.
				OveralLines,	//The number of records to be captured during this acquisition.
				&err,			//error info returned
				CFlags,			//Control Flags
				in1,			//reserved
				&r3,			//reserved
				&r4				//reserved
				);
			if (err !=ADMA_Completed && err!= ADMA_DMAInProgress) {
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				return ThorGetAutoDMAErrorCode(err);
			}
			AlzErrChk(AlazarStartCapture(h));
			//AlazarStartCapture(h);
			SynDMA_In_Process=true;
		}
		while (looping == 1)
		{
			if (WhichOne==0||WhichOne==1)
			{	
				line_start_index=10;
				if (frmSave)
				{
					for (i=0; i<(int)ForwardLines; i++)
					{
						switch (mode)
						{
						case CHANNEL_A:
							for(j=line_start_index;j<line_start_index+(int)ImgPtyDll.width_pixel;j++)
							{
								pVCMFrmDataSave[frmDataIndex++]=
								pVCMFrmDataRGB24[index]=(pVCMFrmDataRGB24[index++]*(numFrmAve-1)+clmapG[data[WhichOne][j]])/numFrmAve;
								pVCMFrmDataRGB24[index]=(pVCMFrmDataRGB24[index++]*(numFrmAve-1)+clmapG[data[WhichOne][j]])/numFrmAve;
								pVCMFrmDataRGB24[index]=(pVCMFrmDataRGB24[index++]*(numFrmAve-1)+clmapG[data[WhichOne][j]])/numFrmAve;
				
							}
							//back scan
							line_start_index+=VCMbd.RecLength-17;
							for(j=line_start_index;j>line_start_index-(int)ImgPtyDll.width_pixel;j--)
							{
								pVCMFrmDataSave[frmDataIndex++]=
								pVCMFrmDataRGB24[index]=(pVCMFrmDataRGB24[index++]*(numFrmAve-1)+clmapG[data[WhichOne][j]])/numFrmAve;
								pVCMFrmDataRGB24[index]=(pVCMFrmDataRGB24[index++]*(numFrmAve-1)+clmapG[data[WhichOne][j]])/numFrmAve;
								pVCMFrmDataRGB24[index]=(pVCMFrmDataRGB24[index++]*(numFrmAve-1)+clmapG[data[WhichOne][j]])/numFrmAve;
							}
							line_start_index+=17;
							break;
						case CHANNEL_B:

							for(j=line_start_index;j<line_start_index+(int)ImgPtyDll.width_pixel;j++)
							{
								pVCMFrmDataSave[frmDataIndex++]=
								pVCMFrmDataRGB24[index]=(pVCMFrmDataRGB24[index++]*(numFrmAve-1)+clmapR[data[WhichOne][j]])/numFrmAve;
								pVCMFrmDataRGB24[index]=(pVCMFrmDataRGB24[index++]*(numFrmAve-1)+clmapR[data[WhichOne][j]])/numFrmAve;
								pVCMFrmDataRGB24[index]=(pVCMFrmDataRGB24[index++]*(numFrmAve-1)+clmapR[data[WhichOne][j]])/numFrmAve;
				
							}
							//back scan
							line_start_index+=VCMbd.RecLength-17;
							for(j=line_start_index;j>line_start_index-(int)ImgPtyDll.width_pixel;j--)
							{
								pVCMFrmDataSave[frmDataIndex++]=
								pVCMFrmDataRGB24[index]=(pVCMFrmDataRGB24[index++]*(numFrmAve-1)+clmapR[data[WhichOne][j]])/numFrmAve;
								pVCMFrmDataRGB24[index]=(pVCMFrmDataRGB24[index++]*(numFrmAve-1)+clmapR[data[WhichOne][j]])/numFrmAve;
								pVCMFrmDataRGB24[index]=(pVCMFrmDataRGB24[index++]*(numFrmAve-1)+clmapR[data[WhichOne][j]])/numFrmAve;
							}
							line_start_index+=17;
							break;

						case CHANNEL_A|CHANNEL_B:

							for(j=line_start_index;j<line_start_index+(int)ImgPtyDll.width_pixel;j++)
							{
								pVCMFrmDataRGB24[index++]=0;  //Blue
								pVCMFrmDataSave[frmDataIndex++]=
								pVCMFrmDataRGB24[index]=(pVCMFrmDataRGB24[index++]*(numFrmAve-1)+clmapG[data[WhichOne][j]])/numFrmAve;  //Green
								pVCMFrmDataSave[frmDataIndex++]=
								pVCMFrmDataRGB24[index]=(pVCMFrmDataRGB24[index++]*(numFrmAve-1)+clmapR[data[WhichOne][VCMbd.RecLength+j]])/numFrmAve;  //Red
				
							}
							//back scan
							line_start_index+=transferLength-17;
							for(j=line_start_index;j>line_start_index-(int)ImgPtyDll.width_pixel;j--)
							{
								pVCMFrmDataRGB24[index++]=0;  //Blue
								pVCMFrmDataSave[frmDataIndex++]=
								pVCMFrmDataRGB24[index]=(pVCMFrmDataRGB24[index++]*(numFrmAve-1)+clmapG[data[WhichOne][j]])/numFrmAve;  //Green
								pVCMFrmDataSave[frmDataIndex++]=
								pVCMFrmDataRGB24[index]=(pVCMFrmDataRGB24[index++]*(numFrmAve-1)+clmapR[data[WhichOne][VCMbd.RecLength+j]])/numFrmAve;  //Red
							}
							line_start_index+=transferLength+17;
							break;
						}
					}
				}
				else
				{
					for (i=0; i<(int)ForwardLines; i++)
					{
						switch (mode)
						{
						case CHANNEL_A:
						case CHANNEL_B:

							for(j=line_start_index;j<line_start_index+(int)ImgPtyDll.width_pixel;j++)
							{
								pVCMFrmDataRGB24[index]=(pVCMFrmDataRGB24[index++]*(numFrmAve-1)+clmapB[data[WhichOne][j]])/numFrmAve;
								pVCMFrmDataRGB24[index]=(pVCMFrmDataRGB24[index++]*(numFrmAve-1)+clmapG[data[WhichOne][j]])/numFrmAve;
								pVCMFrmDataRGB24[index]=(pVCMFrmDataRGB24[index++]*(numFrmAve-1)+clmapR[data[WhichOne][j]])/numFrmAve;
							}
							//back scan
							line_start_index+=VCMbd.RecLength-17;
							for(j=line_start_index;j>line_start_index-(int)ImgPtyDll.width_pixel;j--)
							{
								pVCMFrmDataRGB24[index]=(pVCMFrmDataRGB24[index++]*(numFrmAve-1)+clmapB[data[WhichOne][j]])/numFrmAve;
								pVCMFrmDataRGB24[index]=(pVCMFrmDataRGB24[index++]*(numFrmAve-1)+clmapG[data[WhichOne][j]])/numFrmAve;
								pVCMFrmDataRGB24[index]=(pVCMFrmDataRGB24[index++]*(numFrmAve-1)+clmapR[data[WhichOne][j]])/numFrmAve;
							}
							line_start_index+=17;
							break;
						case CHANNEL_A|CHANNEL_B:

							for(j=line_start_index;j<line_start_index+(int)ImgPtyDll.width_pixel;j++)
							{
								pVCMFrmDataRGB24[index++]=0;  //Blue
								pVCMFrmDataRGB24[index]=(pVCMFrmDataRGB24[index++]*(numFrmAve-1)+clmapG[data[WhichOne][j]])/numFrmAve;  //Green
								pVCMFrmDataRGB24[index]=(pVCMFrmDataRGB24[index++]*(numFrmAve-1)+clmapR[data[WhichOne][VCMbd.RecLength+j]])/numFrmAve;  //Red
							}
							//back scan
							line_start_index+=transferLength-17;
							for(j=line_start_index;j>line_start_index-(int)ImgPtyDll.width_pixel;j--)
							{
								pVCMFrmDataRGB24[index++]=0;  //Blue
								pVCMFrmDataRGB24[index]=(pVCMFrmDataRGB24[index++]*(numFrmAve-1)+clmapG[data[WhichOne][j]])/numFrmAve;  //Green
								pVCMFrmDataRGB24[index]=(pVCMFrmDataRGB24[index++]*(numFrmAve-1)+clmapR[data[WhichOne][VCMbd.RecLength+j]])/numFrmAve;  //Red
							}
							line_start_index+=transferLength+17;
							break;
						}
					}
				}
			}
			AlazarGetNextAutoDMABuffer(h, data[0], data[0], &WhichOne, &RecsTransferred, &err, in1, in1, &TriggersOccurred, &r4);
			loop_count++;
			//if (loop_count > 5000)
			//{
			//	looping = 0;
			//	//AlzErrChk(AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4));
			//	AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
			//	AlazarCloseAUTODma (h);
			//	error = 103;
			//}
			if (loop_count > 5000)
			{
				looping = 0;
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				SynDMA_In_Process=false;				
				return (811 - 1000);
			}
			if (WhichOne==0||WhichOne==1)
			{
				looping=0;

			}
			//An AUTODMA error has happened so abort the current acquisitiion
			//if (err == ADMA_OverFlow)
			//{
			//	/*AlzErrChk(AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4));*/
			//	AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
			//	//AlazarCloseAUTODma (h);
			//	looping = 0;
			//	return 102;
			//}
			if (err !=ADMA_Completed && err!= ADMA_DMAInProgress) {
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				SynDMA_In_Process=false;
				looping = 0;
				return ThorGetAutoDMAErrorCode(err);
			}
		}
		AlazarStartAutoDMA(
			h,				// board handle
			data[0],		// data buffer
			0,				//headear or not  (0, no header)
			mode,			// Channel mode, Channel_A, Channel_B or Channel_A|Channel_B 
			VCMbd.PreDepth, // Pretrigger length, in # of samples
			transferLength, //the amount to transfer for each record
			OveralLines,	//The number of records that will be transferred into Buffer1.
			OveralLines,	//The number of records to be captured during this acquisition.
			&err,			//error info returned
			CFlags,			//Control Flags
			in1,			//reserved
			&r3,			//reserved
			&r4				//reserved
			);
		if (err !=ADMA_Completed && err!= ADMA_DMAInProgress) {
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				return ThorGetAutoDMAErrorCode(err);
				
		}
		AlzErrChk(AlazarStartCapture(h));
		//AlazarStartCapture(h);
		SynDMA_In_Process=true;
	} catch (...) {
		if (AlazarFailed(error))  
			return (error - 1000); 

	}
	return (error-512);
}


EXPORTFUN int __stdcall ThorVCMGetFrmAveU16(unsigned char *pVCMFrmDataRGB24, unsigned short *pVCMFrmDataU16,int numFrmAve)
{
	int32 error = 0;
	AUTODMA_STATUS err;
	U32	transferLength=VCMbd.RecLength;
	WhichOne=0;
	index=0;
	frmDataIndex=0;
	RecsTransferred=0;
	looping=1;
	loop_count=0;
	try
	{
		if (!SynDMA_In_Process)
		{
			AlazarStartAutoDMA(
				h,				// board handle
				data[0],		// data buffer
				0,				//headear or not  (0, no header)
				mode,			// Channel mode, Channel_A, Channel_B or Channel_A|Channel_B 
				VCMbd.PreDepth, // Pretrigger length, in # of samples
				transferLength, //the amount to transfer for each record
				OveralLines,	//The number of records that will be transferred into Buffer1.
				OveralLines,	//The number of records to be captured during this acquisition.
				&err,			//error info returned
				CFlags,			//Control Flags
				in1,			//reserved
				&r3,			//reserved
				&r4				//reserved
				);
			if (err !=ADMA_Completed && err!= ADMA_DMAInProgress) {
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				return ThorGetAutoDMAErrorCode(err);
			}
			AlzErrChk(AlazarStartCapture(h));
			SynDMA_In_Process=true;
		}
		while (looping == 1)
		{
			if (WhichOne==0||WhichOne==1)
			{	
				line_start_index=10;
				for (i=0; i<(int)ForwardLines; i++)
				{
					switch (mode)
					{
					case CHANNEL_A:
						for(j=line_start_index;j<line_start_index+(int)ImgPtyDll.width_pixel;j++)
						{
							pVCMFrmDataU16[frmDataIndex]=(pVCMFrmDataU16[frmDataIndex]*(numFrmAve-1)+data[WhichOne][j])/numFrmAve;
							pVCMFrmDataRGB24[index++]=clmapG[pVCMFrmDataU16[frmDataIndex]];
							pVCMFrmDataRGB24[index++]=clmapG[pVCMFrmDataU16[frmDataIndex]];
							pVCMFrmDataRGB24[index++]=clmapG[pVCMFrmDataU16[frmDataIndex++]];
			
						}
						//back scan
						line_start_index+=VCMbd.RecLength-17;
						for(j=line_start_index;j>line_start_index-(int)ImgPtyDll.width_pixel;j--)
						{
							pVCMFrmDataU16[frmDataIndex]=(pVCMFrmDataU16[frmDataIndex]*(numFrmAve-1)+data[WhichOne][j])/numFrmAve;
							pVCMFrmDataRGB24[index++]=clmapG[pVCMFrmDataU16[frmDataIndex]];;
							pVCMFrmDataRGB24[index++]=clmapG[pVCMFrmDataU16[frmDataIndex]];
							pVCMFrmDataRGB24[index++]=clmapG[pVCMFrmDataU16[frmDataIndex++]];
						}
						line_start_index+=17;
						break;
					case CHANNEL_B:

						for(j=line_start_index;j<line_start_index+(int)ImgPtyDll.width_pixel;j++)
						{
							pVCMFrmDataU16[frmDataIndex]=(pVCMFrmDataU16[frmDataIndex]*(numFrmAve-1)+data[WhichOne][j])/numFrmAve;
							pVCMFrmDataRGB24[index++]=clmapR[pVCMFrmDataU16[frmDataIndex]];
							pVCMFrmDataRGB24[index++]=clmapR[pVCMFrmDataU16[frmDataIndex]];
							pVCMFrmDataRGB24[index++]=clmapR[pVCMFrmDataU16[frmDataIndex++]];
			
						}
						//back scan
						line_start_index+=VCMbd.RecLength-17;
						for(j=line_start_index;j>line_start_index-(int)ImgPtyDll.width_pixel;j--)
						{
							pVCMFrmDataU16[frmDataIndex]=(pVCMFrmDataU16[frmDataIndex]*(numFrmAve-1)+data[WhichOne][j])/numFrmAve;
							pVCMFrmDataRGB24[index++]=clmapR[pVCMFrmDataU16[frmDataIndex]];;
							pVCMFrmDataRGB24[index++]=clmapR[pVCMFrmDataU16[frmDataIndex]];
							pVCMFrmDataRGB24[index++]=clmapR[pVCMFrmDataU16[frmDataIndex++]];
						}
						line_start_index+=17;
						break;

					case CHANNEL_A|CHANNEL_B:

						for(j=line_start_index;j<line_start_index+(int)ImgPtyDll.width_pixel;j++)
						{
							pVCMFrmDataRGB24[index++]=0;  //Blue
							pVCMFrmDataU16[frmDataIndex]=(pVCMFrmDataU16[frmDataIndex]*(numFrmAve-1)+data[WhichOne][j])/numFrmAve;
							pVCMFrmDataRGB24[index++]=clmapG[pVCMFrmDataU16[frmDataIndex++]];  //Green
							pVCMFrmDataU16[frmDataIndex]=(pVCMFrmDataU16[frmDataIndex]*(numFrmAve-1)+data[WhichOne][VCMbd.RecLength+j])/numFrmAve;
							pVCMFrmDataRGB24[index++]=clmapR[pVCMFrmDataU16[frmDataIndex++]];  //Red
			
						}
						//back scan
						line_start_index+=transferLength-17;
						for(j=line_start_index;j>line_start_index-(int)ImgPtyDll.width_pixel;j--)
						{
							pVCMFrmDataRGB24[index++]=0;  //Blue
							pVCMFrmDataU16[frmDataIndex]=(pVCMFrmDataU16[frmDataIndex]*(numFrmAve-1)+data[WhichOne][j])/numFrmAve;
							pVCMFrmDataRGB24[index++]=clmapG[pVCMFrmDataU16[frmDataIndex++]];  //Green
							pVCMFrmDataU16[frmDataIndex]=(pVCMFrmDataU16[frmDataIndex]*(numFrmAve-1)+data[WhichOne][VCMbd.RecLength+j])/numFrmAve;
							pVCMFrmDataRGB24[index++]=clmapR[pVCMFrmDataU16[frmDataIndex++]];  //Red
						}
						line_start_index+=transferLength+17;
						break;
					}
				}
			}
			AlazarGetNextAutoDMABuffer(h, data[0], data[0], &WhichOne, &RecsTransferred, &err, in1, in1, &TriggersOccurred, &r4);
			loop_count++;
			/*if (loop_count > 5000)
			{
				looping = 0;
				AlzErrChk(AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4));
				AlazarCloseAUTODma (h);
				error = 103;
			}*/
			if (loop_count > 5000)
			{
				looping = 0;
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				//SynDMA_In_Process=false;				
				return (811 - 1000);
			}
			if (WhichOne==0||WhichOne==1)
			{
				looping=0;

			}
			//An AUTODMA error has happened so abort the current acquisitiion
			/*if (err == ADMA_OverFlow)
			{
				AlzErrChk(AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4));
				//AlazarCloseAUTODma (h);
				looping = 0;
				return 102;
			}*/
			if (err !=ADMA_Completed && err!= ADMA_DMAInProgress) {
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				SynDMA_In_Process=false;
				looping = 0;
				return ThorGetAutoDMAErrorCode(err);
			}
		}
		AlazarStartAutoDMA(
			h,				// board handle
			data[0],		// data buffer
			0,				//headear or not  (0, no header)
			mode,			// Channel mode, Channel_A, Channel_B or Channel_A|Channel_B 
			VCMbd.PreDepth, // Pretrigger length, in # of samples
			transferLength, //the amount to transfer for each record
			OveralLines,	//The number of records that will be transferred into Buffer1.
			OveralLines,	//The number of records to be captured during this acquisition.
			&err,			//error info returned
			CFlags,			//Control Flags
			in1,			//reserved
			&r3,			//reserved
			&r4				//reserved
			);
		if (err !=ADMA_Completed && err!= ADMA_DMAInProgress) {
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				return ThorGetAutoDMAErrorCode(err);
		}
		AlzErrChk(AlazarStartCapture(h));
		SynDMA_In_Process=true;
	} catch (...) {
		if (AlazarFailed(error))  
			return (error - 1000); 

	}
	return (error-512);
}



EXPORTFUN int __stdcall ThorVCMGetFrmAveUniDirect(unsigned char *pVCMFrmDataRGB24, unsigned short *pVCMFrmDataU16, int numFrmAve)
{
	int32 error = 0;
	AUTODMA_STATUS err;
	U32	transferLength=VCMbd.RecLength;
	WhichOne=0;
	index=0;
	frmDataIndex=0;
	RecsTransferred=0;
	looping=1;
	loop_count=0;
	try
	{
		if (!SynDMA_In_Process)
		{
			AlazarStartAutoDMA(
				h,				// board handle
				data[0],		// data buffer
				0,				//headear or not  (0, no header)
				mode,			// Channel mode, Channel_A, Channel_B or Channel_A|Channel_B 
				VCMbd.PreDepth, // Pretrigger length, in # of samples
				transferLength, //the amount to transfer for each record
				OveralLines,	//The number of records that will be transferred into Buffer1.
				OveralLines,	//The number of records to be captured during this acquisition.
				&err,			//error info returned
				CFlags,			//Control Flags
				in1,			//reserved
				&r3,			//reserved
				&r4				//reserved
				);
			if (err !=ADMA_Completed && err!= ADMA_DMAInProgress) {
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				return ThorGetAutoDMAErrorCode(err);
			}
			AlzErrChk(AlazarStartCapture(h));
			SynDMA_In_Process=true;
		}
		while (looping == 1)
		{
			if (WhichOne==0||WhichOne==1)
			{	
				line_start_index=10;
				for (i=0; i<(int)ForwardLines; i++)
				{
					switch (mode)
					{
					case CHANNEL_A:
						for(j=line_start_index;j<line_start_index+(int)ImgPtyDll.width_pixel;j++)
						{
							pVCMFrmDataU16[frmDataIndex]=(pVCMFrmDataU16[frmDataIndex]*(numFrmAve-1)+data[WhichOne][j])/numFrmAve;
							pVCMFrmDataRGB24[index++]=clmapG[pVCMFrmDataU16[frmDataIndex]];
							pVCMFrmDataRGB24[index++]=clmapG[pVCMFrmDataU16[frmDataIndex]];
							pVCMFrmDataRGB24[index++]=clmapG[pVCMFrmDataU16[frmDataIndex++]];
						}
						line_start_index+=VCMbd.RecLength;
						break;
					case CHANNEL_B:

						for(j=line_start_index;j<line_start_index+(int)ImgPtyDll.width_pixel;j++)
						{
							pVCMFrmDataU16[frmDataIndex]=(pVCMFrmDataU16[frmDataIndex]*(numFrmAve-1)+data[WhichOne][j])/numFrmAve;
							pVCMFrmDataRGB24[index++]=clmapR[pVCMFrmDataU16[frmDataIndex]];
							pVCMFrmDataRGB24[index++]=clmapR[pVCMFrmDataU16[frmDataIndex]];
							pVCMFrmDataRGB24[index++]=clmapR[pVCMFrmDataU16[frmDataIndex++]];
						}
						line_start_index+=VCMbd.RecLength;
						break;

					case CHANNEL_A|CHANNEL_B:

						for(j=line_start_index;j<line_start_index+(int)ImgPtyDll.width_pixel;j++)
						{
							pVCMFrmDataRGB24[index++]=0;  //Blue
							pVCMFrmDataU16[frmDataIndex]=(pVCMFrmDataU16[frmDataIndex]*(numFrmAve-1)+data[WhichOne][j])/numFrmAve;
							pVCMFrmDataRGB24[index++]=clmapG[pVCMFrmDataU16[frmDataIndex++]];  //Green
							pVCMFrmDataU16[frmDataIndex]=(pVCMFrmDataU16[frmDataIndex]*(numFrmAve-1)+data[WhichOne][VCMbd.RecLength+j])/numFrmAve;
							pVCMFrmDataRGB24[index++]=clmapR[pVCMFrmDataU16[frmDataIndex++]];  //Red
						}
						line_start_index+=transferLength*2;
						break;
					}
				}
			}
			AlazarGetNextAutoDMABuffer(h, data[0], data[0], &WhichOne, &RecsTransferred, &err, in1, in1, &TriggersOccurred, &r4);
			loop_count++;
			/*if (loop_count > 5000)
			{
				looping = 0;
				AlzErrChk(AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4));
				AlazarCloseAUTODma (h);
				error = 103;
			}*/
			if (loop_count > 10000)
			{
				looping = 0;
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				SynDMA_In_Process=false;				
				return (811 - 1000);
			}
			if (WhichOne==0||WhichOne==1)
			{
				looping=0;

			}
			//An AUTODMA error has happened so abort the current acquisitiion
			/*if (err == ADMA_OverFlow)
			{
				AlzErrChk(AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4));
				//AlazarCloseAUTODma (h);
				looping = 0;
				return 102;
			}*/
			if (err !=ADMA_Completed && err!= ADMA_DMAInProgress) {
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				SynDMA_In_Process=false;
				looping = 0;
				return ThorGetAutoDMAErrorCode(err);
			}
		}
		AlazarStartAutoDMA(
			h,				// board handle
			data[0],		// data buffer
			0,				//headear or not  (0, no header)
			mode,			// Channel mode, Channel_A, Channel_B or Channel_A|Channel_B 
			VCMbd.PreDepth, // Pretrigger length, in # of samples
			transferLength, //the amount to transfer for each record
			OveralLines,	//The number of records that will be transferred into Buffer1.
			OveralLines,	//The number of records to be captured during this acquisition.
			&err,			//error info returned
			CFlags,			//Control Flags
			in1,			//reserved
			&r3,			//reserved
			&r4				//reserved
			);
		if (err !=ADMA_Completed && err!= ADMA_DMAInProgress) {
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				return ThorGetAutoDMAErrorCode(err);
		}
		AlzErrChk(AlazarStartCapture(h));
		SynDMA_In_Process=true;
	} catch (...) {
		if (AlazarFailed(error))  
			return (error - 1000); 

	}
	return (error-512);
}



unsigned __stdcall ThorVCMStartLineAve(void *thrdDataStruct)
{
	unsigned char* pVCMFrmDataRGB24=((VCMFrmData *)thrdDataStruct)->pVCMFrmDataRGB24;
	unsigned char* pVCMFrmDataSave=((VCMFrmData *)thrdDataStruct)->pVCMFrmDataSave;
	bool frmSave=((VCMFrmData *)thrdDataStruct)->frmSave;
	int numLineAve=((VCMFrmData *)thrdDataStruct)->numLineAve;
	int* pcurLineNum=((VCMFrmData *)thrdDataStruct)->pcurLineNum;
	int32 error = 0;
	AUTODMA_STATUS err;
	U32	transferLength=VCMbd.RecLength;
	WhichOne=-1;
	index=0;
	frmDataIndex=0;
	RecsTransferred=0;
	looping=1;
	loop_count=0;
	int i,j,k,l;
	curLineNum=0;  ///< use first line number as 1;
	curLineNumDummy=0;
	
	AlazarStartAutoDMA(
			h,				// board handle
			data[0],		// data buffer
			0,				//headear or not  (0, no header)
			mode,			// Channel mode, Channel_A, Channel_B or Channel_A|Channel_B 
			VCMbd.PreDepth, // Pretrigger length, in # of samples
			transferLength, //the amount to transfer for each record
			RecsPerBuffer,	//The number of records that will be transferred into Buffer1.
			OveralLines,	//The number of records to be captured during this acquisition.
			&err,			//error info returned
			CFlags,			//Control Flags
			in1,			//reserved
			&r3,			//reserved
			&r4				//reserved
			);
	
	AlazarStartCapture(h);
	while (looping == 1)
	{
		AlazarGetNextAutoDMABuffer(h, data[0], data[1], &WhichOne, &RecsTransferred, &err, in1, in1, &TriggersOccurred, &r4);
		if (WhichOne==0||WhichOne==1)
		{	
			loop_count=0;
			line_start_index=10;

			if (frmSave)
			{
				switch (mode)
				{
				case CHANNEL_A:
					for (i=0; (i<(int)(RecsPerBuffer*2/numLineAve))&&(curLineNum<(int)ImgPtyDll.height_pixel); i++)
					///<process each buffer/stripe data, i identified a line in the stripe
					///<process only forward lines
					{
						for(j=line_start_index, k=0;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++, k++)
						{
							lineBuffer1[k]=data[WhichOne][j]+data[WhichOne][VCMbd.RecLength-17+line_start_index-k];
						}
						line_start_index+=VCMbd.RecLength;
						for(l=1; l<numLineAve/2;l++)
						{
							for(j=line_start_index, k=0;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++, k++)
							{
								lineBuffer1[k]+=data[WhichOne][j]+data[WhichOne][VCMbd.RecLength-17+line_start_index-k];
							}
							line_start_index+=VCMbd.RecLength;
						}
						for(k=0;k<(int)(ImgPtyDll.width_pixel);k++)
						{
							pVCMFrmDataSave[frmDataIndex++]=
							pVCMFrmDataRGB24[index++]=clmapG[lineBuffer1[k]/numLineAve];
							pVCMFrmDataRGB24[index++]=clmapG[lineBuffer1[k]/numLineAve];
							pVCMFrmDataRGB24[index++]=clmapG[lineBuffer1[k]/numLineAve];
						}
						curLineNum++;
					}
					break;
				case CHANNEL_B:
					for (i=0; (i<(int)(RecsPerBuffer*2/numLineAve))&&(curLineNum<(int)ImgPtyDll.height_pixel); i++)
					///<process each buffer/stripe data, i identified a line in the stripe
					///<process only forward lines
					{
						for(j=line_start_index, k=0;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++, k++)
						{
							lineBuffer1[k]=data[WhichOne][j]+data[WhichOne][VCMbd.RecLength-17+line_start_index-k];
						}
						line_start_index+=VCMbd.RecLength;
						for(l=1; l<numLineAve/2;l++)
						{
							for(j=line_start_index, k=0;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++, k++)
							{
								lineBuffer1[k]+=data[WhichOne][j]+data[WhichOne][VCMbd.RecLength-17+line_start_index-k];
							}
							line_start_index+=VCMbd.RecLength;
						}
						for(k=0;k<(int)(ImgPtyDll.width_pixel);k++)
						{
							pVCMFrmDataSave[frmDataIndex++]=
							pVCMFrmDataRGB24[index++]=clmapR[lineBuffer1[k]/numLineAve];
							pVCMFrmDataRGB24[index++]=clmapR[lineBuffer1[k]/numLineAve];
							pVCMFrmDataRGB24[index++]=clmapR[lineBuffer1[k]/numLineAve];
						}
						curLineNum++;
					}
					break;
				case CHANNEL_A|CHANNEL_B:
					for (i=0; (i<(int)(RecsPerBuffer*2/numLineAve))&&(curLineNum<(int)ImgPtyDll.height_pixel); i++) 
					///<process each buffer/stripe data, i identified a line in the stripe
					///<process only forward lines
					{
						for(j=line_start_index, k=0;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++, k++)
						{
							lineBuffer1[k]=data[WhichOne][j]+data[WhichOne][VCMbd.RecLength-17+line_start_index-k];
							lineBuffer2[k]=data[WhichOne][j+VCMbd.RecLength]+data[WhichOne][2*VCMbd.RecLength-17+line_start_index-k];
						}
						line_start_index+=2*VCMbd.RecLength;
						for(l=1; l<numLineAve/2;l++)
						{
							for(j=line_start_index, k=0;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++, k++)
							{
								lineBuffer1[k]+=data[WhichOne][j]+data[WhichOne][VCMbd.RecLength-17+line_start_index-k];
								lineBuffer2[k]+=data[WhichOne][j+VCMbd.RecLength]+data[WhichOne][2*VCMbd.RecLength-17+line_start_index-k];
							}
							line_start_index+=2*VCMbd.RecLength;
						}
						for(k=0;k<(int)(ImgPtyDll.width_pixel);k++)
						{
							pVCMFrmDataRGB24[index++]=0;
							pVCMFrmDataSave[frmDataIndex++]=
							pVCMFrmDataRGB24[index++]=clmapG[lineBuffer1[k]/numLineAve];
							pVCMFrmDataSave[frmDataIndex++]=
							pVCMFrmDataRGB24[index++]=clmapR[lineBuffer2[k]/numLineAve];
						}
						curLineNum++;
					}
					break;
				}
			}
			else
			{
				switch (mode)
				{
				case CHANNEL_A:
				case CHANNEL_B:
					for (i=0; (i<(int)(RecsPerBuffer*2/numLineAve))&&(curLineNum<(int)(ImgPtyDll.height_pixel)); i++)
					///<process each buffer/stripe data, i identified a line in the stripe
					///<process only forward lines
					{
						for(j=line_start_index, k=0;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++, k++)
						{
							lineBuffer1[k]=data[WhichOne][j]+data[WhichOne][VCMbd.RecLength-17+line_start_index-k];
						}
						line_start_index+=VCMbd.RecLength;
						for(l=1; l<numLineAve/2;l++)
						{
							for(j=line_start_index, k=0;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++, k++)
							{
								lineBuffer1[k]+=data[WhichOne][j]+data[WhichOne][VCMbd.RecLength-17+line_start_index-k];
							}
							line_start_index+=VCMbd.RecLength;
						}
						for(k=0;k<(int)(ImgPtyDll.width_pixel);k++)  ///<a line data exported
						{
							pVCMFrmDataRGB24[index++]=clmapB[lineBuffer1[k]/numLineAve];
							pVCMFrmDataRGB24[index++]=clmapG[lineBuffer1[k]/numLineAve];
							pVCMFrmDataRGB24[index++]=clmapR[lineBuffer1[k]/numLineAve];
						}
						curLineNum++;
					}
					break;
				case CHANNEL_A|CHANNEL_B:
					for (i=0; (i<(int)(RecsPerBuffer*2/numLineAve))&&(curLineNum<(int)(ImgPtyDll.height_pixel)); i++) ///<process each buffer/stripe data
					///<process each buffer/stripe data, i identifies a line in the stripe
					///<process only forward lines
					{
						for(j=line_start_index, k=0;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++, k++)
						{
							lineBuffer1[k]=data[WhichOne][j]+data[WhichOne][VCMbd.RecLength-17+line_start_index-k];
							lineBuffer2[k]=data[WhichOne][j+VCMbd.RecLength]+data[WhichOne][2*VCMbd.RecLength-17+line_start_index-k];
						}
						line_start_index+=2*VCMbd.RecLength;
						for(l=1; l<numLineAve/2;l++)
						{
							for(j=line_start_index, k=0;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++, k++)
							{
								lineBuffer1[k]+=data[WhichOne][j]+data[WhichOne][VCMbd.RecLength-17+line_start_index-k];
								lineBuffer2[k]+=data[WhichOne][j+VCMbd.RecLength]+data[WhichOne][2*VCMbd.RecLength-17+line_start_index-k];
							}
							line_start_index+=2*VCMbd.RecLength;
						}
						for(k=0;k<(int)(ImgPtyDll.width_pixel);k++) ///<a line data is exported
						{
							pVCMFrmDataRGB24[index++]=0;
							pVCMFrmDataRGB24[index++]=clmapG[lineBuffer1[k]/numLineAve];
							pVCMFrmDataRGB24[index++]=clmapR[lineBuffer2[k]/numLineAve];
						}
						curLineNum++;
					}
					break;
				}
			}
			curLineNumDummy=curLineNum;
			SetEvent(hDataRefresh);///<only update the curlinenum info when a stripe of data has been transfered and processed
			
		}
		loop_count++;
		if (RecsTransferred==(long)OveralLines*numChannel)
			looping = 0;
		/*if (loop_count>5000)
		{
			looping = 0;
			AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
			AlazarCloseAUTODma (h);
			_endthreadex(loop_count);
			error = 103;
		}*/
		if (loop_count > 2000)
		{
			looping = 0;
			AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
			AlazarCloseAUTODma (h);
			_endthreadex(loop_count);
			SynDMA_In_Process=false;				
			return (811 - 1000);
		}

		//An AUTODMA error has happened so abort the current acquisitiion
		//if (err == ADMA_OverFlow)
		//{
		//	AlzErrChk(AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4));
		//	//AlazarCloseAUTODma (h);
		//	looping = 0;
		//	_endthreadex(102);
		//	return 102;
		//}
		if (err !=ADMA_Completed && err!= ADMA_DMAInProgress) {
			AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
			AlazarCloseAUTODma (h);
			//SynDMA_In_Process=false;
			looping = 0;
			_endthreadex(ThorGetAutoDMAErrorCode(err)/*102*/);				
			return ThorGetAutoDMAErrorCode(err);
		}
	}
	_endthreadex(0);	
	return error;
}

unsigned __stdcall ThorVCMStartLineAveU16(void *thrdDataStruct)
{
	unsigned char* pVCMFrmDataRGB24=((VCMFrmDataU16 *)thrdDataStruct)->pVCMFrmDataRGB24;
	unsigned short* pVCMFrmDataU16=((VCMFrmDataU16 *)thrdDataStruct)->pVCMFrmDataU16;
	int numLineAve=((VCMFrmDataU16 *)thrdDataStruct)->numLineAve;
	int* pcurLineNum=((VCMFrmDataU16 *)thrdDataStruct)->pcurLineNum;
	int32 error = 0;
	AUTODMA_STATUS err;
	U32	transferLength=VCMbd.RecLength;
	WhichOne=-1;
	index=0;
	frmDataIndex=0;
	RecsTransferred=0;
	looping=1;
	loop_count=0;
	int i,j,k,l;
	curLineNum=0;  ///< use first line number as 1;
	curLineNumDummy=0;
	
	AlazarStartAutoDMA(
			h,				// board handle
			data[0],		// data buffer
			0,				//headear or not  (0, no header)
			mode,			// Channel mode, Channel_A, Channel_B or Channel_A|Channel_B 
			VCMbd.PreDepth, // Pretrigger length, in # of samples
			transferLength, //the amount to transfer for each record
			RecsPerBuffer,	//The number of records that will be transferred into Buffer1.
			OveralLines,	//The number of records to be captured during this acquisition.
			&err,			//error info returned
			CFlags,			//Control Flags
			in1,			//reserved
			&r3,			//reserved
			&r4				//reserved
			);
	
	AlazarStartCapture(h);
	while (looping == 1)
	{
		AlazarGetNextAutoDMABuffer(h, data[0], data[1], &WhichOne, &RecsTransferred, &err, in1, in1, &TriggersOccurred, &r4);
		if (WhichOne==0||WhichOne==1)
		{	
			loop_count=0;
			line_start_index=10;
			switch (mode)
			{
			case CHANNEL_A:
				for (i=0; (i<(int)(RecsPerBuffer*2/numLineAve))&&(curLineNum<(int)ImgPtyDll.height_pixel); i++)
				///<process each buffer/stripe data, i identified a line in the stripe
				///<process only forward lines
				{
					for(j=line_start_index, k=0;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++, k++)
					{
						lineBuffer1[k]=data[WhichOne][j]+data[WhichOne][VCMbd.RecLength-17+line_start_index-k];
					}
					line_start_index+=VCMbd.RecLength;
					for(l=1; l<numLineAve/2;l++)
					{
						for(j=line_start_index, k=0;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++, k++)
						{
							lineBuffer1[k]+=data[WhichOne][j]+data[WhichOne][VCMbd.RecLength-17+line_start_index-k];
						}
						line_start_index+=VCMbd.RecLength;
					}
					for(k=0;k<(int)(ImgPtyDll.width_pixel);k++)
					{
						pVCMFrmDataU16[frmDataIndex]=(unsigned short) (lineBuffer1[k]/numLineAve);
						pVCMFrmDataRGB24[index++]=clmapG[pVCMFrmDataU16[frmDataIndex]];
						pVCMFrmDataRGB24[index++]=clmapG[pVCMFrmDataU16[frmDataIndex]];
						pVCMFrmDataRGB24[index++]=clmapG[pVCMFrmDataU16[frmDataIndex++]];
					}
					curLineNum++;
				}
				break;
			case CHANNEL_B:
				for (i=0; (i<(int)(RecsPerBuffer*2/numLineAve))&&(curLineNum<(int)ImgPtyDll.height_pixel); i++)
				///<process each buffer/stripe data, i identified a line in the stripe
				///<process only forward lines
				{
					for(j=line_start_index, k=0;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++, k++)
					{
						lineBuffer1[k]=data[WhichOne][j]+data[WhichOne][VCMbd.RecLength-17+line_start_index-k];
					}
					line_start_index+=VCMbd.RecLength;
					for(l=1; l<numLineAve/2;l++)
					{
						for(j=line_start_index, k=0;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++, k++)
						{
							lineBuffer1[k]+=data[WhichOne][j]+data[WhichOne][VCMbd.RecLength-17+line_start_index-k];
						}
						line_start_index+=VCMbd.RecLength;
					}
					for(k=0;k<(int)(ImgPtyDll.width_pixel);k++)
					{
						pVCMFrmDataU16[frmDataIndex]=(unsigned short) (lineBuffer1[k]/numLineAve);
						pVCMFrmDataRGB24[index++]=clmapR[pVCMFrmDataU16[frmDataIndex]];
						pVCMFrmDataRGB24[index++]=clmapR[pVCMFrmDataU16[frmDataIndex]];
						pVCMFrmDataRGB24[index++]=clmapR[pVCMFrmDataU16[frmDataIndex++]];
					}
					curLineNum++;
				}
				break;
			case CHANNEL_A|CHANNEL_B:
				for (i=0; (i<(int)(RecsPerBuffer*2/numLineAve))&&(curLineNum<(int)ImgPtyDll.height_pixel); i++) 
				///<process each buffer/stripe data, i identified a line in the stripe
				///<process only forward lines
				{
					for(j=line_start_index, k=0;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++, k++)
					{
						lineBuffer1[k]=data[WhichOne][j]+data[WhichOne][VCMbd.RecLength-17+line_start_index-k];
						lineBuffer2[k]=data[WhichOne][j+VCMbd.RecLength]+data[WhichOne][2*VCMbd.RecLength-17+line_start_index-k];
					}
					line_start_index+=2*VCMbd.RecLength;
					for(l=1; l<numLineAve/2;l++)
					{
						for(j=line_start_index, k=0;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++, k++)
						{
							lineBuffer1[k]+=data[WhichOne][j]+data[WhichOne][VCMbd.RecLength-17+line_start_index-k];
							lineBuffer2[k]+=data[WhichOne][j+VCMbd.RecLength]+data[WhichOne][2*VCMbd.RecLength-17+line_start_index-k];
						}
						line_start_index+=2*VCMbd.RecLength;
					}
					for(k=0;k<(int)(ImgPtyDll.width_pixel);k++)
					{
						pVCMFrmDataRGB24[index++]=0;
						pVCMFrmDataU16[frmDataIndex]=(unsigned short) (lineBuffer1[k]/numLineAve);
						pVCMFrmDataRGB24[index++]=clmapG[pVCMFrmDataU16[frmDataIndex++]];
						pVCMFrmDataU16[frmDataIndex]=(unsigned short) (lineBuffer2[k]/numLineAve);
						pVCMFrmDataRGB24[index++]=clmapR[pVCMFrmDataU16[frmDataIndex++]];
					}
					curLineNum++;
				}
				break;
			}
			curLineNumDummy=curLineNum;
			SetEvent(hDataRefresh);///<only update the curlinenum info when a stripe of data has been transfered and processed
		}
		loop_count++;
		if (RecsTransferred==(long)OveralLines*numChannel)
			looping = 0;
		/*if (loop_count>5000)
		{
			looping = 0;
			AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
			AlazarCloseAUTODma (h);
			_endthreadex(loop_count);
			error = 103;
		}*/
		if (loop_count > 5000)
		{
			looping = 0;
			AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
			AlazarCloseAUTODma (h);
			SynDMA_In_Process=false;				
			_endthreadex(loop_count);
			return (811 - 1000);
		}

		//An AUTODMA error has happened so abort the current acquisitiion
		/*if (err == ADMA_OverFlow)
		{
			AlzErrChk(AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4));
			//AlazarCloseAUTODma (h);
			looping = 0;
			_endthreadex(102);
			return 102;
		}*/
		if (err !=ADMA_Completed && err!= ADMA_DMAInProgress) {
			AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
			AlazarCloseAUTODma (h);
			SynDMA_In_Process=false;
			looping = 0;
			_endthreadex(ThorGetAutoDMAErrorCode(err)/*102*/);				
			return ThorGetAutoDMAErrorCode(err);
		}
	}
	_endthreadex(0);	
	return error;
}

unsigned __stdcall ThorVCMStartLineAveUniDirect(void *thrdDataStruct)
{
	unsigned char* pVCMFrmDataRGB24=((VCMFrmDataU16 *)thrdDataStruct)->pVCMFrmDataRGB24;
	unsigned short* pVCMFrmDataU16=((VCMFrmDataU16 *)thrdDataStruct)->pVCMFrmDataU16;
	int numLineAve=((VCMFrmDataU16 *)thrdDataStruct)->numLineAve;
	int* pcurLineNum=((VCMFrmDataU16 *)thrdDataStruct)->pcurLineNum;
	int32 error = 0;
	AUTODMA_STATUS err;
	U32	transferLength=VCMbd.RecLength;
	WhichOne=-1;
	index=0;
	frmDataIndex=0;
	RecsTransferred=0;
	looping=1;
	loop_count=0;
	int i,j,k,l;
	curLineNum=0;  ///< use first line number as 1;
	curLineNumDummy=0;
	
	AlazarStartAutoDMA(
			h,				// board handle
			data[0],		// data buffer
			0,				//headear or not  (0, no header)
			mode,			// Channel mode, Channel_A, Channel_B or Channel_A|Channel_B 
			VCMbd.PreDepth, // Pretrigger length, in # of samples
			transferLength, //the amount to transfer for each record
			RecsPerBuffer,	//The number of records that will be transferred into Buffer1.
			OveralLines,	//The number of records to be captured during this acquisition.
			&err,			//error info returned
			CFlags,			//Control Flags
			in1,			//reserved
			&r3,			//reserved
			&r4				//reserved
			);
	
	AlazarStartCapture(h);
	while (looping == 1)
	{
		AlazarGetNextAutoDMABuffer(h, data[0], data[1], &WhichOne, &RecsTransferred, &err, in1, in1, &TriggersOccurred, &r4);
		if (WhichOne==0||WhichOne==1)
		{	
			loop_count=0;
			line_start_index=10;
			switch (mode)
			{
			case CHANNEL_A:
				for (i=0; (i<(int)(RecsPerBuffer/numLineAve))&&(curLineNum<(int)ImgPtyDll.height_pixel); i++)
				///<process each buffer/stripe data, i identified a line in the stripe
				///<process only forward   
				{
					for(j=line_start_index, k=0;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++, k++)
					{
						lineBuffer1[k]=data[WhichOne][j];
					}
					line_start_index+=VCMbd.RecLength;
					for(l=1; l<numLineAve;l++)
					{
						for(j=line_start_index, k=0;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++, k++)
						{
							lineBuffer1[k]+=data[WhichOne][j];
						}
						line_start_index+=VCMbd.RecLength;
					}
					for(k=0;k<(int)(ImgPtyDll.width_pixel);k++)
					{
						pVCMFrmDataU16[frmDataIndex]=(unsigned short) (lineBuffer1[k]/numLineAve);
						pVCMFrmDataRGB24[index++]=clmapG[pVCMFrmDataU16[frmDataIndex]];
						pVCMFrmDataRGB24[index++]=clmapG[pVCMFrmDataU16[frmDataIndex]];
						pVCMFrmDataRGB24[index++]=clmapG[pVCMFrmDataU16[frmDataIndex++]];
					}
					curLineNum++;
				}
				break;
			case CHANNEL_B:
				for (i=0; (i<(int)(RecsPerBuffer/numLineAve))&&(curLineNum<(int)ImgPtyDll.height_pixel); i++)
				///<process each buffer/stripe data, i identified a line in the stripe
				///<process only forward   
				{
					for(j=line_start_index, k=0;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++, k++)
					{
						lineBuffer1[k]=data[WhichOne][j];
					}
					line_start_index+=VCMbd.RecLength;
					for(l=1; l<numLineAve;l++)
					{
						for(j=line_start_index, k=0;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++, k++)
						{
							lineBuffer1[k]+=data[WhichOne][j];
						}
						line_start_index+=VCMbd.RecLength;
					}
					for(k=0;k<(int)(ImgPtyDll.width_pixel);k++)
					{
						pVCMFrmDataU16[frmDataIndex]=(unsigned short) (lineBuffer1[k]/numLineAve);
						pVCMFrmDataRGB24[index++]=clmapR[pVCMFrmDataU16[frmDataIndex]];
						pVCMFrmDataRGB24[index++]=clmapR[pVCMFrmDataU16[frmDataIndex]];
						pVCMFrmDataRGB24[index++]=clmapR[pVCMFrmDataU16[frmDataIndex++]];
					}
					curLineNum++;
				}
				break;
			case CHANNEL_A|CHANNEL_B:
				for (i=0; (i<(int)(RecsPerBuffer/numLineAve))&&(curLineNum<(int)ImgPtyDll.height_pixel); i++) 
				///<process each buffer/stripe data, i identified a line in the stripe
				///<process only forward lines
				{
					for(j=line_start_index, k=0;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++, k++)
					{
						lineBuffer1[k]=data[WhichOne][j];
						lineBuffer2[k]=data[WhichOne][j+VCMbd.RecLength];
					}
					line_start_index+=2*VCMbd.RecLength;
					for(l=1; l<numLineAve;l++)
					{
						for(j=line_start_index, k=0;j<(line_start_index+(int)ImgPtyDll.width_pixel);j++, k++)
						{
							lineBuffer1[k]+=data[WhichOne][j];
							lineBuffer2[k]+=data[WhichOne][j+VCMbd.RecLength];
						}
						line_start_index+=2*VCMbd.RecLength;
					}
					for(k=0;k<(int)(ImgPtyDll.width_pixel);k++)
					{
						pVCMFrmDataRGB24[index++]=0;
						pVCMFrmDataU16[frmDataIndex]=(unsigned short) (lineBuffer1[k]/numLineAve);
						pVCMFrmDataRGB24[index++]=clmapG[pVCMFrmDataU16[frmDataIndex++]];
						pVCMFrmDataU16[frmDataIndex]=(unsigned short) (lineBuffer2[k]/numLineAve);
						pVCMFrmDataRGB24[index++]=clmapR[pVCMFrmDataU16[frmDataIndex++]];
					}
					curLineNum++;
				}
				break;
			}
			curLineNumDummy=curLineNum;
			SetEvent(hDataRefresh);///<only update the curlinenum info when a stripe of data has been transfered and processed
			
		}
		loop_count++;
		if (RecsTransferred==(long)OveralLines*numChannel)
			looping = 0;
		/*if (loop_count>5000)
		{
			looping = 0;
			AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
			AlazarCloseAUTODma (h);
			_endthreadex(loop_count);
			error = 103;
		}*/
		if (loop_count > 10000)
		{
			looping = 0;
			AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
			AlazarCloseAUTODma (h);
			SynDMA_In_Process=false;				
			_endthreadex(loop_count);
			return (811 - 1000);
		}

		//An AUTODMA error has happened so abort the current acquisitiion
		/*if (err == ADMA_OverFlow)
		{
			AlzErrChk(AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4));
			//AlazarCloseAUTODma (h);
			looping = 0;
			_endthreadex(102);
			return 102;
		}*/
		if (err !=ADMA_Completed && err!= ADMA_DMAInProgress) {
			AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
			AlazarCloseAUTODma (h);
			SynDMA_In_Process=false;
			looping = 0;
			_endthreadex(ThorGetAutoDMAErrorCode(err)/*102*/);				
			return ThorGetAutoDMAErrorCode(err);
		}
	}
	_endthreadex(0);	
	return error;
}

EXPORTFUN int __stdcall ThorVCMGetFrmLineAve(unsigned char *pVCMFrmDataRGB24, unsigned char *pVCMFrmDataSave, bool frmSave, int numLineAve, int *pcurLineNum)
{
	if (allowNewFrm)
	{
		WaitForSingleObject(hThread, INFINITE);
		frmDataDll.pVCMFrmDataRGB24=pVCMFrmDataRGB24;
		frmDataDll.pVCMFrmDataSave=pVCMFrmDataSave;
		frmDataDll.frmSave=frmSave;
		frmDataDll.numLineAve=numLineAve; ///<has to be an even number
		frmDataDll.pcurLineNum=pcurLineNum;
		hThread=(HANDLE) _beginthreadex(NULL, 0, &ThorVCMStartLineAve, (void *) &frmDataDll, 0, ThreadID);
		*pcurLineNum=0;
		allowNewFrm=false;
	}
	else
	{
		while (WaitForSingleObject(hDataRefresh,5)==WAIT_TIMEOUT);
		*pcurLineNum=curLineNumDummy;
		if (*pcurLineNum==ImgPtyDll.height_pixel)
		   allowNewFrm=true;
	}
	return 0;
}


EXPORTFUN int __stdcall ThorVCMGetFrmLineAveU16(unsigned char *pVCMFrmDataRGB24, unsigned short *pVCMFrmDataU16,int numLineAve, int *pcurLineNum)
{
	if (allowNewFrm)
	{
		WaitForSingleObject(hThread, INFINITE);
		frmDataU16Dll.pVCMFrmDataRGB24=pVCMFrmDataRGB24;
		frmDataU16Dll.pVCMFrmDataU16=pVCMFrmDataU16;
		frmDataU16Dll.numLineAve=numLineAve; ///<has to be an even number
		frmDataU16Dll.pcurLineNum=pcurLineNum;
		hThread=(HANDLE) _beginthreadex(NULL, 0, &ThorVCMStartLineAveU16, (void *) &frmDataU16Dll, 0, ThreadID);
		*pcurLineNum=0;
		allowNewFrm=false;
	}
	else
	{
		while (WaitForSingleObject(hDataRefresh,5)==WAIT_TIMEOUT);
		*pcurLineNum=curLineNumDummy;
		if (*pcurLineNum==ImgPtyDll.height_pixel)
		   allowNewFrm=true;
	}
	return 0;
}







EXPORTFUN int __stdcall ThorVCMGetFrmLineAveUniDirect(unsigned char *pVCMFrmDataRGB24, unsigned short *pVCMFrmDataU16, int numLineAve, int *pcurLineNum)
{
	if (allowNewFrm)
	{
		WaitForSingleObject(hThread, INFINITE);
		frmDataU16Dll.pVCMFrmDataRGB24=pVCMFrmDataRGB24;
		frmDataU16Dll.pVCMFrmDataU16=pVCMFrmDataU16;
		frmDataU16Dll.numLineAve=numLineAve; ///<has to be an even number
		frmDataU16Dll.pcurLineNum=pcurLineNum;
		hThread=(HANDLE) _beginthreadex(NULL, 0, &ThorVCMStartLineAveUniDirect, (void *) &frmDataU16Dll, 0, ThreadID);
		*pcurLineNum=0;
		allowNewFrm=false;
	}
	else
	{
		while (WaitForSingleObject(hDataRefresh,5)==WAIT_TIMEOUT);
		*pcurLineNum=curLineNumDummy;
		if (*pcurLineNum==ImgPtyDll.height_pixel)
		   allowNewFrm=true;
	}
	return 0;
}







EXPORTFUN int __stdcall ThorVCMGetFrmRGB24PixAve(unsigned char *pVCMFrmDataRGB24)
{
	int32 error = 0;
	AUTODMA_STATUS err;
	U32	transferLength=VCMbd.RecLength;
	WhichOne=0;
	index=0;
	RecsTransferred=0;
	looping=1;
	loop_count=0;
	try
	{
		AlazarStartAutoDMA(
			h,				// board handle
			data[0],		// data buffer
			0,				//headear or not  (0, no header)
			mode,			// Channel mode, Channel_A, Channel_B or Channel_A|Channel_B 
			VCMbd.PreDepth, // Pretrigger length, in # of samples
			transferLength, //the amount to transfer for each record
			OveralLines,	//The number of records that will be transferred into Buffer1.
			OveralLines,	//The number of records to be captured during this acquisition.
			&err,			//error info returned
			CFlags,			//Control Flags
			in1,			//reserved
			&r3,			//reserved
			&r4				//reserved
			);
		if (err !=ADMA_Completed && err!= ADMA_DMAInProgress) {
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				return ThorGetAutoDMAErrorCode(err);
		}
		AlzErrChk(AlazarStartCapture(h));
		while (looping == 1)
		{
			AlazarGetNextAutoDMABuffer(h, data[0], data[0], &WhichOne, &RecsTransferred, &err, in1, in1, &TriggersOccurred, &r4);
			U32 pix_Value=0;
			loop_count++;
			if (WhichOne==0||WhichOne==1)
			{	
				line_start_index=0;
				for (i=0; i<(int)ForwardLines; i++)
				{
					switch (mode)
					{
					case CHANNEL_A:
						for(j=0;j<(int)ImgPtyDll.width_pixel;j++)
						{
							pix_Value+=data[0][line_start_index+remap_index[j]-2];
							pix_Value+=data[0][line_start_index+remap_index[j]-1];
							pix_Value+=data[0][line_start_index+remap_index[j]];
							pix_Value+=data[0][line_start_index+remap_index[j]+1];
							pix_Value+=data[0][line_start_index+remap_index[j]+2];
							pix_Value/=5;
							pVCMFrmDataRGB24[index++]=clmapG[pix_Value];
							pVCMFrmDataRGB24[index++]=clmapG[pix_Value];
							pVCMFrmDataRGB24[index++]=clmapG[pix_Value];
							pix_Value=0;
						}
						//back scan
						for(j=(int)(ImgPtyDll.width_pixel*2-1);j>=(int)ImgPtyDll.width_pixel;j--)
						{
							pix_Value+=data[0][line_start_index+remap_index[j]-2];
							pix_Value+=data[0][line_start_index+remap_index[j]-1];
							pix_Value+=data[0][line_start_index+remap_index[j]];
							pix_Value+=data[0][line_start_index+remap_index[j]+1];
							pix_Value+=data[0][line_start_index+remap_index[j]+2];
							pix_Value/=5;
							pVCMFrmDataRGB24[index++]=clmapG[pix_Value];
							pVCMFrmDataRGB24[index++]=clmapG[pix_Value];
							pVCMFrmDataRGB24[index++]=clmapG[pix_Value];
							pix_Value=0;
						}
						line_start_index+=VCMbd.RecLength;
						break;
					case CHANNEL_B:
						for(j=0;j<(int)ImgPtyDll.width_pixel;j++)
						{
							pix_Value+=data[0][line_start_index+remap_index[j]-2];
							pix_Value+=data[0][line_start_index+remap_index[j]-1];
							pix_Value+=data[0][line_start_index+remap_index[j]];
							pix_Value+=data[0][line_start_index+remap_index[j]+1];
							pix_Value+=data[0][line_start_index+remap_index[j]+2];
							pix_Value/=5;
							pVCMFrmDataRGB24[index++]=clmapR[pix_Value];
							pVCMFrmDataRGB24[index++]=clmapR[pix_Value];
							pVCMFrmDataRGB24[index++]=clmapR[pix_Value];
							pix_Value=0;
						}
						//back scan
						for(j=(int)(ImgPtyDll.width_pixel*2-1);j>=(int)ImgPtyDll.width_pixel;j--)
						{
							pix_Value+=data[0][line_start_index+remap_index[j]-2];
							pix_Value+=data[0][line_start_index+remap_index[j]-1];
							pix_Value+=data[0][line_start_index+remap_index[j]];
							pix_Value+=data[0][line_start_index+remap_index[j]+1];
							pix_Value+=data[0][line_start_index+remap_index[j]+2];
							pix_Value/=5;
							pVCMFrmDataRGB24[index++]=clmapR[pix_Value];
							pVCMFrmDataRGB24[index++]=clmapR[pix_Value];
							pVCMFrmDataRGB24[index++]=clmapR[pix_Value];
							pix_Value=0;
						}
						line_start_index+=VCMbd.RecLength;
						break;
					case CHANNEL_A|CHANNEL_B:
						for(j=0;j<(int)ImgPtyDll.width_pixel;j++)
						{
							pix_Value+=data[0][line_start_index+transferLength+remap_index[j]-2];
							pix_Value+=data[0][line_start_index+transferLength+remap_index[j]-1];
							pix_Value+=data[0][line_start_index+transferLength+remap_index[j]];
							pix_Value+=data[0][line_start_index+transferLength+remap_index[j]+1];
							pix_Value+=data[0][line_start_index+transferLength+remap_index[j]+2];
							pix_Value/=5;
							pVCMFrmDataRGB24[index++]=0;
							pVCMFrmDataRGB24[index++]=clmapB[pix_Value];
							pix_Value=0;
							pix_Value+=data[0][line_start_index+remap_index[j]-2];
							pix_Value+=data[0][line_start_index+remap_index[j]-1];
							pix_Value+=data[0][line_start_index+remap_index[j]];
							pix_Value+=data[0][line_start_index+remap_index[j]+1];
							pix_Value+=data[0][line_start_index+remap_index[j]+2];
							pix_Value/=5;
							pVCMFrmDataRGB24[index++]=clmapR[pix_Value];
			
						}
						//back scan
						for(j=(int)(ImgPtyDll.width_pixel*2-1);j>=(int)(ImgPtyDll.width_pixel);j--)
						{
							pix_Value+=data[0][line_start_index+transferLength+remap_index[j]-2];
							pix_Value+=data[0][line_start_index+transferLength+remap_index[j]-1];
							pix_Value+=data[0][line_start_index+transferLength+remap_index[j]];
							pix_Value+=data[0][line_start_index+transferLength+remap_index[j]+1];
							pix_Value+=data[0][line_start_index+transferLength+remap_index[j]+2];
							pix_Value/=5;
							pVCMFrmDataRGB24[index++]=0;
							pVCMFrmDataRGB24[index++]=clmapB[pix_Value];
							pix_Value=0;
							pix_Value+=data[0][line_start_index+remap_index[j]-2];
							pix_Value+=data[0][line_start_index+remap_index[j]-1];
							pix_Value+=data[0][line_start_index+remap_index[j]];
							pix_Value+=data[0][line_start_index+remap_index[j]+1];
							pix_Value+=data[0][line_start_index+remap_index[j]+2];
							pix_Value/=5;
							pVCMFrmDataRGB24[index++]=clmapR[pix_Value];
							pix_Value=0;
						}
						line_start_index+=transferLength*2;
						break;
					}
				}
				loop_count=0;
				looping=0;
			}
			/*if (loop_count > 5000)
			{
				looping = 0;
				error = 103;
			}*/
			if (loop_count > 5000)
			{
				looping = 0;
				//AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				//SynDMA_In_Process=false;				
				return (811 - 1000);
			}
			//An AUTODMA error has happened so abort the current acquisitiion
			/*if (err == ADMA_OverFlow)
			{
				AlazarCloseAUTODma (h);
				looping = 0;
				error = 102;
			}*/
			if (err !=ADMA_Completed && err!= ADMA_DMAInProgress) {
				AlazarAbortAutoDMA(h, data[0], &err, in1, in1, &r4, &r4);
				AlazarCloseAUTODma (h);
				//SynDMA_In_Process=false;
				looping = 0;
				return ThorGetAutoDMAErrorCode(err);
			}
		}
		AlazarCloseAUTODma (h);
		SynDMA_In_Process=false;
		//return (int) transferLength;
	} catch (...) {
		if (AlazarFailed(error))  
			return (error - 1000); 
	}
	return error;
}

EXPORTFUN int* __stdcall ThorVCMGetRemap(void)
{
	return remap_index;
}




EXPORTFUN int __stdcall ThorVCMGSIZoom(int gsi_amp)
{
	//-----------------------
	//Set Digital Potitenometer cmd and value to an array
	int32 error = 0;
	try {

		DigPotCmd[0]=1;  // set digipot  CS bit to 1 and SCK bit to 0
		DigPotCmd[1]=0;  // set digipot  CS and SCK bit to 0
		int i,j;
		uInt8 BitSelector=1;
		for (i=0;i<8;i++)  
		// 11001000 (00010011 is the order)
		//is the command to tell pot to change value according to consequent 8 bits 
		{
			DigPotCmd[i*2+2]=(uInt8) (200 & BitSelector) ? 4 : 0;
			DigPotCmd[i*2+3]=(uInt8) 2+((200 & BitSelector) ? 4 : 0); //digipot latch in command data at the rising edge
			BitSelector *=2;
		}
		BitSelector=128;
		for (i=8;i<16;i++) // value represented by a 8 bit number, the most significant bit goes first
		{
			DigPotCmd[i*2+2]=(uInt8) (gsi_amp & BitSelector) ? 4 : 0;
			DigPotCmd[i*2+3]=(uInt8) 2+((gsi_amp & BitSelector) ? 4 : 0);
			BitSelector /=2;
		}
		DigPotCmd[34]=1; //set digipot CS bit to 1 and SCK bit to 0
		DigPotCmd[35]=0;
		//-------------------------

		int32		*sampsPerChanWritten=0;

		DAQmxErrChk(DAQmxCreateTask("", &taskHandleDO0));
		DAQmxErrChk(DAQmxCreateDOChan(taskHandleDO0, "Dev1/port0/line0:2", "", DAQmx_Val_ChanForAllLines));
		DAQmxErrChk(DAQmxStartTask(taskHandleDO0));
		for (i=0;i<36;i++)
		{
			for (j=0;j<20;j++)
			{
				DAQmxErrChk(DAQmxWriteDigitalU8(taskHandleDO0, 1, 1, 0, DAQmx_Val_GroupByChannel, &DigPotCmd[i], sampsPerChanWritten, NULL));
			}
			// extra digital update to hold the digital line longer to compensate for slow opto-electronic convertion
		}
		DAQmxStopTask(taskHandleDO0);
		DAQmxClearTask(taskHandleDO0);
	} catch(...) {
		if( DAQmxFailed(error)) 
			//ThorCloseNITasks();
			return error;

	}
	return error;
}
EXPORTFUN int __stdcall ThorVCMSetOffset(int offset_x, double offset_y)
{
	galvo_offset=offset_y;
	gsi_offset=offset_x;
	return 0;
}

EXPORTFUN int __stdcall ThorVCMSetBdDMA(VCMImgPty *pImgPty)
{
	int32 error = 0;
	try 
	{
		WaitForSingleObject(hThread, INFINITE);
		//ThorVCMSetLumi(pImgPty);
		ImgPtyDll=*pImgPty;
		CFlags=ADMA_TRADITIONAL_MODE|ADMA_EXTERNAL_STARTCAPTURE;
		if (SynDMA_In_Process)
		{
			AlazarCloseAUTODma(h);
			//AlazarAbortAutoDMA(h, data[0], &error, in1, in1, &r4, &r4);
			SynDMA_In_Process=false;
		}

		//Set Galvo Related Line number per frame
		ForwardLines=(ImgPtyDll.height_pixel)/2;
		if (ImgPtyDll.height_size<=1.0) //NI AO amplitude less than 1.0
			BackwardLines=FLYBACK_CYCLE_SCALE;
		else
			BackwardLines=(U32) ImgPtyDll.height_size*FLYBACK_CYCLE_SCALE;
		OveralLines=ForwardLines+BackwardLines;

		VCMbd.RecordCount=OveralLines;

		VCMbd.RecLength=ImgPtyDll.width_pixel*2+16;

		int eff_gsizoom=(int) (ImgPtyDll.width_size)+2*abs(gsi_offset);
		ThorVCMGSIZoom(eff_gsizoom);

		AlzErrChk(AlazarSetRecordSize( h
			, VCMbd.PreDepth
			, VCMbd.RecLength-VCMbd.PreDepth));

		AlzErrChk(AlazarSetCaptureClock( h
			,VCMbd.ClockSource
			,VCMbd.SampleRate
			,VCMbd.ClockEdge
			,0));

		ThorVCMSetRngA(ImgPtyDll.InputRngA);
		ThorVCMSetRngB(ImgPtyDll.InputRngB);

		//set data remaping
		ThorVCMSetRemap(ImgPtyDll.alignment+shift_array[eff_gsizoom],ImgPtyDll.width_pixel);

		switch (ImgPtyDll.ChannelMode)
		{
		case 0:
			mode=CHANNEL_A;
			numChannel=1;
			break;
		case 1:
			mode=CHANNEL_B;
			numChannel=1;
			break;
		case 2:
			mode=CHANNEL_A|CHANNEL_B;
			numChannel=2;
			break;
		}

		RecsPerBuffer=OveralLines;
		sizePerChannel = numChannel*RecsPerBuffer*VCMbd.RecLength*sizeof(U16);
		for (i=0;i<2;i++)
			data[i]= (U16*) realloc(data[i],sizePerChannel*sizeof(U16));

		U32	transferLength=VCMbd.RecLength;

		//Set Galvo driving waveform
		galvo_data_length=OveralLines*galvo_subtrigger_length;
		galvo_data_forward=ForwardLines*galvo_subtrigger_length;
		galvo_data_back=BackwardLines*galvo_subtrigger_length;
		//allocate mem space for the waveform
		galvo_waveform= (float64 *) realloc(galvo_waveform, galvo_data_length*sizeof(float64));
		//calculate the waveform
		for (i=0;i<galvo_data_forward;i++)
			galvo_waveform[i]=galvo_offset+(ImgPtyDll.height_size/galvo_data_forward)*(double)i-ImgPtyDll.height_size/2;
		for (i=galvo_data_forward;i<galvo_data_length;i++)
			galvo_waveform[i]=galvo_offset+ImgPtyDll.height_size/
			2.0*cos(PI/(double) galvo_data_back*((double)i-galvo_data_forward));
		sample_rate=crs_frequency*galvo_subtrigger_length;

		frameTrigger= (uInt8 *) realloc(frameTrigger, sizeof(uInt8)*galvo_data_length);
		frameTrigger[0]=255; //set the line7 to generate a pulse at the beginning of first line
		for (i=1;i<galvo_data_length;i++)
			frameTrigger[i]=0;

		DAQmxStopTask(taskHandleAO1);
		DAQmxClearTask(taskHandleAO1);
		DAQmxErrChk(DAQmxCreateTask("",&taskHandleAO1));
		DAQmxErrChk(DAQmxCreateAOVoltageChan(taskHandleAO1,"/Dev1/ao1","",-10.0,10.0,DAQmx_Val_Volts,NULL));
		DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandleAO1,"/Dev1/Ctr0InternalOutput",sample_rate,DAQmx_Val_Rising,DAQmx_Val_ContSamps,galvo_data_length));
		DAQmxErrChk(DAQmxWriteAnalogF64(taskHandleAO1,galvo_data_length,false,-1,DAQmx_Val_GroupByChannel,galvo_waveform,NULL,NULL));
		DAQmxErrChk(DAQmxStartTask(taskHandleAO1));

		DAQmxStopTask(taskHandleCO0);
		DAQmxClearTask(taskHandleCO0);
		DAQmxErrChk(DAQmxCreateTask("",&taskHandleCO0));
		DAQmxErrChk(DAQmxCreateCOPulseChanFreq(taskHandleCO0,"/Dev1/ctr0","",DAQmx_Val_Hz,DAQmx_Val_Low,0.0,sample_rate,0.5));
		DAQmxErrChk(DAQmxCfgImplicitTiming (taskHandleCO0, DAQmx_Val_FiniteSamps, galvo_subtrigger_length));
		DAQmxErrChk(DAQmxCfgDigEdgeStartTrig (taskHandleCO0, "/Dev1/PFI0", DAQmx_Val_Rising));
		DAQmxErrChk(DAQmxSetStartTrigRetriggerable(taskHandleCO0, true));
		DAQmxErrChk(DAQmxStartTask(taskHandleCO0));

		DAQmxStopTask(taskHandleDO1);
		DAQmxClearTask(taskHandleDO1);
		DAQmxErrChk(DAQmxCreateTask("",&taskHandleDO1));
		DAQmxErrChk(DAQmxCreateDOChan(taskHandleDO1, "Dev1/port0/line7", "", DAQmx_Val_ChanForAllLines));
		DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandleDO1,"/Dev1/Ctr0InternalOutput",sample_rate,DAQmx_Val_Rising,DAQmx_Val_ContSamps,galvo_data_length));
		DAQmxErrChk(DAQmxWriteDigitalU8(taskHandleDO1, galvo_data_length, 0, 0, DAQmx_Val_GroupByChannel, frameTrigger, NULL, NULL));
 		DAQmxErrChk(DAQmxStartTask(taskHandleDO1));
	} catch (...) {
		if( DAQmxFailed(error))
			//ThorCloseNITasks();
			return error;
		if (AlazarFailed(error))  
			return (error - 1000); 
	}

	return error;
}

EXPORTFUN int __stdcall ThorVCMSetBdDMAUniDirect(VCMImgPty *pImgPty)
{
	int32 error = 0;
	try 
	{
		WaitForSingleObject(hThread, INFINITE);
		//ThorVCMSetLumi(pImgPty);
		ImgPtyDll=*pImgPty;
		CFlags=ADMA_TRADITIONAL_MODE|ADMA_EXTERNAL_STARTCAPTURE;
		if (SynDMA_In_Process)
		{
			AlazarCloseAUTODma(h);
			//AlazarAbortAutoDMA(h, data[0], &error, in1, in1, &r4, &r4);
			SynDMA_In_Process=false;
		}

		//Set Galvo Related Line number per frame
		ForwardLines=ImgPtyDll.height_pixel; //  need to divide by 2 for Bidireciton acquisitio;
		if (ImgPtyDll.height_size<=1.0) //NI AO amplitude less than 1.0
			BackwardLines=FLYBACK_CYCLE_SCALE;
		else
			BackwardLines=(U32) ImgPtyDll.height_size*FLYBACK_CYCLE_SCALE;
		OveralLines=ForwardLines+BackwardLines;

		VCMbd.RecordCount=OveralLines;

		VCMbd.RecLength=ImgPtyDll.width_pixel+16;//ImgPtyDll.width_pixel*2+16, for bidirection 

		int eff_gsizoom=(int) (ImgPtyDll.width_size)+2*abs(gsi_offset);
		ThorVCMGSIZoom(eff_gsizoom);

		AlzErrChk(AlazarSetRecordSize( h
			, VCMbd.PreDepth
			, VCMbd.RecLength-VCMbd.PreDepth));

		AlzErrChk(AlazarSetCaptureClock( h
			,VCMbd.ClockSource
			,VCMbd.SampleRate
			,VCMbd.ClockEdge
			,0));

		ThorVCMSetRngA(ImgPtyDll.InputRngA);
		ThorVCMSetRngB(ImgPtyDll.InputRngB);

		//set data remaping
		ThorVCMSetRemapUniDirect(ImgPtyDll.alignment+shift_array[gsi_offset],ImgPtyDll.width_pixel);

		switch (ImgPtyDll.ChannelMode)
		{
		case 0:
			mode=CHANNEL_A;
			numChannel=1;
			break;
		case 1:
			mode=CHANNEL_B;
			numChannel=1;
			break;
		case 2:
			mode=CHANNEL_A|CHANNEL_B;
			numChannel=2;
			break;
		}

		RecsPerBuffer=OveralLines;
		sizePerChannel = numChannel*RecsPerBuffer*VCMbd.RecLength*sizeof(U16);
		for (i=0;i<2;i++)
			data[i]= (U16*) realloc(data[i],sizePerChannel*sizeof(U16));

		U32	transferLength=VCMbd.RecLength;

		//Set Galvo driving waveform
		galvo_data_length=OveralLines*galvo_subtrigger_length;
		galvo_data_forward=ForwardLines*galvo_subtrigger_length;
		galvo_data_back=BackwardLines*galvo_subtrigger_length;
		//allocate mem space for the waveform
		galvo_waveform= (float64 *) realloc(galvo_waveform, galvo_data_length*sizeof(float64));
		//calculate the waveform
		for (i=0;i<galvo_data_forward;i++)
			galvo_waveform[i]=galvo_offset+(ImgPtyDll.height_size/galvo_data_forward)*(double)i-ImgPtyDll.height_size/2;
		for (i=galvo_data_forward;i<galvo_data_length;i++)
			galvo_waveform[i]=galvo_offset+ImgPtyDll.height_size/
			2.0*cos(PI/(double) galvo_data_back*((double)i-galvo_data_forward));
		sample_rate=crs_frequency*galvo_subtrigger_length;

		frameTrigger= (uInt8 *) realloc(frameTrigger, sizeof(uInt8)*galvo_data_length);
		frameTrigger[0]=255; //set the line7 to generate a pulse at the beginning of first line
		for (i=1;i<galvo_data_length;i++)
			frameTrigger[i]=0;

		DAQmxStopTask(taskHandleAO1);
		DAQmxClearTask(taskHandleAO1);
		DAQmxErrChk(DAQmxCreateTask("",&taskHandleAO1));
		DAQmxErrChk(DAQmxCreateAOVoltageChan(taskHandleAO1,"/Dev1/ao1","",-10.0,10.0,DAQmx_Val_Volts,NULL));
		DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandleAO1,"/Dev1/Ctr0InternalOutput",sample_rate,DAQmx_Val_Rising,DAQmx_Val_ContSamps,galvo_data_length));
		DAQmxErrChk(DAQmxWriteAnalogF64(taskHandleAO1,galvo_data_length,false,-1,DAQmx_Val_GroupByChannel,galvo_waveform,NULL,NULL));
		DAQmxErrChk(DAQmxStartTask(taskHandleAO1));

		DAQmxStopTask(taskHandleCO0);
		DAQmxClearTask(taskHandleCO0);
		DAQmxErrChk(DAQmxCreateTask("",&taskHandleCO0));
		DAQmxErrChk(DAQmxCreateCOPulseChanFreq(taskHandleCO0,"/Dev1/ctr0","",DAQmx_Val_Hz,DAQmx_Val_Low,0.0,sample_rate,0.5));
		DAQmxErrChk(DAQmxCfgImplicitTiming (taskHandleCO0, DAQmx_Val_FiniteSamps, galvo_subtrigger_length));
		DAQmxErrChk(DAQmxCfgDigEdgeStartTrig (taskHandleCO0, "/Dev1/PFI0", DAQmx_Val_Rising));
		DAQmxErrChk(DAQmxSetStartTrigRetriggerable(taskHandleCO0, true));
		DAQmxErrChk(DAQmxStartTask(taskHandleCO0));

		DAQmxStopTask(taskHandleDO1);
		DAQmxClearTask(taskHandleDO1);
		DAQmxErrChk(DAQmxCreateTask("",&taskHandleDO1));
		DAQmxErrChk(DAQmxCreateDOChan(taskHandleDO1, "Dev1/port0/line7", "", DAQmx_Val_ChanForAllLines));
		DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandleDO1,"/Dev1/Ctr0InternalOutput",sample_rate,DAQmx_Val_Rising,DAQmx_Val_ContSamps,galvo_data_length));
		DAQmxErrChk(DAQmxWriteDigitalU8(taskHandleDO1, galvo_data_length, 0, 0, DAQmx_Val_GroupByChannel, frameTrigger, NULL, NULL));
 		DAQmxErrChk(DAQmxStartTask(taskHandleDO1));
	} catch (...) {
		if( DAQmxFailed(error)) 
			return error;
		if (AlazarFailed(error))  
			return (error - 1000); 
	}

	return error;
}

EXPORTFUN int __stdcall ThorVCMSetBdDMACal(VCMImgPty *pImgPty)
{
	int32 error = 0;
	try 
	{

		//ThorVCMSetLumi(pImgPty);
		ImgPtyDll=*pImgPty;
		CFlags=ADMA_TRADITIONAL_MODE|ADMA_EXTERNAL_STARTCAPTURE;
		if (SynDMA_In_Process)
		{
			AlazarCloseAUTODma(h);
			SynDMA_In_Process=false;
		}

		//Set Galvo Related Line number per frame
		ForwardLines=(ImgPtyDll.height_pixel)/2;
		if (ImgPtyDll.height_size<=1.0) //NI AO amplitude less than 1.0
			BackwardLines=FLYBACK_CYCLE_SCALE;
		else
			BackwardLines=(U32) ImgPtyDll.height_size*FLYBACK_CYCLE_SCALE;
		OveralLines=ForwardLines+BackwardLines;

		VCMbd.RecordCount=OveralLines;

		VCMbd.RecLength=ImgPtyDll.width_pixel*2+16;

		ThorVCMGSIZoom((int) (ImgPtyDll.width_size));

		AlzErrChk(AlazarSetRecordSize( h
			, VCMbd.PreDepth
			, VCMbd.RecLength-VCMbd.PreDepth));

		AlzErrChk(AlazarSetCaptureClock( h
			,VCMbd.ClockSource
			,VCMbd.SampleRate
			,VCMbd.ClockEdge
			,0));

		ThorVCMSetRngA(ImgPtyDll.InputRngA);
		ThorVCMSetRngB(ImgPtyDll.InputRngB);

		//set data remaping
		ThorVCMSetRemap(ImgPtyDll.alignment,ImgPtyDll.width_pixel);

		switch (ImgPtyDll.ChannelMode)
		{
		case 0:
			mode=CHANNEL_A;
			numChannel=1;
			break;
		case 1:
			mode=CHANNEL_B;
			numChannel=1;
			break;
		case 2:
			mode=CHANNEL_A|CHANNEL_B;
			numChannel=2;
			break;
		}

		RecsPerBuffer=OveralLines;
		sizePerChannel = numChannel*RecsPerBuffer*VCMbd.RecLength*sizeof(U16);
		for (int i=0;i<2;i++)
			data[i]= (U16*) realloc(data[i],sizePerChannel*sizeof(U16));

		U32	transferLength=VCMbd.RecLength;

		//Set Galvo driving waveform
		galvo_data_length=OveralLines*galvo_subtrigger_length;
		galvo_data_forward=ForwardLines*galvo_subtrigger_length;
		galvo_data_back=BackwardLines*galvo_subtrigger_length;
		//allocate mem space for the waveform
		galvo_waveform= (float64 *) realloc(galvo_waveform, galvo_data_length*sizeof(float64));
		//calculate the waveform
		for (int i=0;i<galvo_data_forward;i++)
			galvo_waveform[i]=(ImgPtyDll.height_size/galvo_data_forward)*(double)i-ImgPtyDll.height_size/2;
		for (int i=galvo_data_forward;i<galvo_data_length;i++)
			galvo_waveform[i]=ImgPtyDll.height_size/
			2.0*cos(PI/(double) galvo_data_back*((double)i-galvo_data_forward));
		sample_rate=crs_frequency*galvo_subtrigger_length;

		DAQmxStopTask(taskHandleAO1);
		DAQmxClearTask(taskHandleAO1);
		DAQmxCreateTask("",&taskHandleAO1);
		DAQmxErrChk(DAQmxCreateAOVoltageChan(taskHandleAO1,"/Dev1/ao1","",-10.0,10.0,DAQmx_Val_Volts,NULL));
		DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandleAO1,"/Dev1/Ctr0InternalOutput",sample_rate,DAQmx_Val_Rising,DAQmx_Val_ContSamps,galvo_data_length));
		DAQmxErrChk(DAQmxWriteAnalogF64(taskHandleAO1,galvo_data_length,false,-1,DAQmx_Val_GroupByChannel,galvo_waveform,NULL,NULL));
		DAQmxErrChk(DAQmxStartTask(taskHandleAO1));

		DAQmxStopTask(taskHandleCO0);
		DAQmxClearTask(taskHandleCO0);
		DAQmxErrChk(DAQmxCreateTask("",&taskHandleCO0));
		DAQmxErrChk(DAQmxCreateCOPulseChanFreq(taskHandleCO0,"/Dev1/ctr0","",DAQmx_Val_Hz,DAQmx_Val_Low,0.0,sample_rate,0.5));
		DAQmxErrChk(DAQmxCfgImplicitTiming (taskHandleCO0, DAQmx_Val_FiniteSamps, galvo_subtrigger_length));
		DAQmxErrChk(DAQmxCfgDigEdgeStartTrig (taskHandleCO0, "/Dev1/PFI0", DAQmx_Val_Rising));
		DAQmxErrChk(DAQmxSetStartTrigRetriggerable(taskHandleCO0, true));
		DAQmxErrChk(DAQmxStartTask(taskHandleCO0));
	} catch (...) {
		if( DAQmxFailed(error)) 

			return error;
		if (AlazarFailed(error))  
			return (error - 1000); 
	}

	return error;
	
}

EXPORTFUN int __stdcall ThorVCMSetBdDMALineAve(VCMImgPty *pImgPty, int numLineAve)
{
	int32 error = 0;
	try 
	{

		//ThorVCMSetLumi(pImgPty);
		ImgPtyDll=*pImgPty;
		CFlags=ADMA_TRADITIONAL_MODE|ADMA_EXTERNAL_STARTCAPTURE;
		if (WaitForSingleObject(hThread, INFINITE)==WAIT_OBJECT_0)
			SetEvent(hThread);
		else
			hThread=CreateEvent(NULL, false, true, NULL);
		if (SynDMA_In_Process)
		{
			AlazarCloseAUTODma(h);
			//AlzErrChk(AlazarAbortAutoDMA(h, data[0], &error, in1, in1, &r4, &r4));
			SynDMA_In_Process=false;
		}

		//Set Galvo Related Line numbers, 
		int linePerStrip=64; //num of displayed lines per screen refresh, to get the visual effect of slow scanning
		//the screen refresh has to be:
		//1. fast enough to have video rate
		//2. small enough to create visual scanning effect
		//3. Can be larger than y pixel number.
		//4. numLineAve has to be even;
		if (linePerStrip/2*numLineAve>crs_frequency*0.030)  // 0.030s is the human vision limit to detect motion
			linePerStrip=(int) (crs_frequency*0.030)*2/numLineAve; ///<numLineAve has to be even
		if (linePerStrip>(int) ImgPtyDll.height_pixel)
			linePerStrip=ImgPtyDll.height_pixel;

		RecsPerBuffer=linePerStrip*numLineAve/2;
		OveralLines=(1+ImgPtyDll.height_pixel/linePerStrip)*RecsPerBuffer; ///<numLineAve has to be even;
		ForwardLines=ImgPtyDll.height_pixel*numLineAve/2;
		BackwardLines=OveralLines-ForwardLines;
		if (BackwardLines<FLYBACK_CYCLE_SCALE*ImgPtyDll.height_size)
		BackwardLines+=RecsPerBuffer; ///<the flyback time has a lower limit
		OveralLines=ForwardLines+BackwardLines;
		VCMbd.RecordCount=OveralLines;
		VCMbd.RecLength=ImgPtyDll.width_pixel*2+16;

		int eff_gsizoom=(int) (ImgPtyDll.width_size)+2*abs(gsi_offset);
		ThorVCMGSIZoom(eff_gsizoom);

		AlzErrChk(AlazarSetRecordSize( h
			, VCMbd.PreDepth
			, VCMbd.RecLength-VCMbd.PreDepth));
		AlzErrChk(AlazarSetCaptureClock( h
			,VCMbd.ClockSource
			,VCMbd.SampleRate
			,VCMbd.ClockEdge
			,0));

		ThorVCMSetRngA(ImgPtyDll.InputRngA);
		ThorVCMSetRngB(ImgPtyDll.InputRngB);
		//set data remaping
		ThorVCMSetRemap(ImgPtyDll.alignment+shift_array[eff_gsizoom],ImgPtyDll.width_pixel);

		switch (ImgPtyDll.ChannelMode)
		{
		case 0:
			mode=CHANNEL_A;
			numChannel=1;
			break;
		case 1:
			mode=CHANNEL_B;
			numChannel=1;
			break;
		case 2:
			mode=CHANNEL_A|CHANNEL_B;
			numChannel=2;
			break;
		}
		sizePerChannel = numChannel*RecsPerBuffer*VCMbd.RecLength*sizeof(U16);
		for (i=0;i<2;i++)
			data[i]= (U16*) realloc(data[i],sizePerChannel*sizeof(U16));
		lineBuffer1=(U32*) realloc(lineBuffer1, ImgPtyDll.width_pixel*sizeof(U32));
		lineBuffer2=(U32*) realloc(lineBuffer2, ImgPtyDll.width_pixel*sizeof(U32));

		U32	transferLength=VCMbd.RecLength;
		allowNewFrm=true;
		hDataRefresh=CreateEvent(NULL, false, false, NULL);

		//Set Galvo driving waveform
		galvo_data_length=OveralLines*galvo_subtrigger_length;
		galvo_data_forward=ForwardLines*galvo_subtrigger_length;
		galvo_data_back=BackwardLines*galvo_subtrigger_length;
		//allocate mem space for the waveform
		galvo_waveform= (float64 *) realloc(galvo_waveform, galvo_data_length*sizeof(float64));
		//calculate the waveform
		for (i=0;i<galvo_data_forward;i++)
			galvo_waveform[i]=galvo_offset+(ImgPtyDll.height_size/galvo_data_forward)*(double)i-ImgPtyDll.height_size/2;
		for (i=galvo_data_forward;i<galvo_data_length;i++)
			galvo_waveform[i]=galvo_offset+ImgPtyDll.height_size/
			2.0*cos(PI/(double) galvo_data_back*((double)i-galvo_data_forward));
		sample_rate=crs_frequency*galvo_subtrigger_length;

		frameTrigger= (uInt8 *) realloc(frameTrigger, sizeof(uInt8)*galvo_data_length);
		frameTrigger[0]=255; //set the line7 to generate a pulse at the beginning of first line
		for (i=1;i<galvo_data_length;i++)
			frameTrigger[i]=0;

		DAQmxStopTask(taskHandleAO1);
		DAQmxClearTask(taskHandleAO1);
		DAQmxCreateTask("",&taskHandleAO1);
		DAQmxErrChk(DAQmxCreateAOVoltageChan(taskHandleAO1,"/Dev1/ao1","",-10.0,10.0,DAQmx_Val_Volts,NULL));
		DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandleAO1,"/Dev1/Ctr0InternalOutput",sample_rate,DAQmx_Val_Rising,DAQmx_Val_ContSamps,galvo_data_length));
		DAQmxErrChk(DAQmxWriteAnalogF64(taskHandleAO1,galvo_data_length,false,-1,DAQmx_Val_GroupByChannel,galvo_waveform,NULL,NULL));
		DAQmxErrChk(DAQmxStartTask(taskHandleAO1));

		DAQmxStopTask(taskHandleCO0);
		DAQmxClearTask(taskHandleCO0);
		DAQmxErrChk(DAQmxCreateTask("",&taskHandleCO0));
		DAQmxErrChk(DAQmxCreateCOPulseChanFreq(taskHandleCO0,"/Dev1/ctr0","",DAQmx_Val_Hz,DAQmx_Val_Low,0.0,sample_rate,0.5));
		DAQmxErrChk(DAQmxCfgImplicitTiming (taskHandleCO0, DAQmx_Val_FiniteSamps, galvo_subtrigger_length));
		DAQmxErrChk(DAQmxCfgDigEdgeStartTrig (taskHandleCO0, "/Dev1/PFI0", DAQmx_Val_Rising));
		DAQmxErrChk(DAQmxSetStartTrigRetriggerable(taskHandleCO0, true));
		DAQmxErrChk(DAQmxStartTask(taskHandleCO0));

		DAQmxStopTask(taskHandleDO1);
		DAQmxClearTask(taskHandleDO1);
		DAQmxErrChk(DAQmxCreateTask("",&taskHandleDO1));
		DAQmxErrChk(DAQmxCreateDOChan(taskHandleDO1, "Dev1/port0/line7", "", DAQmx_Val_ChanForAllLines));
		DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandleDO1,"/Dev1/Ctr0InternalOutput",sample_rate,DAQmx_Val_Rising,DAQmx_Val_ContSamps,galvo_data_length));
		DAQmxErrChk(DAQmxWriteDigitalU8(taskHandleDO1, galvo_data_length, 0, 0, DAQmx_Val_GroupByChannel, frameTrigger, NULL, NULL));
 		DAQmxErrChk(DAQmxStartTask(taskHandleDO1));
	} catch (...) {
		if( DAQmxFailed(error))
			return error;
		if (AlazarFailed(error))  
			return (error - 1000); 
	}

	return error;
	
}


EXPORTFUN int __stdcall ThorVCMSetBdDMALineAveUniDirect(VCMImgPty *pImgPty, int numLineAve)
{
	int32 error = 0;
	try 
	{

		//ThorVCMSetLumi(pImgPty);
		ImgPtyDll=*pImgPty;
		CFlags=ADMA_TRADITIONAL_MODE|ADMA_EXTERNAL_STARTCAPTURE;
		if (WaitForSingleObject(hThread, INFINITE)==WAIT_OBJECT_0)
			SetEvent(hThread);
		else
			hThread=CreateEvent(NULL, false, true, NULL);
		if (SynDMA_In_Process)
		{
			AlazarCloseAUTODma(h);
			//AlzErrChk(AlazarAbortAutoDMA(h, data[0], &error, in1, in1, &r4, &r4));
			SynDMA_In_Process=false;
		}

		//Set Galvo Related Line numbers, 
		int linePerStrip=64; //num of displayed lines per screen refresh, to get the visual effect of slow scanning
		//the screen refresh has to be:
		//1. fast enough to have video rate
		//2. small enough to create visual scanning effect
		//3. Can be larger than y pixel number.
		//4. numLineAve has to be even;
		if (linePerStrip*numLineAve>crs_frequency*0.030)  // 0.030s is the human vision limit to detect motion
			linePerStrip=(int) (crs_frequency*0.030)/numLineAve; ///<numLineAve has to be even
		if (linePerStrip>(int) ImgPtyDll.height_pixel)
			linePerStrip=ImgPtyDll.height_pixel;

		RecsPerBuffer=linePerStrip*numLineAve;
		OveralLines=(1+ImgPtyDll.height_pixel/linePerStrip)*RecsPerBuffer; ///<numLineAve has to be even;
		ForwardLines=ImgPtyDll.height_pixel*numLineAve;
		BackwardLines=OveralLines-ForwardLines;
		if (BackwardLines<FLYBACK_CYCLE_SCALE*ImgPtyDll.height_size)
		BackwardLines+=RecsPerBuffer; ///<the flyback time has a lower limit
		OveralLines=ForwardLines+BackwardLines;
		VCMbd.RecordCount=OveralLines;
		VCMbd.RecLength=ImgPtyDll.width_pixel+16;

		int eff_gsizoom=(int) (ImgPtyDll.width_size)+2*abs(gsi_offset);
		ThorVCMGSIZoom(eff_gsizoom);
		ThorVCMGSIZoom((int) (ImgPtyDll.width_size));
		AlzErrChk(AlazarSetRecordSize( h
			, VCMbd.PreDepth
			, VCMbd.RecLength-VCMbd.PreDepth));
		AlzErrChk(AlazarSetCaptureClock( h
			,VCMbd.ClockSource
			,VCMbd.SampleRate
			,VCMbd.ClockEdge
			,0));

		ThorVCMSetRngA(ImgPtyDll.InputRngA);
		ThorVCMSetRngB(ImgPtyDll.InputRngB);
		//set data remaping
		ThorVCMSetRemapUniDirect(ImgPtyDll.alignment+shift_array[eff_gsizoom],ImgPtyDll.width_pixel);

		switch (ImgPtyDll.ChannelMode)
		{
		case 0:
			mode=CHANNEL_A;
			numChannel=1;
			break;
		case 1:
			mode=CHANNEL_B;
			numChannel=1;
			break;
		case 2:
			mode=CHANNEL_A|CHANNEL_B;
			numChannel=2;
			break;
		}
		sizePerChannel = numChannel*RecsPerBuffer*VCMbd.RecLength*sizeof(U16);
		for (i=0;i<2;i++)
			data[i]= (U16*) realloc(data[i],sizePerChannel*sizeof(U16));
		lineBuffer1=(U32*) realloc(lineBuffer1, ImgPtyDll.width_pixel*sizeof(U32));
		lineBuffer2=(U32*) realloc(lineBuffer2, ImgPtyDll.width_pixel*sizeof(U32));

		U32	transferLength=VCMbd.RecLength;
		allowNewFrm=true;
		hDataRefresh=CreateEvent(NULL, false, false, NULL);

		//Set Galvo driving waveform
		galvo_data_length=OveralLines*galvo_subtrigger_length;
		galvo_data_forward=ForwardLines*galvo_subtrigger_length;
		galvo_data_back=BackwardLines*galvo_subtrigger_length;
		//allocate mem space for the waveform
		galvo_waveform= (float64 *) realloc(galvo_waveform, galvo_data_length*sizeof(float64));
		//calculate the waveform
		for (i=0;i<galvo_data_forward;i++)
			galvo_waveform[i]=galvo_offset+(ImgPtyDll.height_size/galvo_data_forward)*(double)i-ImgPtyDll.height_size/2;
		for (i=galvo_data_forward;i<galvo_data_length;i++)
			galvo_waveform[i]=galvo_offset+ImgPtyDll.height_size/
			2.0*cos(PI/(double) galvo_data_back*((double)i-galvo_data_forward));
		sample_rate=crs_frequency*galvo_subtrigger_length;

		frameTrigger= (uInt8 *) realloc(frameTrigger, sizeof(uInt8)*galvo_data_length);
		frameTrigger[0]=255; //set the line7 to generate a pulse at the beginning of first line
		for (i=1;i<galvo_data_length;i++)
			frameTrigger[i]=0;

		DAQmxStopTask(taskHandleAO1);
		DAQmxClearTask(taskHandleAO1);
		DAQmxCreateTask("",&taskHandleAO1);
		DAQmxErrChk(DAQmxCreateAOVoltageChan(taskHandleAO1,"/Dev1/ao1","",-10.0,10.0,DAQmx_Val_Volts,NULL));
		DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandleAO1,"/Dev1/Ctr0InternalOutput",sample_rate,DAQmx_Val_Rising,DAQmx_Val_ContSamps,galvo_data_length));
		DAQmxErrChk(DAQmxWriteAnalogF64(taskHandleAO1,galvo_data_length,false,-1,DAQmx_Val_GroupByChannel,galvo_waveform,NULL,NULL));
		DAQmxErrChk(DAQmxStartTask(taskHandleAO1));

		DAQmxStopTask(taskHandleCO0);
		DAQmxClearTask(taskHandleCO0);
		DAQmxErrChk(DAQmxCreateTask("",&taskHandleCO0));
		DAQmxErrChk(DAQmxCreateCOPulseChanFreq(taskHandleCO0,"/Dev1/ctr0","",DAQmx_Val_Hz,DAQmx_Val_Low,0.0,sample_rate,0.5));
		DAQmxErrChk(DAQmxCfgImplicitTiming (taskHandleCO0, DAQmx_Val_FiniteSamps, galvo_subtrigger_length));
		DAQmxErrChk(DAQmxCfgDigEdgeStartTrig (taskHandleCO0, "/Dev1/PFI0", DAQmx_Val_Rising));
		DAQmxErrChk(DAQmxSetStartTrigRetriggerable(taskHandleCO0, true));
		DAQmxErrChk(DAQmxStartTask(taskHandleCO0));

		DAQmxStopTask(taskHandleDO1);
		DAQmxClearTask(taskHandleDO1);
		DAQmxErrChk(DAQmxCreateTask("",&taskHandleDO1));
		DAQmxErrChk(DAQmxCreateDOChan(taskHandleDO1, "Dev1/port0/line7", "", DAQmx_Val_ChanForAllLines));
		DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandleDO1,"/Dev1/Ctr0InternalOutput",sample_rate,DAQmx_Val_Rising,DAQmx_Val_ContSamps,galvo_data_length));
		DAQmxErrChk(DAQmxWriteDigitalU8(taskHandleDO1, galvo_data_length, 0, 0, DAQmx_Val_GroupByChannel, frameTrigger, NULL, NULL));
 		DAQmxErrChk(DAQmxStartTask(taskHandleDO1));
	} catch (...) {
		if( DAQmxFailed(error)) 
			return error;
		if (AlazarFailed(error))  
			return (error - 1000); 
	}

	return error;
	
}



EXPORTFUN int __stdcall ThorVCMSetBdPixAve(VCMImgPty *pImgPty)
{	
	int32 error = 0;
	try
	{
		WaitForSingleObject(hThread, INFINITE);
		//ThorVCMSetLumi(pImgPty);
		ImgPtyDll=*pImgPty;
		CFlags=ADMA_TRADITIONAL_MODE|ADMA_EXTERNAL_STARTCAPTURE;
		if (SynDMA_In_Process)
		{
			AlazarCloseAUTODma(h);
			SynDMA_In_Process=false;
		}

		/*****************************************/
		//Reset (disable) data skipping enable bit Reg_03[3]=0
		U32 Reg_03;
		AlzErrChk(AlazarReadRegister(h,3,&Reg_03,0x32145876));
		Reg_03=Reg_03& ~0x00000008;
		AlzErrChk(AlazarWriteRegister(h,3,Reg_03,0x32145876));
		/*****************************************/

		//*********************************
		//Set Galvo Related Line number per frame
		ForwardLines=(ImgPtyDll.height_pixel)/2;
		if (ImgPtyDll.height_size<=1.0) //NI AO amplitude less than 1.0
			BackwardLines=FLYBACK_CYCLE_SCALE;
		else
			BackwardLines=(U32) ImgPtyDll.height_size*FLYBACK_CYCLE_SCALE;
		OveralLines=ForwardLines+BackwardLines;

		VCMbd.RecordCount=OveralLines;

		VCMbd.RecLength=pixel_clock_length;//ImgPtyDll.width_pixel*2;

		int eff_gsizoom=(int) (ImgPtyDll.width_size)+2*abs(gsi_offset);
		ThorVCMGSIZoom(eff_gsizoom);

		AlzErrChk(AlazarSetRecordSize( h
			, VCMbd.PreDepth
			, VCMbd.RecLength-VCMbd.PreDepth));

		AlzErrChk(AlazarSetCaptureClock( h
			,VCMbd.ClockSource
			,VCMbd.SampleRate
			,VCMbd.ClockEdge
			,0));

		ThorVCMSetRngA(ImgPtyDll.InputRngA);
		ThorVCMSetRngB(ImgPtyDll.InputRngB);

		//set data remaping
		ThorVCMSetRemapPixAve(ImgPtyDll.alignment+shift_array[eff_gsizoom],ImgPtyDll.width_pixel);

		switch (ImgPtyDll.ChannelMode)
		{
		case 0:
			mode=CHANNEL_A;
			numChannel=1;
			break;
		case 1:
			mode=CHANNEL_B;
			numChannel=1;
			break;
		case 2:
			mode=CHANNEL_A|CHANNEL_B;
			numChannel=2;
			break;
		}

		RecsPerBuffer=OveralLines;
		sizePerChannel = numChannel*RecsPerBuffer*VCMbd.RecLength*sizeof(U16);
		for (int i=0;i<2;i++)
			data[i]= (U16*) realloc(data[i],sizePerChannel*sizeof(U16));

		//U32	transferLength=VCMbd.RecLength;

		//Set Galvo driving waveform
		galvo_data_length=OveralLines*galvo_subtrigger_length;
		galvo_data_forward=ForwardLines*galvo_subtrigger_length;
		galvo_data_back=BackwardLines*galvo_subtrigger_length;
		//allocate mem space for the waveform
		galvo_waveform= (float64 *) realloc(galvo_waveform, galvo_data_length*sizeof(float64));
		//calculate the waveform
		for (int i=0;i<galvo_data_forward;i++)
			galvo_waveform[i]=galvo_offset+(ImgPtyDll.height_size/galvo_data_forward)*(double)i-ImgPtyDll.height_size/2;
		for (int i=galvo_data_forward;i<galvo_data_length;i++)
			galvo_waveform[i]=galvo_offset+ImgPtyDll.height_size/
			2.0*cos(PI/(double) galvo_data_back*((double)i-galvo_data_forward));
		sample_rate=crs_frequency*galvo_subtrigger_length;

		DAQmxStopTask(taskHandleAO1);
		DAQmxClearTask(taskHandleAO1);
		DAQmxErrChk(DAQmxCreateTask("",&taskHandleAO1));
		DAQmxErrChk(DAQmxCreateAOVoltageChan(taskHandleAO1,"/Dev1/ao1","",-10.0,10.0,DAQmx_Val_Volts,NULL));
		DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandleAO1,"/Dev1/Ctr0InternalOutput",sample_rate,DAQmx_Val_Rising,DAQmx_Val_ContSamps,galvo_data_length));
		DAQmxErrChk(DAQmxWriteAnalogF64(taskHandleAO1,galvo_data_length,false,-1,DAQmx_Val_GroupByChannel,galvo_waveform,NULL,NULL));
		DAQmxErrChk(DAQmxStartTask(taskHandleAO1));

		DAQmxStopTask(taskHandleCO0);
		DAQmxClearTask(taskHandleCO0);
		DAQmxErrChk(DAQmxCreateTask("",&taskHandleCO0));
		DAQmxErrChk(DAQmxCreateCOPulseChanFreq(taskHandleCO0,"/Dev1/ctr0","",DAQmx_Val_Hz,DAQmx_Val_Low,0.0,sample_rate,0.5));
		DAQmxErrChk(DAQmxCfgImplicitTiming (taskHandleCO0, DAQmx_Val_FiniteSamps, galvo_subtrigger_length));
		DAQmxErrChk(DAQmxCfgDigEdgeStartTrig (taskHandleCO0, "/Dev1/PFI0", DAQmx_Val_Rising));
		DAQmxErrChk(DAQmxSetStartTrigRetriggerable(taskHandleCO0, true));
		DAQmxErrChk(DAQmxStartTask(taskHandleCO0));
	} catch (...) {
		if( DAQmxFailed(error)) 
			return error;
		if (AlazarFailed(error))  
			return (error - 1000);
	}
	return error;	
}




EXPORTFUN int __stdcall ThorVCMSetImpedA(int inputImpedA)
{
	int32 error = 0;
	try
	{
		switch (inputImpedA)
		{
		case 0:
			VCMbd.InputImpedChanA=IMPEDANCE_50_OHM;
			break;
		case 1:
			VCMbd.InputImpedChanA=IMPEDANCE_1M_OHM;
			break;
		}
		AlzErrChk(AlazarInputControl( h
		,CHANNEL_A,VCMbd.CouplingChanA
		,VCMbd.InputRangeChanA
		,VCMbd.InputImpedChanA));
	} catch (...) {
		if (AlazarFailed(error))  
			return (error - 1000);
	}
	return error;
}

EXPORTFUN int __stdcall ThorVCMSetImpedB(int inputImpedB)
{
	int32 error = 0;
	try
	{
		switch (inputImpedB)
		{
		case 0:
			VCMbd.InputImpedChanB=IMPEDANCE_50_OHM;
			break;
		case 1:
			VCMbd.InputImpedChanB=IMPEDANCE_1M_OHM;
			break;
		}
		AlzErrChk(AlazarInputControl( h
		,CHANNEL_B,VCMbd.CouplingChanB
		,VCMbd.InputRangeChanB
		,VCMbd.InputImpedChanB));

	} catch (...) {
		if (AlazarFailed(error))  
			return (error - 1000);
	}
	return error;
}





EXPORTFUN int __stdcall ThorVCMSetLumi(VCMImgPty *pImgPty)
{
	int contrast=pImgPty->contrast;
	int brightness=32768-64*(pImgPty->brightness);
	int colormap=pImgPty->colormap;
	double slope=tan((double) contrast*PI/200.0);
	int slopeint= (int) slope;
	int i;
	if (contrast>=0)
	{
		for (i=0;i<brightness;i++)
		{
			clmapR[i]=
			clmapG[i]=
			clmapB[i]= 0;
		}
		switch (colormap)
		{
		case 0:
			for (i=brightness;(i<65536) && (i<32640/slope+brightness);i++)
			{
				clmapR[i]=
				clmapG[i]=
				clmapB[i]=(BYTE) ( (double) ((i-brightness)/128.0)*slope);
			}
			break;
		case 1:
			i=brightness;
			clmapR[brightness]=
			clmapG[brightness]=
			clmapB[brightness]=0;
			for (i=brightness+1;i<65536 && (i< exp(255.0/slope)*128+brightness);i++)
			{
				clmapR[i]=
				clmapG[i]=
				clmapB[i]=(BYTE) (log((double) (i-brightness)/128.0)*slope);
			}
			break;
		case 2:
			for (i=brightness;i<65536&& i< (int) (sqrt(255.0/slope)*128.0+brightness);i++)
			{
				clmapR[i]=
				clmapG[i]=
				clmapB[i]=(BYTE) ((double) (i-brightness)* (double) (i-brightness)/16384.0*slope);
			}
			break;
		case 3:
			for (i=brightness; (i<65536) && i < (int) (brightness+8323200.0/slope/slope);i++)
			{
				clmapR[i]=
				clmapG[i]=
				clmapB[i]=(BYTE) (sqrt((double) (i-brightness)/128.0)*slope);
			}
			break;
		case 4:
			for (i=brightness;i<65536&&i<brightness+contrast;i++)
			{
				clmapR[i]=
				clmapG[i]=
				clmapB[i]=(BYTE) ((1-cos(((double) (i-brightness))*PI/((double) (contrast))))*127.5);
			}
			break;
		}
		for (;i<65536;i++)
		{
			clmapR[i]=
			clmapG[i]=
			clmapB[i]=255;
		}
	}
	else
	{
		for (i=65535;i>brightness;i--)
		{
			clmapR[i]=
			clmapG[i]=
			clmapB[i]= 0;
		}
		switch (colormap)
		{
		case 0:

			for (i=brightness;(i>=0) && (i>32640/slope+brightness);i--)
			{
				clmapR[i]=
				clmapG[i]=
				clmapB[i]=(BYTE) ((double) (i-brightness)/128*slope);
			}
			break;
		case 1:
			i=brightness;
			clmapR[brightness]=
			clmapG[brightness]=
			clmapB[brightness]=0;
			for (i=brightness-1;i>=0 && (i> brightness-exp(-255.0/slope)*128);i--)
			{
				clmapR[i]=
				clmapG[i]=
				clmapB[i]=(BYTE) (-1.0*log((double) (brightness-i)/128.0)*slope);
			}
			break;
		case 2:
			for (i=brightness;i>=0 && i> (int) (brightness-sqrt(-255.0/slope)*128.0);i--)
			{
				clmapR[i]=
				clmapG[i]=
				clmapB[i]=(BYTE) ((double) (brightness-i)* (double) (i-brightness)/16384.0*slope);
			}
			break;
		case 3:
			for (i=brightness; (i>=0) && i > (int) (brightness-8323200.0/slope/slope);i--)
			{
				clmapR[i]=
				clmapG[i]=
				clmapB[i]=(BYTE) (sqrt((double) (brightness-i)/128.0)*(-1.0)*slope);
			}
			break;
		case 4:
			for (i=brightness;i<65536&&i<brightness+contrast;i++)
			{
				clmapR[i]=
				clmapG[i]=
				clmapB[i]=(BYTE) ((1-cos(((double) (i-brightness))*PI/((double) (contrast))))*127.5);
			}
			break;
		}
		for (;i>=0;i--)
		{
			clmapR[i]=
			clmapG[i]=
			clmapB[i]=255;
		}
	}
	return 0;
}

EXPORTFUN int __stdcall ThorVCMSetBWLevel(int blacklv, int whitelv, bool positive_input, int ChanNum)
{
	int i, blvU16, wlvU16;
	if (blacklv>=256||blacklv<0||whitelv>=256||whitelv<0||blacklv>=whitelv)
		return -1;
	if (positive_input)
	{
		blvU16=blacklv*128+32768;
		wlvU16=whitelv*128+32768;
		switch (ChanNum)
		{
		case 0:
			memset(clmapB, 0, blvU16);
			for (i=blvU16;i<wlvU16; i++)
				clmapB[i]=(BYTE) ((i-blvU16)*255/(wlvU16-blvU16));
			for (i=wlvU16; i<65536; i++)
				clmapB[i]=255;
			break;
		case 1:
			memset(clmapG, 0, blvU16);
			for (i=blvU16;i<wlvU16; i++)
				clmapG[i]=(BYTE) ((i-blvU16)*255/(wlvU16-blvU16));
			for (i=wlvU16; i<65536; i++)
				clmapG[i]=255;
			break;
		case 2:
			memset(clmapR, 0, blvU16);
			for (i=blvU16;i<wlvU16; i++)
				clmapR[i]=(BYTE) ((i-blvU16)*255/(wlvU16-blvU16));
			for (i=wlvU16; i<65536; i++)
				clmapR[i]=255;
			break;
		}
	}
	else 
	{
		blvU16=32768-blacklv*128;
		wlvU16=32768-whitelv*128;
		switch (ChanNum)
		{
		case 0:
			memset(clmapB, 255, wlvU16);
			for (i=wlvU16;i<blvU16; i++)
				clmapB[i]=(BYTE)((blvU16-i)*255/(blvU16-wlvU16));
			for (i=blvU16;i<65535; i++)
				clmapB[i]=0;
			break;
		case 1:
			memset(clmapG, 255, wlvU16);
			for (i=wlvU16;i<blvU16; i++)
				clmapG[i]=(BYTE)((blvU16-i)*255/(blvU16-wlvU16));
			for (i=blvU16;i<65535; i++)
				clmapG[i]=0;
			break;
		case 2:
			memset(clmapR, 255, wlvU16);
			for (i=wlvU16;i<blvU16; i++)
				clmapR[i]=(BYTE)((blvU16-i)*255/(blvU16-wlvU16));
			for (i=blvU16;i<65535; i++)
				clmapR[i]=0;
		}
	}
	return 0;
}



EXPORTFUN int __stdcall ThorVCMSetRemap(int shift, unsigned xpixel)
{
	//data skipping begins
	U32 skip_Data;
	U32 skip_Address;
	U32 Reg_31;
	U32 Reg_03;
	unsigned cntr;
	remap_shift=shift;
	int32 error = 0;
	int eff_gsizoom=(int) (ImgPtyDll.width_size)+2*abs(gsi_offset);

	double x;

	//offset is considered here
	double marginalfactor=2.0*(double)(abs(gsi_offset))/(double) (ImgPtyDll.width_size);
	double marginalxsize=(double) xpixel*marginalfactor;
	double dxsize=(double) xpixel+marginalxsize;
	double dclocklength=pixel_clock_length;
	double dRecLength=xpixel*2+16;

	remap_index= (int*) realloc(remap_index, sizeof(int)*(xpixel*2+1));
	remap_index[0]=0;

	try 
	{
		if (gsi_offset<=0)
		{
			for (unsigned i=1;i<=xpixel;i++)
			{
				x=i; //
				remap_index[i]=(int) ((acos(1-2*x/dxsize)*dclocklength/(2*PI))-7-remap_shift);
				remap_index[xpixel*2+1-i]=(int) ((dclocklength-acos(1-2*x/dxsize)*dclocklength/(2*PI))-7-remap_shift);
			}
		}
		else if (gsi_offset>0)
		{
			for (unsigned i=1;i<=xpixel;i++)
			{
				x=i+marginalxsize; //
				remap_index[i]=(int) ((acos(1-2*x/dxsize)*dclocklength/(2*PI))-7-remap_shift);
				remap_index[xpixel*2+1-i]=(int) ((dclocklength-acos(1-2*x/dxsize)*dclocklength/(2*PI))-7-remap_shift);
			}
		}


		//Download Data skipping parameters
		skip_Data=0;
		for (cntr=0;cntr<=xpixel*2; cntr++)
		{
			skip_Address=cntr;
			skip_Data=remap_index[cntr];
			Reg_31=0;       //Make sure you are starting with a properly initalized 32 bit value
			Reg_31=(skip_Address<<16); 
			Reg_31=Reg_31+skip_Data; //Upper 16bits = address, lower 16bits=Data
			Reg_31=Reg_31|0x80000000;//Set MSB to be 1 to signify a write operation
			AlzErrChk(AlazarWriteRegister(h,31,Reg_31,0x32145876));
		}

		skip_Address=cntr;
		skip_Data=0; //This value can be any number less than FFFE
		Reg_31=0xFFFE;       //Make sure you are starting with a properly initalized 32 bit value
		Reg_31=(skip_Address<<16); 
		Reg_31=Reg_31+skip_Data; //Upper 16bits = address, lower 16bits=Data
		Reg_31=Reg_31|0x80000000;//Set MSB to be 1 to signify a write operation
		AlzErrChk(AlazarWriteRegister(h,31,Reg_31,0x32145876));

		skip_Address=cntr+1;
		skip_Data=0xFFFE; //0xFFFE to mark the end of data skip
		Reg_31=0;       //Make sure you are starting with a properly initalized 32 bit value
		Reg_31=(skip_Address<<16); 
		Reg_31=Reg_31+skip_Data; //Upper 16bits = address, lower 16bits=Data
		Reg_31=Reg_31|0x80000000;//Set MSB to be 1 to signify a write operation
		AlzErrChk(AlazarWriteRegister(h,31,Reg_31,0x32145876));

		//Enable data skipping mode Reg_03[3]=1
		AlzErrChk(AlazarReadRegister(h,3,&Reg_03,0x32145876));
		Reg_03=Reg_03|0x00000008;
		AlzErrChk(AlazarWriteRegister(h,3,Reg_03,0x32145876));
		//****************************************
		//data skipping ends

	} catch (...) {
		if (AlazarFailed(error))  
			return (error - 1000);
	}
	return error;
}




EXPORTFUN int __stdcall ThorVCMSetRemapUniDirect(int shift, unsigned xpixel)
{
	//Data skipping for unidirection image acquisition
	//Data is acquired only for the forward scan
	//data skipping begins
	//xpixel has to be multiples of 16

	U32 skip_Data;
	U32 skip_Address;
	U32 Reg_31;
	U32 Reg_03;
	unsigned cntr;
	remap_shift=shift;
	int32 error = 0;

	int eff_gsizoom=(int) (ImgPtyDll.width_size)+2*abs(gsi_offset);

	double x;
	double marginalfactor=2.0*(double)(abs(gsi_offset))/(double) (ImgPtyDll.width_size);
	double marginalxsize=(double) xpixel*marginalfactor;
	double dxsize=(double) xpixel+marginalxsize;

	double dclocklength=pixel_clock_length;
	double dRecLength=xpixel+16;

	remap_index= (int*) realloc(remap_index, sizeof(int)*(xpixel+1));
	remap_index[0]=0;

	try 
	{
		if (gsi_offset<=0)
		{
			for (unsigned i=1;i<=xpixel;i++)
			{
				x=i; //
				remap_index[i]=(int) ((acos(1-2*x/dxsize)*dclocklength/(2*PI))-7-remap_shift);
			}
		}
		else
		{
			for (unsigned i=1;i<=xpixel;i++)
			{
				x=i+marginalxsize; //
				remap_index[i]=(int) ((acos(1-2*x/dxsize)*dclocklength/(2*PI))-7-remap_shift);
			}
		}

		//Download Data skipping parameters
		skip_Data=0;
		for (cntr=0;cntr<=xpixel; cntr++)  //cntr<=xpixel*2 for bidirection
		{
			skip_Address=cntr;
			skip_Data=remap_index[cntr];
			Reg_31=0;       //Make sure you are starting with a properly initalized 32 bit value
			Reg_31=(skip_Address<<16); 
			Reg_31=Reg_31+skip_Data; //Upper 16bits = address, lower 16bits=Data
			Reg_31=Reg_31|0x80000000;//Set MSB to be 1 to signify a write operation
			AlzErrChk(AlazarWriteRegister(h,31,Reg_31,0x32145876));
		}

		skip_Address=cntr;
		skip_Data=0; //This value can be any number less than FFFE
		Reg_31=0xFFFE;       //Make sure you are starting with a properly initalized 32 bit value
		Reg_31=(skip_Address<<16); 
		Reg_31=Reg_31+skip_Data; //Upper 16bits = address, lower 16bits=Data
		Reg_31=Reg_31|0x80000000;//Set MSB to be 1 to signify a write operation
		AlzErrChk(AlazarWriteRegister(h,31,Reg_31,0x32145876));

		skip_Address=cntr+1;
		skip_Data=0xFFFE; //0xFFFE to mark the end of data skip
		Reg_31=0;       //Make sure you are starting with a properly initalized 32 bit value
		Reg_31=(skip_Address<<16); 
		Reg_31=Reg_31+skip_Data; //Upper 16bits = address, lower 16bits=Data
		Reg_31=Reg_31|0x80000000;//Set MSB to be 1 to signify a write operation
		AlzErrChk(AlazarWriteRegister(h,31,Reg_31,0x32145876));

		//Enable data skipping mode Reg_03[3]=1
		AlzErrChk(AlazarReadRegister(h,3,&Reg_03,0x32145876));
		Reg_03=Reg_03|0x00000008;
		AlzErrChk(AlazarWriteRegister(h,3,Reg_03,0x32145876));
		//****************************************
		//data skipping ends

	} catch (...) {
		if (AlazarFailed(error))  
			return (error - 1000);
	}
	return error;
}

EXPORTFUN int __stdcall ThorVCMSetRngA(int RngA)
{
	int32 error = 0; 
	try
	{
		switch (RngA)
		{
		case 0:
			VCMbd.InputRangeChanA = INPUT_RANGE_PM_20_MV;
			break;
		case 1:
			VCMbd.InputRangeChanA = INPUT_RANGE_PM_40_MV;
			break;
		case 2:
			VCMbd.InputRangeChanA = INPUT_RANGE_PM_50_MV;
			break;
		case 3:
			VCMbd.InputRangeChanA = INPUT_RANGE_PM_80_MV;
			break;
		case 4:
			VCMbd.InputRangeChanA = INPUT_RANGE_PM_100_MV;
			break;
		case 5:
			VCMbd.InputRangeChanA = INPUT_RANGE_PM_200_MV;
			break;
		case 6:
			VCMbd.InputRangeChanA = INPUT_RANGE_PM_400_MV;
			break;
		case 7:
			VCMbd.InputRangeChanA = INPUT_RANGE_PM_500_MV;
			break;
		case 8:
			VCMbd.InputRangeChanA = INPUT_RANGE_PM_800_MV;
			break;
		case 9:
			VCMbd.InputRangeChanA = INPUT_RANGE_PM_1_V;
			break;
		case 10:
			VCMbd.InputRangeChanA = INPUT_RANGE_PM_2_V;
			break;
		case 11:
			VCMbd.InputRangeChanA = INPUT_RANGE_PM_4_V;
			break;
		case 12:
			VCMbd.InputRangeChanA =	INPUT_RANGE_PM_5_V;
			break;
		case 13:
			VCMbd.InputRangeChanA = INPUT_RANGE_PM_8_V;
			break;
		case 14:
			VCMbd.InputRangeChanA = INPUT_RANGE_PM_10_V;
			break;
		default:
			VCMbd.InputImpedChanA = INPUT_RANGE_PM_400_MV;
			break;
		}
		AlzErrChk(AlazarInputControl( h
			,CHANNEL_A,VCMbd.CouplingChanA
			,VCMbd.InputRangeChanA
			,VCMbd.InputImpedChanA));
	} catch (...) {
		if (AlazarFailed(error))  
			return (error - 1000);
	}
	return error;
}

EXPORTFUN int __stdcall ThorVCMSetRngB(int RngB)
{
	int32 error = 0;
	try
	{
		switch (RngB)
		{
		case 0:
			VCMbd.InputRangeChanB = INPUT_RANGE_PM_20_MV;
			break;
		case 1:
			VCMbd.InputRangeChanB = INPUT_RANGE_PM_40_MV;
			break;
		case 2:
			VCMbd.InputRangeChanB = INPUT_RANGE_PM_50_MV;
			break;
		case 3:
			VCMbd.InputRangeChanB = INPUT_RANGE_PM_80_MV;
			break;
		case 4:
			VCMbd.InputRangeChanB = INPUT_RANGE_PM_100_MV;
			break;
		case 5:
			VCMbd.InputRangeChanB = INPUT_RANGE_PM_200_MV;
			break;
		case 6:
			VCMbd.InputRangeChanB = INPUT_RANGE_PM_400_MV;
			break;
		case 7:
			VCMbd.InputRangeChanB = INPUT_RANGE_PM_500_MV;
			break;
		case 8:
			VCMbd.InputRangeChanB = INPUT_RANGE_PM_800_MV;
			break;
		case 9:
			VCMbd.InputRangeChanB = INPUT_RANGE_PM_1_V;
			break;
		case 10:
			VCMbd.InputRangeChanB = INPUT_RANGE_PM_2_V;
			break;
		case 11:
			VCMbd.InputRangeChanB = INPUT_RANGE_PM_4_V;
			break;
		case 12:
			VCMbd.InputRangeChanB =	INPUT_RANGE_PM_5_V;
			break;
		case 13:
			VCMbd.InputRangeChanB = INPUT_RANGE_PM_8_V;
			break;
		case 14:
			VCMbd.InputRangeChanB = INPUT_RANGE_PM_10_V;
			break;
		default:
			VCMbd.InputImpedChanB = INPUT_RANGE_PM_400_MV;
			break;
		}
		AlzErrChk(AlazarInputControl( h
			,CHANNEL_B,VCMbd.CouplingChanB
			,VCMbd.InputRangeChanB
			,VCMbd.InputImpedChanB));
	} catch (...) {
		if (AlazarFailed(error))  
			return (error - 1000);
	}
	return error;
}

EXPORTFUN int __stdcall ThorVCMSetSplRate(int SplRateIndex)
{
	int32 error = 0;
	try
	{
		switch (SplRateIndex)
		{
		case 0:
			VCMbd.SampleRate=SAMPLE_RATE_1KSPS; //0X00000001UL
			break;
		case 1:
			VCMbd.SampleRate=SAMPLE_RATE_2KSPS; //0X00000002UL
			break;
		case 2:
			VCMbd.SampleRate=SAMPLE_RATE_5KSPS; //0X00000004UL
			break;
		case 3:
			VCMbd.SampleRate=SAMPLE_RATE_10KSPS; //0X00000008UL
			break;
		case 4:
			VCMbd.SampleRate=SAMPLE_RATE_20KSPS; //0X0000000AUL
			break;
		case 5:
			VCMbd.SampleRate=SAMPLE_RATE_50KSPS; //0X0000000CUL
			break;
		case 6:
			VCMbd.SampleRate=SAMPLE_RATE_100KSPS; //0X0000000EUL
			break;
		case 7:
			VCMbd.SampleRate=SAMPLE_RATE_200KSPS; //0X00000010UL
			break;
		case 8:
			VCMbd.SampleRate=SAMPLE_RATE_500KSPS; //0X00000012UL
			break;
		case 9:
			VCMbd.SampleRate=SAMPLE_RATE_1MSPS; //0X00000014UL
			break;
		case 10:
			VCMbd.SampleRate=SAMPLE_RATE_2MSPS; //0X00000018UL
			break;
		case 11:
			VCMbd.SampleRate=SAMPLE_RATE_5MSPS; //0X0000001AUL
			break;
		case 12:
			VCMbd.SampleRate=SAMPLE_RATE_10MSPS; //0X0000001CUL
			break;
		case 13:
			VCMbd.SampleRate=SAMPLE_RATE_20MSPS; //0X0000001EUL
			break;
		case 14:
			VCMbd.SampleRate=SAMPLE_RATE_25MSPS; //0X00000021UL
			break;
		case 15:
			VCMbd.SampleRate=SAMPLE_RATE_50MSPS; //0X00000022UL
			break;
		case 16:
			VCMbd.SampleRate=SAMPLE_RATE_100MSPS; //0X00000024UL
			break;
		case 17:
			VCMbd.SampleRate=SAMPLE_RATE_125MSPS; //0x00000025UL
			break;
		case 18:
			VCMbd.SampleRate=SAMPLE_RATE_160MSPS; //0x00000026UL
			break;
		case 19:
			VCMbd.SampleRate=SAMPLE_RATE_180MSPS; //0x00000027UL
			break;
		case 20:
			VCMbd.SampleRate=SAMPLE_RATE_200MSPS; //0X00000028UL
			break;
		case 21:
			VCMbd.SampleRate=SAMPLE_RATE_250MSPS; //0X0000002BUL
			break;
		case 22:
			VCMbd.SampleRate=SAMPLE_RATE_500MSPS; //0X00000030UL
			break;
		case 23:
			VCMbd.SampleRate=SAMPLE_RATE_1GSPS; //0x00000035UL
			break;
		case 24:
			VCMbd.SampleRate=SAMPLE_RATE_USER_DEF; //0x00000040UL
			break;
		}
		AlzErrChk(AlazarSetCaptureClock( h
		,VCMbd.ClockSource
		,VCMbd.SampleRate
		,VCMbd.ClockEdge
		,0));
	} catch (...) {
		if (AlazarFailed(error))  
			return (error - 1000);
	}
	return error;
}


EXPORTFUN int __stdcall ThorVCMSetTrigSlop(int trigSlope)
{
	int32 error = 0;
	try
	{
		switch (trigSlope)
		{
		case 0:
			VCMbd.TrigEngSlope1=TRIGGER_SLOPE_POSITIVE;
			break;
		case 1:
			VCMbd.TrigEngSlope1=TRIGGER_SLOPE_NEGATIVE;
			break;
		}

		AlzErrChk(AlazarSetTriggerOperation( h
			,VCMbd.TriEngOperation
			,VCMbd.TriggerEngine1
			,VCMbd.TrigEngSource1
			,VCMbd.TrigEngSlope1
			,VCMbd.TrigEngLevel1
			,VCMbd.TriggerEngine2
			,VCMbd.TrigEngSource2
			,VCMbd.TrigEngSlope2
			,VCMbd.TrigEngLevel2));
	} catch (...) {
		if (AlazarFailed(error))  
			return (error - 1000);
	}
	return error;
}

EXPORTFUN int __stdcall ThorVCMSetClock(bool intClock, double rateMHz, bool edgefalling)
{
	int32 error = 0;
	if (intClock)
	{
		VCMbd.ClockSource=INTERNAL_CLOCK;
		VCMbd.SampleRate=SAMPLE_RATE_125MSPS;
		pixel_clock_length=INTERNAL_CLOCK_PIXEL_LENGTH;
	}
	else
	{
		if (rateMHz>=80)
			VCMbd.ClockSource=FAST_EXTERNAL_CLOCK;
		else if (rateMHz>=10)
			VCMbd.ClockSource=MEDIUM_EXTERNAL_CLOCK;
		else
			VCMbd.ClockSource=SLOW_EXTERNAL_CLOCK;
		VCMbd.SampleRate=SAMPLE_RATE_USER_DEF;
		pixel_clock_length=(int) (rateMHz*INTERNAL_CLOCK_PIXEL_LENGTH/125.0);
	}
	if (edgefalling)
		VCMbd.ClockEdge=CLOCK_EDGE_FALLING;
	else
		VCMbd.ClockEdge=CLOCK_EDGE_RISING;

	try
	{
		AlzErrChk(::AlazarSetCaptureClock(h
			,VCMbd.ClockSource
			,VCMbd.SampleRate
			,VCMbd.ClockEdge
			,1));
	} catch (...) {
		if (AlazarFailed(error))  
			return (error - 1000);
	}
	return error;
}

EXPORTFUN int __stdcall ThorVCMSetTrigLevel(int trigLevel)
{
	int32 error = 0;
	try
	{
		if (trigLevel<0||trigLevel>255)
			return error;
		VCMbd.TrigEngLevel1= (U32) trigLevel;
		AlzErrChk(AlazarSetTriggerOperation( h
			,VCMbd.TriEngOperation
			,VCMbd.TriggerEngine1
			,VCMbd.TrigEngSource1
			,VCMbd.TrigEngSlope1
			,VCMbd.TrigEngLevel1
			,VCMbd.TriggerEngine2
			,VCMbd.TrigEngSource2
			,VCMbd.TrigEngSlope2
			,VCMbd.TrigEngLevel2));
	} catch (...) {
		if (AlazarFailed(error))  
			return (error - 1000);
	}
	return error;
}


EXPORTFUN int __stdcall ThorVCMSetZVolt(double zVolt)
{
	int32 error = 0;
	try 
	{
		DAQmxErrChk(DAQmxCreateTask("",&taskHandleAO0));
		DAQmxErrChk(DAQmxCreateAOVoltageChan(taskHandleAO0,"/Dev1/ao0","",-10.0,10.0,DAQmx_Val_Volts,NULL));
		DAQmxErrChk(DAQmxStartTask(taskHandleAO0));
		if ((zVolt>=-10.0)&&(zVolt<=10.0))
		{	
			DAQmxErrChk(DAQmxWriteAnalogScalarF64(taskHandleAO0,1,10.0, zVolt, NULL));
			DAQmxStopTask(taskHandleAO0);
			DAQmxClearTask(taskHandleAO0);
			error = 0;
		}
		else if (zVolt>10.0)
		{
			DAQmxErrChk(DAQmxWriteAnalogScalarF64(taskHandleAO0,1,10.0, 10.0, NULL));
			DAQmxStopTask(taskHandleAO0);
			DAQmxClearTask(taskHandleAO0);
			error = (101 - 3000);
		}
		else
		{
			DAQmxErrChk(DAQmxWriteAnalogScalarF64(taskHandleAO0,1,10.0, -10.0, NULL));
			DAQmxStopTask(taskHandleAO0);
			DAQmxClearTask(taskHandleAO0);
			error = (102 - 3000);
		}
	} catch (...) {
		if( DAQmxFailed(error)) 
			return error;
	}
	return error;
}

EXPORTFUN int __stdcall ThorVCMSetYVolt(double yVolt)
{
	int32 error = 0;
	try 
	{
		DAQmxStopTask(taskHandleAO1);
		DAQmxClearTask(taskHandleAO1);
		DAQmxErrChk(DAQmxCreateTask("",&taskHandleAO1));
		DAQmxErrChk(DAQmxCreateAOVoltageChan(taskHandleAO1,"/Dev1/ao1","",-10.0,10.0,DAQmx_Val_Volts,NULL));
		DAQmxErrChk(DAQmxStartTask(taskHandleAO1));
		if ((yVolt>=-10.0)&&(yVolt<=10.0))
		{	
			DAQmxErrChk(DAQmxWriteAnalogScalarF64(taskHandleAO1,1,10.0, yVolt, NULL));
			DAQmxStopTask(taskHandleAO1);
			DAQmxClearTask(taskHandleAO1);
			error = 0;
		}
		else if (yVolt>10.0)
		{
			DAQmxErrChk(DAQmxWriteAnalogScalarF64(taskHandleAO1,1,10.0, 10.0, NULL));
			DAQmxStopTask(taskHandleAO1);
			DAQmxClearTask(taskHandleAO1);
			error = (101 - 3000);
		}
		else
		{
			DAQmxErrChk(DAQmxWriteAnalogScalarF64(taskHandleAO1,1,10.0, -10.0, NULL));
			DAQmxStopTask(taskHandleAO1);
			DAQmxClearTask(taskHandleAO1);
			error = (102 - 3000);
		}
	} catch (...) {
		if( DAQmxFailed(error)) 
			return error;
	}
	return error;
}




EXPORTFUN int __stdcall TestFunction(int x, int y) 
{
	//ThorVCMAlignDataLoadFile();
	return x*y;
}

EXPORTFUN void __stdcall TestArray(int* a, int b)
{
	for (int i=0; i<b; i++)
	a[i]=i*i;
}


void InitDAQ()
{
	//----------------------------
	//Alzar Parameters
	VCMbd.ClockEdge=CLOCK_EDGE_RISING;
	VCMbd.ClockSource=INTERNAL_CLOCK;
	VCMbd.CouplingChanA=DC_COUPLING;
	VCMbd.CouplingChanB=DC_COUPLING;
	VCMbd.InputImpedChanA=IMPEDANCE_50_OHM;
	VCMbd.InputImpedChanB=IMPEDANCE_50_OHM;
	VCMbd.InputRangeChanA=INPUT_RANGE_PM_400_MV;
	VCMbd.InputRangeChanB=INPUT_RANGE_PM_400_MV;
	VCMbd.PreDepth=0;
	VCMbd.RecLength=1024;
	VCMbd.RecordCount=3;
	VCMbd.SampleRate=SAMPLE_RATE_125MSPS;
	VCMbd.TriEngOperation=TRIG_ENGINE_OP_J;
	VCMbd.TrigEngLevel1=160;
	VCMbd.TrigEngLevel2=160;
	VCMbd.TrigEngSlope1=TRIGGER_SLOPE_POSITIVE;
	VCMbd.TrigEngSlope2=TRIGGER_SLOPE_POSITIVE;
	VCMbd.TrigEngSource1=TRIG_EXTERNAL;
	VCMbd.TrigEngSource2=TRIG_DISABLE;
	VCMbd.TriggerEngine1=TRIG_ENGINE_J;
	VCMbd.TriggerEngine2=TRIG_ENGINE_K;


	remap_index= (int*) malloc(sizeof(int)*(128*2+1));
	for (i=0;i<NUM_ASYNBUF;i++)
		data[i]= (U16*) malloc(128*sizeof(U16));
	for (i=0;i<257;i++)
	{	// the 0th calibration data is always 0
		// 1 extra dummy calibration value for interpolation purpose
		cal_ZoomV[i]=0;
		cal_Align[i]=0;
	}
	ThorVCMAlignDataLoadFile();	//load the alignment data if exists;

	galvo_waveform= (float64 *) malloc(128*sizeof(float64));

	clmapR=(BYTE*) malloc(65536*sizeof(BYTE)); //Genearte a linear color map
	clmapG=(BYTE*) malloc(65536*sizeof(BYTE));	
	clmapB=(BYTE*) malloc(65536*sizeof(BYTE));
	ThorVCMSetBWLevel(0, 255, false, 0);
	ThorVCMSetBWLevel(0, 255, false, 1);
	ThorVCMSetBWLevel(0, 255, false, 2);



	//set alarzar digitizer, general settings
	h=AlazarGetBoardBySystemID(1,1);
	AlazarSetTriggerOperation( h
		,VCMbd.TriEngOperation
		,VCMbd.TriggerEngine1
		,VCMbd.TrigEngSource1
		,VCMbd.TrigEngSlope1
		,VCMbd.TrigEngLevel1
		,VCMbd.TriggerEngine2
		,VCMbd.TrigEngSource2
		,VCMbd.TrigEngSlope2
		,VCMbd.TrigEngLevel2);
	AlazarSetExternalTrigger(h, DC_COUPLING, ETR_5V);
	AlazarSetTriggerDelay (h, clockdelay);
	//-----------------------------------

}

void CloseDAQ()
{
	WaitForSingleObject(hThread, INFINITE);
	ThorVCMAlignDataSaveFile();
	for (int i=0; i<NUM_ASYNBUF; i++)
		free(data[i]);
	free(lineBuffer1);
	free(lineBuffer2);
	free(remap_index);free(galvo_waveform);
	DAQmxStopTask(taskHandleAO1);
	DAQmxStopTask(taskHandleCO0); 
	DAQmxStopTask(taskHandleDO1);
	DAQmxClearTask(taskHandleCO0);
//	DAQmxClearTask(taskHandleAO1);
//	DAQmxClearTask(taskHandleDO1);
	CloseHandle(hThread);
}

int ThorVCMSetRemapPixAve(int shift, unsigned xpixel)
{
	double x;
	double dxpixel=xpixel;
	double dclocklength=pixel_clock_length;
	//double dRecLength=xpixel*2;

	remap_shift=shift+14;
	remap_index= (int*) realloc(remap_index, sizeof(int)*xpixel*2);

	for (unsigned i=0;i<xpixel;i++)
	{
		x=i+1; //
		remap_index[i]=(int) ((acos(1-2*x/dxpixel)*dclocklength/(2*PI))-remap_shift);
		remap_index[xpixel*2-1-i]=(int) ((dclocklength-acos(1-2*x/dxpixel)*dclocklength/(2*PI))-remap_shift);
	}
	return 0;
}


void ThorVCMAlignDataLoadFile()
{
	int i;
	_getcwd(appPath, 256);
	strcpy_s(filePath, appPath);
	strcat_s(filePath, "\\AlignData.txt");
	FILE *AlignDataFile;
	if (fopen_s(&AlignDataFile, filePath, "r")!=0)
	{
		MessageBox(NULL, CString(filePath)+L" does not exist", NULL, MB_OK);
		for (i=0;i<256;i++)
			shift_array[i]=0;
	}
	else
	{
		for (i=0;i<256;i++)
		{
			//MessageBox(NULL, L"reading", NULL, MB_OK);
			if (fscanf_s(AlignDataFile, "%d", &shift_array[i])==EOF)
			{
				//char tempstr[256];
				MessageBox(NULL, L"not enough lines in data file", NULL, MB_OK);
				for (i;i<256;i++)
					shift_array[i]=0;
			}
			else if ((shift_array[i]<-128)||(shift_array[i]>128))
				shift_array[i]=0;
		}
		fclose(AlignDataFile);
	}
}

void ThorVCMAlignDataSaveFile()
{
	FILE *AlignDataFile;
	if (fopen_s(&AlignDataFile, filePath, "w")!=0)
	{
		MessageBox(NULL, CString(filePath)+L" Error Open Alignment Data File", NULL, MB_OK);
	}
	else
	{
		for (i=0;i<256;i++)
		{
			if (fprintf_s(AlignDataFile, "%d\n", shift_array[i])<=0)
			{
				MessageBox(NULL, L"Error writing Alignment Data", NULL, MB_OK);
				i=256;
			}
		}
		fclose(AlignDataFile);
	}
}


int ThorGetAutoDMAErrorCode(AUTODMA_STATUS err)
{
	switch (err) {
		case ADMA_Buffer1Invalid: return (801 - 1000); break;
		case ADMA_Buffer2Invalid: return (802 - 1000); break;
		case ADMA_InternalBuffer1Invalid: return (803 - 1000); break;
		case ADMA_InternalBuffer2Invalid: return (804 - 1000); break;
		case ADMA_InvalidChannel: return (805 - 1000); break;

		case ADMA_UseHeaderNotSet: return (806 - 1000); break;
		case ADMA_HeaderNotValid: return (807 - 1000); break;		
		case ADMA_InvalidRecsPerBuffer: return (808 - 1000); break;
		case ADMA_InvalidTransferOffset: return (89 - 1000); break;
		case ADMA_InvalidCFlags: return (810 - 1000); break;
	}
	return 0;
}
int ThorCloseNITasks()
{
	if (taskHandleDO0 !=0){
		DAQmxStopTask(taskHandleDO0);
		DAQmxClearTask(taskHandleDO0);
	}
	if (taskHandleAO1 !=0){
		DAQmxStopTask(taskHandleAO1);
		DAQmxClearTask(taskHandleAO1);
	}
	if (taskHandleCO0 !=0){
		DAQmxStopTask(taskHandleCO0);
		DAQmxClearTask(taskHandleCO0);
	}

	if (taskHandleDO1 !=0){
		DAQmxStopTask(taskHandleDO1);
		DAQmxClearTask(taskHandleDO1);
	}
	return 0;
}
/*
EXPORTFUN int __stdcall ThorVCMCalculatePeriod(int* linerawdata, int numPoint)
{
	return 0;
}
int ThorVCMCrossCorrelation(int* array1, int* array2, int arraysize)
{
	return 0;
}
*/