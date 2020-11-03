#include "NativeMatlabEngine.h"
#include "engine.h"
#include "mclmcrrt.h"
#include "mclcppclass.h"
#include <map>
#include <string>
using namespace std;

const int MAX_PATH_LENGH = 256; 
map<int, Engine*> * engineList = new map<int, Engine*>;

DllExport int OpenEngine(bool isSingle, bool isFigureViable)
{
	 Engine *ep;
	 if(isSingle)
	 {
		 ep = engOpenSingleUse(NULL, NULL, NULL);
	 }
	 else
	 {
		 ep = engOpen(NULL);
	 }

	 if (!ep) {
		return FALSE;
	}

	mclInitializeApplication(NULL, 0);
	if(!isFigureViable)
	{
		engSetVisible(ep, 0);
		engEvalString(ep, "set(0, 'DefaultFigureVisible', 'off')");
	}
	    
	//get max index increment
	int engineId =1;
	if(engineList->size()>0)
		engineId = engineList->rbegin()->first +1;

	 engineList->insert(pair<int, Engine*>(engineId, ep));

	 return engineId;
 }

DllExport int SetOutputBuffer(int engineId, char* outputBuffer, int bufferLen)
{
	map<int,Engine*>::iterator it = engineList->find(engineId);
	if(it != engineList->end())
	 {
		 Engine * ep = it->second;
		 if(ep!= NULL)
		 {
			 if(bufferLen < 1)
			 {
				 outputBuffer = NULL; 
				 bufferLen = 0;
			 }
			 engOutputBuffer(ep, outputBuffer, bufferLen);
			 return TRUE;
		 }
	 }
	 return FALSE;
}

DllExport int RunScript(int engineId, const char * scriptPath, const char * inputPath, const char * logPath)
{
	map<int,Engine*>::iterator it = engineList->find(engineId);
	if(it != engineList->end())
	 {
		 Engine * ep = it->second;
		 if(ep!= NULL)
		 {
			 string sPath(scriptPath, 0, MAX_PATH_LENGH);
	         size_t lastIndex = sPath.find_last_of("\\");
	         
	         string fileName = sPath.substr(lastIndex+1);
	         fileName = fileName.substr(0, fileName.find_last_of("."));
	         
	         string directory = sPath.substr(0, lastIndex);

	         string cmdAddPath("addpath ");
	         cmdAddPath =  cmdAddPath + "'" + directory + "'";;
	         
	         // add work path for engine
	         engEvalString(ep, cmdAddPath.c_str());
	         
	         string cmdStript = fileName+"('"+inputPath+"'"+",'"+ logPath +"')";
	         
	         // run macro
	         engEvalString(ep, cmdStript.c_str());
			 return TRUE;
		 }
	 }
	 return FALSE;
}

DllExport int ExcuteStatement(int engineId, const char * statement)
{
	map<int,Engine*>::iterator it = engineList->find(engineId);
	if(it != engineList->end())
	 {
		 Engine * ep = it->second;
		 if(ep!= NULL)
		 {
	         engEvalString(ep, statement);
			 return TRUE;
		 }
	 }
	 return FALSE;
}

 DllExport int CloseEngine(int engineId)
 {
	 map<int,Engine*>::iterator it = engineList->find(engineId);
	 if(it != engineList->end())
	 {
		 if(it->second != NULL)
		     engClose(it->second);
		 engineList->erase(it);
		 return TRUE;
	 }
	 return FALSE;
 }