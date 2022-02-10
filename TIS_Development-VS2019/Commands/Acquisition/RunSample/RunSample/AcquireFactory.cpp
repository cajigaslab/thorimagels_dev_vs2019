#include "stdafx.h"
#include "AcquireSingle.h"
#include "AcquireMultiWavelength.h"
#include "AcquireZStack.h"
#include "AcquireTSeries.h"
#include "AcquireTStream.h"
#include "AcquireBleaching.h"
#include "AcquireSequence.h"
#include "AcquireHyperspectral.h"
#include "AcquireFactory.h"
#include "Observer.h"

AcquireFactory::AcquireFactory()
{
}

const char* const AcquireFactory::bufferChannelName[] = {"ChanA","ChanB","ChanC","ChanD"};

Observer * AcquireFactory::pDefaultObserver = NULL;

IAcquire * AcquireFactory::getAcquireInstance(AcquireType type, Observer *pOb, IExperiment *pExp,wstring path)
{
	IAcquire *pAcquire;
	switch(type)
	{
	case ACQ_SINGLE:
		{
			pAcquire = new AcquireSingle(pExp,path);
		}
		break;
	case ACQ_MULTI_WAVELENGTH:
		{
			pAcquire = new AcquireMultiWavelength(pExp,path);
		}
		break;
	case ACQ_Z_STACK:
		{			
			pAcquire = new AcquireZStack(pExp,path);
		}
		break;
	case ACQ_T_SERIES:
		{			
			pAcquire = new AcquireTSeries(pExp,path);
		}
		break;
	case ACQ_T_STREAM:
		{
			pAcquire = new AcquireTStream(pExp,path);
		}
		break;
	case ACQ_BLEACHING:
		{
			pAcquire = new AcquireBleaching(pExp,path);
		}
		break;
	case ACQ_SEQUENCE:
		{
			pAcquire = new AcquireSequence(pExp,path);	
		}
		break;
	case ACQ_HYPERSPECTRAL:
		{
			pAcquire = new AcquireHyperspectral(pExp,path);	
		}
		break;
	default:
		{
			pAcquire = new AcquireSingle(pExp,path);
		}
	}

	if(NULL == pOb)
	{
		//in the case of a null observer. use the default observer that was stored previously
		pOb = pDefaultObserver;
	}

	if(pOb != NULL)
	{
		pDefaultObserver = pOb;
		pAcquire->CaptureImage.connect(pOb,&Observer::OnCaptureImage);
		pAcquire->SaveImage.connect(pOb,&Observer::OnSaveImage);
		pAcquire->CaptureSubImage.connect(pOb,&Observer::OnCaptureSubImage);
		pAcquire->SaveSubImage.connect(pOb,&Observer::OnSaveSubImage);
		pAcquire->StopCapture.connect(pOb,&Observer::OnStopCapture);
		pAcquire->SaveZImage.connect(pOb,&Observer::OnSaveZImage);
		pAcquire->SaveTImage.connect(pOb,&Observer::OnSaveTImage);
		pAcquire->CaptureComplete.connect(pOb,&Observer::OnCaptureComplete);
		pAcquire->PreCapture.connect(pOb,&Observer::OnPreCapture);
		pAcquire->SequenceStepCurrent.connect(pOb,&Observer::OnSequenceStepCurrent);
		pAcquire->StartProgressBar.connect(pOb,&Observer::OnProgressBarStart);
		pAcquire->InformMessage.connect(pOb, &Observer::OnInformMessage);
		pAcquire->NotifySavedFileIPC.connect(pOb, &Observer::OnNotifySavedFileIPC);
	}

	return pAcquire;
}