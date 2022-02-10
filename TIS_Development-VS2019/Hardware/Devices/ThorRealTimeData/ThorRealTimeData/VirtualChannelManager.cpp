#include "stdafx.h"
#include "strsafe.h"
#include "VirtualChannelManager.h"

///	***************************************** <summary> VirtualChannelProcessor - process single virtual channel at a time </summary>	********************************************** ///

VirtualChannelProcessor::VirtualChannelProcessor(std::string expr) 
{
	_strExpression = expr;
	_variable = ChannelCenter::getInstance()->YieldVariables(_strExpression);
}

VirtualChannelProcessor::~VirtualChannelProcessor() 
{
}

long VirtualChannelProcessor::ExecuteTimeDomain(double* pAIn, unsigned char* pDIn, size_t length, long lineID, double* pOut)
{
	double* pDInValue = new double[_variable.size()];
	unsigned long tmpDVal = 0;
	auto t1 = Clock::now();

	//setup table, expression and parser
	symbol_table_t symbol_table;

	for (std::size_t i = 0; i < _variable.size(); i++)
	{
		switch ((SignalType)(_variable[i].sType))
		{
		case SignalType::ANALOG_IN:
			if(NULL == pAIn)
			{
				StringCbPrintfW(message,MSG_LENGTH,L"Error: analog channel buffer is not valid.\n");
				LogMessage(message, ERROR_EVENT);
				SAFE_DELETE_ARRAY(pDInValue);
				return FALSE;
			}
			_variable[i].pValue = *(pAIn + _variable[i].offset * length);
			break;
		case SignalType::DIGITAL_IN:
			if(NULL == pDIn)
			{
				StringCbPrintfW(message,MSG_LENGTH,L"Error: digital channel buffer is not valid.\n");
				LogMessage(message, ERROR_EVENT);
				SAFE_DELETE_ARRAY(pDInValue);
				return FALSE;
			}

			tmpDVal = *(pDIn + _variable[i].offset * length);
			pDInValue[i] = std::max(0.0, std::min(1.0, static_cast<double>(tmpDVal)));
			_variable[i].pValue = pDInValue[i];
			break;
		case SignalType::VIRTUAL:
			_variable[i].pValue = *(pOut + _variable[i].offset * length);
			break;
		}
		symbol_table.add_variable(_variable[i].name, _variable[i].pValue);
	}
	symbol_table.add_constants();

	expression_t expression;
	if(TRUE == TryParseExpression(symbol_table, &expression))
	{
		//iterate for expression output
		double* pTgt = (pOut + lineID * length);
		for (std::size_t i = 0; i < length; i++)
		{
			//update variable values for next operation
			for (std::size_t j = 0; j < _variable.size(); j++)
			{
				switch ((SignalType)(_variable[j].sType))
				{
				case SignalType::ANALOG_IN:
					_variable[j].pValue = *(pAIn + _variable[j].offset * length + i);
					break;
				case SignalType::DIGITAL_IN:
					tmpDVal = *(pDIn + _variable[j].offset * length + i);
					pDInValue[j] = std::max(0.0, std::min(1.0, static_cast<double>(tmpDVal)));
					_variable[j].pValue = pDInValue[j];
					break;
				case SignalType::VIRTUAL:
					_variable[j].pValue = *(pOut + _variable[j].offset * length + i);
					break;
				}
			}
			//operate expression
			*pTgt = expression.value();
			pTgt++;
		}
	}

	//done
	SAFE_DELETE_ARRAY(pDInValue);
	auto t2 = Clock::now();
	milliseconds ms = duration_cast<milliseconds>(t2-t1);
	StringCbPrintfW(message,MSG_LENGTH,L"VirtualChannel process %s: %u ms", std::wstring(_strExpression.begin(),_strExpression.end()).c_str(), static_cast<unsigned long>(ms.count()));
	LogMessage(message, VERBOSE_EVENT);
	return TRUE;
}

long VirtualChannelProcessor::ExecuteFreqDomain(double* pSInRe, double* pSInIm, size_t length, long lineID, double* pOut)
{
	double* pInValue = new double[_variable.size()];

	auto t1 = Clock::now();

	//setup table, expression and parser
	symbol_table_t symbol_table;

	for (std::size_t i = 0; i < _variable.size(); i++)
	{
		switch ((SignalType)(_variable[i].sType))
		{
		case SignalType::SPECTRAL:
			if((NULL == pSInRe) || (NULL == pSInIm))
			{
				StringCbPrintfW(message,MSG_LENGTH,L"Error: analog channel buffer is not valid.\n");
				LogMessage(message, ERROR_EVENT);
				SAFE_DELETE_ARRAY(pInValue);
				return FALSE;
			}
			//_variable[i].pValue = *(pSInRe + _variable[i].offset * length);
			break;
		}
		symbol_table.add_variable(_variable[i].name, _variable[i].pValue);
	}
	symbol_table.add_constants();

	double* pTgt = (pOut + lineID * length);
	expression_t expression;
	if(TRUE == TryParseExpression(symbol_table, &expression))
	{
		for (std::size_t i = 0; i < length; i++)
		{
			//update variable values for next operation
			for (std::size_t j = 0; j < _variable.size(); j++)
			{
				switch ((SignalType)(_variable[j].sType))
				{
				case SignalType::SPECTRAL:
				case SignalType::SPECTRAL_VIRTUAL:
					SpectralAnalyzer::Mag(pSInRe + _variable[j].offset * length + i, pSInIm + _variable[j].offset * length + i, &pInValue[j], 1);	//OTM: [nm]			
					_variable[j].pValue = pInValue[j];
					break;
				}
			}
			//operate expression
			*pTgt = expression.value();
			pTgt++;
		}
	}
	else
	{
		if(_strExpression.find(CROSSTALK) != std::string::npos)
		{
			//crosstalk must have two variables only
			if(2 != _variable.size())
				return FALSE;

			//operate expression
			for (size_t i = 0; i < length; i++)
			{
				double magA = sqrt(pow((*(pSInRe + _variable[0].offset * length + i)), 2) + pow((*(pSInIm + _variable[0].offset * length + i)), 2));
				double magB = sqrt(pow((*(pSInRe + _variable[1].offset * length + i)), 2) + pow((*(pSInIm + _variable[1].offset * length + i)), 2));
				*(pTgt + i) = (0 < magA) && (0 < magB) ? (double)(((*(pSInRe + _variable[0].offset * length + i)) * (*(pSInRe + _variable[1].offset * length + i)) + (*(pSInIm + _variable[0].offset * length + i)) * (*(pSInIm + _variable[1].offset * length + i))) / magA / magB) : MIN_VALUE_LOG;
			}
		}
		else if (((_strExpression.find(POWERSPECTRUMX) != std::string::npos) || (_strExpression.find(POWERSPECTRUMY) != std::string::npos)))
		{
			//crosstalk must have one variables only
			if(1 != _variable.size())
				return FALSE;

			//get DFT buffer power spectrum P(x) = (Re(x)^2+Im(x)^2)/timeRange, unit:[nm^2/Hz]
			double timeRange = (ChannelCenter::getInstance()->_isLive) ? 
				ChannelCenter::getInstance()->_specParam.liveSampleSec : 
				abs(ChannelCenter::getInstance()->_specParam.sampleMaxSec - ChannelCenter::getInstance()->_specParam.sampleMinSec);
			for (size_t i = 0; i < length; i++)
			{
				*(pTgt + i) = (pow(*(pSInRe + _variable[0].offset * length + i), 2)+pow(*(pSInIm + _variable[0].offset * length + i), 2))/timeRange;
			}
		}
		else if (((_strExpression.find(LORENTZIANFITX) != std::string::npos) || (_strExpression.find(LORENTZIANFITY) != std::string::npos)) && 
			(!ChannelCenter::getInstance()->_isLive && ChannelCenter::getInstance()->_otmParam.isCurveFit))
		{
			double timeRange = abs(ChannelCenter::getInstance()->_specParam.sampleMaxSec - ChannelCenter::getInstance()->_specParam.sampleMinSec);
			if(1 != _variable.size() || 0 == timeRange)
				return FALSE;

			double spq00 = OTMSumPQ((double)0, (double)0, timeRange, pSInRe + _variable[0].offset * length, pSInIm + _variable[0].offset * length);
			double spq01 = OTMSumPQ((double)0, (double)1, timeRange, pSInRe + _variable[0].offset * length, pSInIm + _variable[0].offset * length);
			double spq02 = OTMSumPQ((double)0, (double)2, timeRange, pSInRe + _variable[0].offset * length, pSInIm + _variable[0].offset * length);
			double spq11 = OTMSumPQ((double)1, (double)1, timeRange, pSInRe + _variable[0].offset * length, pSInIm + _variable[0].offset * length);
			double spq12 = OTMSumPQ((double)1, (double)2, timeRange, pSInRe + _variable[0].offset * length, pSInIm + _variable[0].offset * length);
			double spq22 = OTMSumPQ((double)2, (double)2, timeRange, pSInRe + _variable[0].offset * length, pSInIm + _variable[0].offset * length);
			double powerSpec0 = (pow(*(pSInRe + _variable[0].offset * length + ChannelCenter::getInstance()->_freqRangeIdx[0]), 2) + pow(*(pSInIm + _variable[0].offset * length + ChannelCenter::getInstance()->_freqRangeIdx[0]), 2)) / timeRange;
			double beta2FitPowerSpec1 = (pow(*(pSInRe + _variable[0].offset * length + ChannelCenter::getInstance()->_freqRangeIdx[2]), 2) + pow(*(pSInIm + _variable[0].offset * length + ChannelCenter::getInstance()->_freqRangeIdx[2]), 2)) / timeRange;
			double beta2FitPowerSpec2 = (pow(*(pSInRe + _variable[0].offset * length + ChannelCenter::getInstance()->_freqRangeIdx[3]), 2) + pow(*(pSInIm + _variable[0].offset * length + ChannelCenter::getInstance()->_freqRangeIdx[3]), 2)) / timeRange;
			double beta2Slope = exp(((log(beta2FitPowerSpec1) + log(beta2FitPowerSpec2))/2) + log(ChannelCenter::getInstance()->_otmParam.beta2FreqMin) + log(ChannelCenter::getInstance()->_otmParam.beta2FreqMax));

			double a = abs(spq01*spq22-spq11*spq12)/abs(spq02*spq22-pow(spq12,2));
			double b = abs(spq11*spq02-spq01*spq12)/abs(spq02*spq22-pow(spq12,2));
			double gammaTheory = ChannelCenter::getInstance()->_otmParam.gammaTheory*1e-6;		//[kg/s]
			double diffusionTheory = ChannelCenter::getInstance()->_otmParam.diffTheory*1e-12;	//[m^2/s]

			//LorentzianFit, partial buffer of spectral virtual channel,
			//based on freqRange length with freq. block
			long fblockId = 1;			//freq. block index
			double fblockSumFreq = 0.0;	//sum freq value: sum(f1,...,fN)
			double powerSpecAvgInOneBlock = 0;
			long offset = (ChannelCenter::getInstance()->_freqRangeIdx[1] + 1 - ChannelCenter::getInstance()->_freqRangeIdx[0]) % ChannelCenter::getInstance()->_otmParam.freqBlock; 
			std::vector<double> powerSpecForFit;	//power spectrum values respect to fit for chiXY [sum(freqBlock*(1-(PSavg/PSfit))^2)], even: PSfit, odd: PSavg
			for (long fIdx = ChannelCenter::getInstance()->_freqRangeIdx[0] + offset; fIdx <= ChannelCenter::getInstance()->_freqRangeIdx[1]; fIdx++, fblockId++)
			{
				fblockSumFreq += ChannelCenter::getInstance()->_freqHz.at(fIdx);
				powerSpecAvgInOneBlock += (pow(*(pSInRe + _variable[0].offset * length + fIdx), 2) + pow(*(pSInIm + _variable[0].offset * length + fIdx), 2)) / timeRange;
				if(fblockId >= ChannelCenter::getInstance()->_otmParam.freqBlock)
				{
					//average block value: sum(f1,...,fN)/N
					fblockSumFreq = (0 < ChannelCenter::getInstance()->_otmParam.freqBlock) ? (fblockSumFreq / ChannelCenter::getInstance()->_otmParam.freqBlock) : fblockSumFreq;
					powerSpecAvgInOneBlock = (0 < ChannelCenter::getInstance()->_otmParam.freqBlock) ? (powerSpecAvgInOneBlock / ChannelCenter::getInstance()->_otmParam.freqBlock) : fblockSumFreq;
					*(pTgt) = 1 / (a + (b * pow(fblockSumFreq, 2)));
					//prepare raw power spectrum for chiXY fit
					powerSpecForFit.push_back(*(pTgt));
					powerSpecForFit.push_back(powerSpecAvgInOneBlock);
					//reset freq. block index & sum value for next block
					pTgt++;
					fblockId = 0;
					fblockSumFreq = powerSpecAvgInOneBlock = 0.0;
				}
			}

			if(_strExpression.find(LORENTZIANFITX) != std::string::npos)
			{
				ChannelCenter::getInstance()->_otmFit.cornerXY[0] = sqrt(abs(a)/abs(b));	//[Hz]

				ChannelCenter::getInstance()->_otmFit.diffXY1[0] = ChannelCenter::getInstance()->_otmFit.diffXY2[0] = abs(2.0*pow(PI,2.0)/b/timeRange);		//[V^2s]

				//ChannelCenter::getInstance()->_otmFit.chiXY[0] = abs(ChannelCenter::getInstance()->_otmParam.freqBlock*(spq00 - ((pow(spq01,2.0)*spq22 + pow(spq11,2.0)*spq02 - 2*spq01*spq11*spq12)/(spq02*spq22-pow(spq12,2)))))/fRangeLength;
				ChannelCenter::getInstance()->_otmFit.chiXY[0] = 0;
				for (long fIdx = 0; fIdx < static_cast<long>(powerSpecForFit.size()/2); fIdx++)
				{
					//ChannelCenter::getInstance()->_otmFit.chiXY[0] += pow((-1/powerSpecForFit[fIdx] + 1/(*pTmp))*(*pTmp)*sqrt(ChannelCenter::getInstance()->_otmParam.freqBlock), 2);
					ChannelCenter::getInstance()->_otmFit.chiXY[0] += pow((1 - powerSpecForFit[2*fIdx+1]/powerSpecForFit[2*fIdx]), 2);
				}
				ChannelCenter::getInstance()->_otmFit.chiXY[0] = ChannelCenter::getInstance()->_otmFit.chiXY[0] * ChannelCenter::getInstance()->_otmParam.freqBlock / (static_cast<long>(powerSpecForFit.size()/2)-2);

				ChannelCenter::getInstance()->_otmFit.betaXY1[0] = sqrt((ChannelCenter::getInstance()->_otmFit.diffXY1[0])/diffusionTheory)*1e-9;	//[V/nm]
				ChannelCenter::getInstance()->_otmFit.betaXY2[0] = ((ChannelCenter::getInstance()->_otmParam.beta2FreqMin > ChannelCenter::getInstance()->_otmParam.beta2FreqMax) ||
					(ChannelCenter::getInstance()->_otmParam.beta2FreqMin < ChannelCenter::getInstance()->_freqHz.at(ChannelCenter::getInstance()->_freqRangeIdx[0])) || 
					(ChannelCenter::getInstance()->_otmParam.beta2FreqMax > ChannelCenter::getInstance()->_freqHz.at(ChannelCenter::getInstance()->_freqRangeIdx[1]))) ? 0 : sqrt(beta2Slope*2.0*pow(PI,2)/diffusionTheory)*1e-9;	//[V/nm]

				ChannelCenter::getInstance()->_otmFit.diffXY1[0] = ChannelCenter::getInstance()->_otmFit.diffXY1[0]/pow(ChannelCenter::getInstance()->_otmFit.betaXY1[0],2)*1e-6;	//[um^2s]
				ChannelCenter::getInstance()->_otmFit.diffXY2[0] = ChannelCenter::getInstance()->_otmFit.diffXY2[0]/pow(ChannelCenter::getInstance()->_otmFit.betaXY2[0],2)*1e-6;	//[um^2s]

				ChannelCenter::getInstance()->_otmFit.gammaXY[0] = ((BOLTZMAN_CONST*(ChannelCenter::getInstance()->_otmParam.temperature+CELSIUS_TO_KELVIN))/(pow(PI,2.0)*pow(ChannelCenter::getInstance()->_otmFit.cornerXY[0],2)*powerSpec0))/pow(ChannelCenter::getInstance()->_otmFit.betaXY1[0]*1e-9,2);	//[kg/s]

				ChannelCenter::getInstance()->_otmFit.kappaXY[0] = 2*PI*gammaTheory*ChannelCenter::getInstance()->_otmFit.cornerXY[0]*1e3;		//[pN/nm]

			}
			else if(_strExpression.find(LORENTZIANFITY) != std::string::npos)
			{
				ChannelCenter::getInstance()->_otmFit.cornerXY[1] = sqrt(abs(a)/abs(b));	//[Hz]

				ChannelCenter::getInstance()->_otmFit.diffXY1[1] = ChannelCenter::getInstance()->_otmFit.diffXY2[1] = abs(2.0*pow(PI,2.0)/b/timeRange);		//[V^2s]

				//ChannelCenter::getInstance()->_otmFit.chiXY[1] = abs(ChannelCenter::getInstance()->_otmParam.freqBlock*(spq00 - ((pow(spq01,2.0)*spq22 + pow(spq11,2.0)*spq02 - 2*spq01*spq11*spq12)/(spq02*spq22-pow(spq12,2)))))/fRangeLength;
				ChannelCenter::getInstance()->_otmFit.chiXY[1] = 0;
				for (long fIdx = 0; fIdx < static_cast<long>(powerSpecForFit.size()/2); fIdx++)
				{
					//ChannelCenter::getInstance()->_otmFit.chiXY[1] += pow((-1/powerSpecForFit[fIdx] + 1/(*pTmp))*(*pTmp)*sqrt(ChannelCenter::getInstance()->_otmParam.freqBlock), 2);
					ChannelCenter::getInstance()->_otmFit.chiXY[1] += pow((1 - powerSpecForFit[2*fIdx+1]/powerSpecForFit[2*fIdx]), 2);
				}
				ChannelCenter::getInstance()->_otmFit.chiXY[1] = ChannelCenter::getInstance()->_otmFit.chiXY[1] * ChannelCenter::getInstance()->_otmParam.freqBlock / (static_cast<long>(powerSpecForFit.size()/2)-2);

				ChannelCenter::getInstance()->_otmFit.betaXY1[1] = sqrt((ChannelCenter::getInstance()->_otmFit.diffXY1[1])/diffusionTheory)*1e-9;	//[V/nm]
				ChannelCenter::getInstance()->_otmFit.betaXY2[1] = ((ChannelCenter::getInstance()->_otmParam.beta2FreqMin > ChannelCenter::getInstance()->_otmParam.beta2FreqMax) ||
					(ChannelCenter::getInstance()->_otmParam.beta2FreqMin < ChannelCenter::getInstance()->_freqHz.at(ChannelCenter::getInstance()->_freqRangeIdx[0])) || 
					(ChannelCenter::getInstance()->_otmParam.beta2FreqMax > ChannelCenter::getInstance()->_freqHz.at(ChannelCenter::getInstance()->_freqRangeIdx[1]))) ? 0 : sqrt(beta2Slope*2.0*pow(PI,2)/diffusionTheory)*1e-9;	//[V/nm]

				ChannelCenter::getInstance()->_otmFit.diffXY1[1] = ChannelCenter::getInstance()->_otmFit.diffXY1[1]/pow(ChannelCenter::getInstance()->_otmFit.betaXY1[1],2)*1e-6;	//[um^2s]
				ChannelCenter::getInstance()->_otmFit.diffXY2[1] = ChannelCenter::getInstance()->_otmFit.diffXY2[1]/pow(ChannelCenter::getInstance()->_otmFit.betaXY2[1],2)*1e-6;	//[um^2s]

				ChannelCenter::getInstance()->_otmFit.gammaXY[1] = ((BOLTZMAN_CONST*(ChannelCenter::getInstance()->_otmParam.temperature+CELSIUS_TO_KELVIN))/(pow(PI,2.0)*pow(ChannelCenter::getInstance()->_otmFit.cornerXY[1],2)*powerSpec0))/pow(ChannelCenter::getInstance()->_otmFit.betaXY1[1]*1e-9,2);	//[kg/s]

				ChannelCenter::getInstance()->_otmFit.kappaXY[1] = 2*PI*gammaTheory*ChannelCenter::getInstance()->_otmFit.cornerXY[1]*1e3;		//[pN/nm]
			}
		}
	}
	//done
	SAFE_DELETE_ARRAY(pInValue);
	auto t2 = Clock::now();
	milliseconds ms = duration_cast<milliseconds>(t2-t1);
	StringCbPrintfW(message,MSG_LENGTH,L"SpectralVirtualChannel process %s: %u ms", std::wstring(_strExpression.begin(),_strExpression.end()).c_str(), static_cast<unsigned long>(ms.count()));
	LogMessage(message, VERBOSE_EVENT);
	return TRUE;
}

template <typename T>
T VirtualChannelProcessor::OTMSumPQ(T P, T Q, T timeRange, T* pRe, T* pIm)
{
	T retVal = 0;
	long fblockId = 1;				//freq. block index
	T fblockSumFreq = 0.0;			//sum freq value: sum(f1,...,fN)
	T fblockSumPowerSpec = 0.0;		//sum block value: sum(b1,...,bN)
	long offset = (ChannelCenter::getInstance()->_freqRangeIdx[1] + 1 - ChannelCenter::getInstance()->_freqRangeIdx[0]) % ChannelCenter::getInstance()->_otmParam.freqBlock; 
	for (long i = ChannelCenter::getInstance()->_freqRangeIdx[0] + offset; i <= ChannelCenter::getInstance()->_freqRangeIdx[1]; i++, fblockId++)
	{
		fblockSumFreq += ChannelCenter::getInstance()->_freqHz.at(i);
		fblockSumPowerSpec += (pow(*(pRe + i), 2) + pow(*(pIm + i), 2)) / timeRange;

		if(fblockId >= ChannelCenter::getInstance()->_otmParam.freqBlock)
		{
			//average block value & freq: sum(b1,...,bN)/N & sum(f1,...,fN)/N
			fblockSumFreq = (0 < ChannelCenter::getInstance()->_otmParam.freqBlock) ? (fblockSumFreq / ChannelCenter::getInstance()->_otmParam.freqBlock) : fblockSumFreq;
			fblockSumPowerSpec = (0 < ChannelCenter::getInstance()->_otmParam.freqBlock) ? (fblockSumPowerSpec / ChannelCenter::getInstance()->_otmParam.freqBlock) : fblockSumPowerSpec;
			retVal += pow(fblockSumFreq, 2*P) * pow(fblockSumPowerSpec, Q);
			//reset freq. block index & sum value for next block
			fblockId = 0;
			fblockSumPowerSpec = fblockSumFreq = 0.0;
		}
	}
	return retVal;
}

long VirtualChannelProcessor::TryParseExpression(symbol_table_t symbol_table, expression_t* expression)
{
	std::string errStr;
	expression->register_symbol_table(symbol_table);

	parser_t parser;
	if (!parser.compile(_strExpression, *expression))
	{
#if _DEBUG
		for (std::size_t i = 0; i < parser.error_count(); ++i)
		{
			error_t error = parser.get_error(i);
			errStr = exprtk::parser_error::to_str(error.mode);

			StringCbPrintfW(message,MSG_LENGTH,L"Parse Error: %02d Position: %02d Type: [%14s] Msg: %s Expression: %s",
				static_cast<unsigned int>(i),
				static_cast<unsigned int>(error.token.position),
				std::wstring(errStr.begin(),errStr.end()).c_str(),
				std::wstring(error.diagnostic.begin(),error.diagnostic.end()).c_str(),
				std::wstring(_strExpression.begin(),_strExpression.end()).c_str());

			LogMessage(message, VERBOSE_EVENT);
		}
#endif
		errStr = parser.error();
		StringCbPrintfW(message,MSG_LENGTH,L"Parse Error: %s, Expression: %s", 
			std::wstring(errStr.begin(),errStr.end()).c_str(), 
			std::wstring(_strExpression.begin(),_strExpression.end()).c_str());

		//log while not reserved expressions:
		if ((_strExpression.find(CROSSTALK) == std::string::npos) && 
			(_strExpression.find(LORENTZIANFITX) == std::string::npos) && (_strExpression.find(LORENTZIANFITY) == std::string::npos) &&
			(_strExpression.find(VAR) == std::string::npos)) 
			LogMessage(message, ERROR_EVENT);

		return FALSE;
	}
	return TRUE;
}

///	***************************************** <summary> VirtualTimeChannelManager - manage multiple time domain virtual channels </summary>	********************************************** ///

VirtualTimeChannelManager::VirtualTimeChannelManager() 
{
}

VirtualTimeChannelManager::~VirtualTimeChannelManager() 
{
	_instanceFlag = false; 
}

//instance flag must initialize after constructor
bool VirtualTimeChannelManager::_instanceFlag = false;
std::unique_ptr<VirtualTimeChannelManager> VirtualTimeChannelManager::_single = NULL;

VirtualTimeChannelManager* VirtualTimeChannelManager::getInstance()
{ 
	if(! _instanceFlag)
	{ 
		_single.reset(new VirtualTimeChannelManager()); 
		_instanceFlag = true;
	} 
	return _single.get(); 
}

void VirtualTimeChannelManager::UpdateProcessor()
{
	ClearAll();

	for (int i = 0; i < ChannelCenter::getInstance()->_virChannel.size(); i++)
	{
		if(0 == ChannelCenter::getInstance()->_virChannel[i].type.compare("/VT"))
		{
			VirtualChannelProcessor vcProcessor(ChannelCenter::getInstance()->_virChannel[i].lineId);

			_processors.push_back(vcProcessor);
		}
	}
}

void VirtualTimeChannelManager::ClearAll()
{
	_processors.clear();
}

long VirtualTimeChannelManager::Execute(void* data)
{
	CompoundData* cData = NULL;
	for (long i = 0; i < static_cast<long>(_processors.size()); i++)
	{
		cData = (CompoundData*)data;
		_processors.at(i).ExecuteTimeDomain(cData->GetStrucData()->aiDataPtr,cData->GetStrucData()->diDataPtr,cData->GetStrucData()->gcLength, i, cData->GetStrucData()->viDataPtr);
	}

	return TRUE;
}

///	***************************************** <summary> VirtualFreqChannelManager - manage multiple frequency domain virtual channels </summary>	********************************************** ///

VirtualFreqChannelManager::VirtualFreqChannelManager() 
{
}

VirtualFreqChannelManager::~VirtualFreqChannelManager() 
{
	_instanceFlag = false; 
}

//instance flag must initialize after constructor
bool VirtualFreqChannelManager::_instanceFlag = false;
std::unique_ptr<VirtualFreqChannelManager> VirtualFreqChannelManager::_single = NULL;

VirtualFreqChannelManager* VirtualFreqChannelManager::getInstance()
{ 
	if(! _instanceFlag)
	{ 
		_single.reset(new VirtualFreqChannelManager()); 
		_instanceFlag = true;
	} 
	return _single.get(); 
}

void VirtualFreqChannelManager::UpdateProcessor()
{
	ClearAll();

	for (int i = 0; i < ChannelCenter::getInstance()->_specVirChannel.size(); i++)
	{
		if(0 == ChannelCenter::getInstance()->_specVirChannel[i].type.compare("/VF"))
		{
			VirtualChannelProcessor vcProcessor(ChannelCenter::getInstance()->_specVirChannel[i].lineId);

			_processors.push_back(vcProcessor);
		}
	}

}

void VirtualFreqChannelManager::ClearAll()
{
	_processors.clear();
}

long VirtualFreqChannelManager::Execute(void* data)
{
	FreqCompoundData* fData = NULL;

	for (long i = 0; i < static_cast<long>(_processors.size()); i++)
	{
		fData = (FreqCompoundData*)data;
		_processors.at(i).ExecuteFreqDomain(fData->GetStrucData()->specDataRe,fData->GetStrucData()->specDataIm,fData->GetfreqSizeValue(), i, fData->GetStrucData()->vSpecData);
	}

	return TRUE;
}
