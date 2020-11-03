#pragma once


class AcquireFactory
{
public:
	enum AcquireType
	{
		ACQ_SINGLE = 0,
		ACQ_MULTI_WAVELENGTH = 1,
		ACQ_Z_STACK = 2,
		ACQ_T_SERIES = 3,
		ACQ_T_STREAM = 4,
		//ACQ_TDI = 5,
		ACQ_BLEACHING = 6,
		ACQ_SEQUENCE = 7,
		ACQ_HYPERSPECTRAL = 8
	};

public:
	AcquireFactory();
	IAcquire *getAcquireInstance(AcquireType id,IAutoFocus *pAF,Observer *pOb,IExperiment *pExp,wstring path);

	static const char* const bufferChannelName[];

private:
	static Observer *pDefaultObserver;///<observer stored as the default observer
	//each time an acquire is created from the factory it will reassign the default observer
	//in the case where a NULL is passed as the observer. the acquire factory will attach the default observer.
};
