#include "stdafx.h"
#include "IROIDataStoreFactory.h"
#include "ROIDataStore.h"


IROIDataStore *IROIDataStoreFactory::CreateIROIDataStore(int choice)
{
  if (choice == 1)
    return ROIDataStore::GetInstance();
  return 0;
}


