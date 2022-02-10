#pragma once

#include <string>
#include <map>
#include "..\..\..\..\Common\thread.h"
#include "..\..\..\..\Common\SLM.h"

using namespace std;


class SlmManager
{
public:
	~SlmManager();
	static SlmManager* getInstance();
	long FindSLMs(char* xml);
	long ReleaseSLMs();
	ISLM* GetSLM(long id);

private:
	SlmManager();

	static bool _instanceFlag;
	static std::unique_ptr<SlmManager> _single;
	static CritSect _critSect;
	static long _dllIdFoundCounter;
	static long _dllIdNotFoundCounter;
	void cleanup(void);

	bool _initialFound;
	map<long, SLMDll*> _slmMap;					//found slms' map, unique id but not unique dll memory, eg> [0 1 2 3] = [*p0 *p1 *p1 *p2]
	map<long, wstring> _slmNameMap;			//found slms' map name, unique id but not unique dll name, eg> [0 1 2 3] = [A B B C]
	map<long, SLMDll*> _slmNotFoundMap;	//not found slms' map, should be unique device per id
	map<long, wstring> _slmNotFoundNameMap;	//not found slms' map name, should be unique device per id
	map<long, SLMDll*>::const_iterator _it;

};


#pragma warning( push )