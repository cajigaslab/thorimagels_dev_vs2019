#include "stdafx.h"
#include "MesoScanWaveform.h"

class GalvoController
{
public:
	static GalvoController * GetInstance(MesoScanWaveform *MesoScanWaveform)
	{
		if (_pInstance == NULL)
		{
			_pInstance = new GalvoController(MesoScanWaveform);
		}
		return _pInstance;
	}
	~GalvoController();

	long StartGG();
	long StopGG();
	long MoveGGToPos(double x1, double x2);
private:
	GalvoController(MesoScanWaveform *MesoScanWaveform);

	long GetCurrentPos(double &x1, double &x2, double &y);
	long GetTravelLength(double xStart, double xEnd, double yStart, double yEnd, double zStart, double zEnd);
	void CreateGalvoWaveform(double* data, long length, double start, double end);

private:
	static GalvoController* _pInstance;

	bool _hasGalvoController;

	bool _isActive;
	bool _isContinueOutput;
	MesoScanWaveform *_pMesoScanWaveform;

	TaskHandle _taskHandleAO0;  // galvoX1 galvoX2
	TaskHandle _taskHandleAO1;  // galvoY
	TaskHandle _taskHandleAI0;  // input for reading current position
};

