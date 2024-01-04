#include "CustomizationData.h"

FCustomizationData::FCustomizationData()
{
	this->AssociatedCharacter = 0;
	this->AssociatedRole = EPlayerRole::VE_None;
	this->Rarity = EItemRarity::Legendary;
	this->Prefix = ECustomizationModifier::None;
	this->UIData = FItemUIData{};
	this->Availability = FItemAvailability{};
	this->InclusionVersion = TEXT("Legacy");
	this->CustomizationId = TEXT("");
}
