// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/Widget.h"
#include "SLenientColorPicker.h"
#include "ColorWheel.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FColChangedNotify, class UColorWheel*, Instigator, FLinearColor, Col);

/**
 * 
 */
UCLASS()
class MPHORSO_API UColorWheel : public UWidget
{
	GENERATED_BODY()

public:

	UColorWheel(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Color Wheel")
		FLinearColor SelectedColor = FLinearColor::White;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Color Wheel")
		bool AlphaSelectable = false;

	UPROPERTY(BlueprintAssignable, Category = "Color Wheel")
		FColChangedNotify ColorChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Color Wheel")
		FColChangedNotify ColorCancelledDelegate;


	UFUNCTION(BlueprintCallable, Category = "Color Wheel")
		void SetColor(FLinearColor NewCol);


	void OnColorUpdated(FLinearColor NewCol);
	void OnColorCancelled(FLinearColor NewCol);

	//~ Begin UVisual Interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~ End UVisual Interface

protected:
	//~ Begin UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	//~ End UWidget Interface

#if WITH_EDITOR
	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface
#endif

protected:
	TSharedPtr<SLenientColorPicker> MyPicker;
};
