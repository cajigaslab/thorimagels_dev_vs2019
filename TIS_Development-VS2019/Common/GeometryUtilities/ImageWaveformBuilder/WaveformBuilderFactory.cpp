#include "stdafx.h"
#include "WaveformBuilderFactory.h"
#include "EPhysWaveformBuilder.h"
#include "EPhysAOWaveformBuilder.h"

WaveformBuilderFactory::WaveformBuilderFactory()
{
}

IWaveformBuilder* WaveformBuilderFactory::GetBuilderInstance(int type)
{
	IWaveformBuilder* pBuilder = NULL;
	switch ((BuilderType)type)
	{
	case ALAZAR:
		break;
	case GR:
		break;
	case EPHYS_DO:
		pBuilder = (IWaveformBuilder*)EPhysWaveformBuilder::getInstance();
		break;
	case EPHYS_AO:
		pBuilder = (IWaveformBuilder*)EPhysAOWaveformBuilder::getInstance();
	default:
		break;
	}
	return pBuilder;
}
