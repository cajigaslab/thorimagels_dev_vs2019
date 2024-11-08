// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

#pragma once

#include <stdio.h>
#include <map>
#include <list>
#include <memory>
#include <functional>
#include <thread>
#include <windows.h>
#include <cstring>
#include <string>

#include "Strsafe.h"

using namespace std;

// TODO: add headers that you want to pre-compile here

#include "CommonDefination.h"
#include "..\..\..\..\Common\Camera.h"
#include "..\..\..\..\Tools\Intel IPP\intel64\include\IPPlib.h"

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif

#include "..\..\..\..\Tools\ticpp\ticpp.h"
#include "..\..\..\..\Tools\ticpp\tinyxml.h"
#include "..\..\..\..\Tools\ticpp\ticpprc.h"

#define DllExport extern "C" long __declspec( dllexport )

//STDAFX_H
