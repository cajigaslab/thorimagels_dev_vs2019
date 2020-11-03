#pragma once

class IXYStage
{
public:

	virtual long Execute(double x, double y) = 0;//Synchrnous positioning of the stage
};
