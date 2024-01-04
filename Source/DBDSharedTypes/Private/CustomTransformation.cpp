#include "CustomTransformation.h"

FCustomTransformation::FCustomTransformation()
{
	this->UseCustomTransformation = false;
	this->CustomScale = FVector2D(1.0f, 1.0f);
	this->CustomTranslation = FVector2D{};
}
