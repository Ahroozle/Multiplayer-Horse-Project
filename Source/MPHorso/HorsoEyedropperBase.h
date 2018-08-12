// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "HorsoEyedropperBase.generated.h"

/**
 * 
 */
UCLASS()
class MPHORSO_API UHorsoEyedropperBase : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintImplementableEvent)
		EMouseCursor::Type RetrieveCursorType();

protected:

	virtual FCursorReply NativeOnCursorQuery(const FGeometry& InGeometry, const FPointerEvent& InCursorEvent) override;
	
	
};
