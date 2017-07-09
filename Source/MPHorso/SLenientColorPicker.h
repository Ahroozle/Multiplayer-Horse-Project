// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SColorPicker.h"

/**
 * 
 */
class MPHORSO_API SLenientColorPicker : public SColorPicker
{
public:
	FORCEINLINE void SetColor(FLinearColor NewCol) { SetNewTargetColorRGB(NewCol, true); }
	//FORCEINLINE void SetUseAlpha(bool NewUseAlpha) { bUseAlpha = NewUseAlpha; }
};
