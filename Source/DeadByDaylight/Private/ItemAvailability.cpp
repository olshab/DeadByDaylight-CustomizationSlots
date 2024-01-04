#include "ItemAvailability.h"

FItemAvailability::FItemAvailability()
{
	this->itemAvailability = EItemAvailability::Available;
	this->DLCId = TEXT("0");
	this->AdditionalDlcIds = TArray<FString>();
	this->CloudInventoryId = -1;
	this->CommunityId = TEXT("0");
}
