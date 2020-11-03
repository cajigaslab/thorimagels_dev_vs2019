#pragma once
#include "IROIDataStore.h"


class IROIDataStoreFactory
{
public:
	static IROIDataStore *CreateIROIDataStore(int Type);
};

