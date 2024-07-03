#pragma once
class ThorDAQRemoteFocusXML
{
private:


public:
	ThorDAQRemoteFocusXML();
	~ThorDAQRemoteFocusXML();

	long ReadPositionVoltages(long& numberOfPlanes, vector<double>* posVoltages);
};

