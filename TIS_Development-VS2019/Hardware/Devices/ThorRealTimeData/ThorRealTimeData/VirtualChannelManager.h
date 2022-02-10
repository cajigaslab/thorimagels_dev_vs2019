#pragma once

#include "PublicType.h"
#include "..\..\..\..\GUI\Controls\RealTimeLineChart\RealTimeLineChart\PublicEnum.cs"
#include <string>
#include <vector>
#include "..\..\..\..\Common\exprtk.hpp"

typedef exprtk::symbol_table<double> symbol_table_t;
typedef exprtk::expression<double> expression_t;
typedef exprtk::parser<double> parser_t;
typedef exprtk::parser_error::type error_t;
typedef exprtk::function_compositor<double> compositor_t;

//**************************************************************************//
//*** class to manage virtual channels which consist of complex     	 ***//
//*** mathmatical expressions. User may use inputs with data channels	 ***//
//*** or spectral channels but not virtual channels at this moment.		 ***//
//**************************************************************************//

class VirtualChannelProcessor
{
private:
	std::string _strExpression;

	//build-in functions:
	template <typename T>
	static T OTMSumPQ(T P, T Q, T timeRange, T* pRe, T* pIm);

	long TryParseExpression(symbol_table_t symbol_table, expression_t* expression);

public:
	std::vector<VirtualVariable> _variable;

	VirtualChannelProcessor(std::string expr);

	~VirtualChannelProcessor();

	//execute math expression on time domain channels, including both analog and digital
	long ExecuteTimeDomain(double* pAIn, unsigned char* pDIn, size_t length, long lineID, double* pOut);

	//execute math expression on freq domain channels
	long ExecuteFreqDomain(double* pSInRe, double* pSInIm, size_t length, long lineID, double* pOut);

	std::string GetExpression() { return _strExpression; }

};

class IVirtualChannelManager
{
public:
	virtual void UpdateProcessor() = 0;
	virtual void ClearAll() = 0;
	virtual long Execute(void* data) = 0;
};

class VirtualTimeChannelManager : IVirtualChannelManager
{
private:
	static bool _instanceFlag;
	static std::unique_ptr<VirtualTimeChannelManager> _single;

	std::vector<VirtualChannelProcessor> _processors;

	VirtualTimeChannelManager();

public:
	~VirtualTimeChannelManager();

	static VirtualTimeChannelManager* getInstance();

	//add one processor with one math expression, invoked after adding enabled channels
	virtual void UpdateProcessor();

	//clear all existing processors
	virtual void ClearAll();

	//execute all processors by providing buffers
	virtual long Execute(void* data);

};

class VirtualFreqChannelManager : IVirtualChannelManager
{
private:
	static bool _instanceFlag;
	static std::unique_ptr<VirtualFreqChannelManager> _single;

	std::vector<VirtualChannelProcessor> _processors;
	std::vector<std::string> _enabledFI;

	VirtualFreqChannelManager();

public:
	~VirtualFreqChannelManager();

	static VirtualFreqChannelManager* getInstance();

	//add one processor with one math expression, invoked after adding enabled channels
	virtual void UpdateProcessor();

	//clear all existing processors
	virtual void ClearAll();

	//execute all processors by providing buffers
	virtual long Execute(void* data);

};
