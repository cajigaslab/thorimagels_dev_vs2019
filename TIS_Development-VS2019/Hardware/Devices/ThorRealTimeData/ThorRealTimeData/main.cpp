#include "stdafx.h"
#include <string.h>
#include <stdio.h>



#include "AcquireDataFactory.h"

//std::unique_ptr<HDF5ioDLL> hdf5io(new HDF5ioDLL(L"..\\..\\..\\..\\Tools\\HDF5\\HDF5IO\\x64\\Debug\\HDF5IO.dll"));
std::unique_ptr<HDF5ioDLL> hdf5io(new HDF5ioDLL(L".\\Modules_Native\\HDF5IO.dll"));


int _tmain(int argc, _TCHAR* argv[])
{

	uInt64 size = 0;
	wchar_t fname[_MAX_FNAME];
	std::wstring filename ( L"C:\\Temp\\Dev1_0001.h5" );
	wcscpy_s(fname,_MAX_FNAME,filename.c_str());
	hdf5io.get()->SetPathandFilename(fname);
	AcquireDataFactory::getInstance()->getAcquireInstance()->LoadXML();
	AcquireDataFactory::getInstance()->getAcquireInstance()->SetupChannels();
	AcquireDataFactory::getInstance()->getAcquireInstance()->SetupFileIO();
	AcquireDataFactory::getInstance()->getAcquireInstance()->SetSaving(TRUE);
	AcquireDataFactory::getInstance()->getAcquireInstance()->Start();
	
	DWORD workTime = GetTickCount();
	DWORD currentTime = 0;

	while(1==AcquireDataFactory::getInstance()->getAcquireInstance()->GetAcquiring()) //TRUE
	{
		/*currentTime = GetTickCount();
		if(6000 < (currentTime-workTime))
		{
			AcquireDataFactory::getInstance()->getAcquireInstance()->Stop();
			Sleep(5000);
			AcquireDataFactory::getInstance()->getAcquireInstance()->LoadXML();
			AcquireDataFactory::getInstance()->getAcquireInstance()->SetupChannels();
			AcquireDataFactory::getInstance()->getAcquireInstance()->SetupFileIO();
			AcquireDataFactory::getInstance()->getAcquireInstance()->SetSaving(TRUE);
			AcquireDataFactory::getInstance()->getAcquireInstance()->Start();
		}*/
	}
	AcquireDataFactory::getInstance()->getAcquireInstance()->Stop();
	AcquireDataFactory::getInstance()->~AcquireDataFactory();
	hdf5io.get()->DestroyFileIO();
	hdf5io.release();
	//printf("Acquiring samples continuously. Press Enter to interrupt\n");
	//printf("\nRead:\tAI\tDI\tCI\tTotal:\tAI\tDI\tCI\n");
	//getchar();
	

	
	return 0;
}
