// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "ColorWheel.h"


UColorWheel::UColorWheel(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

void UColorWheel::SetColor(FLinearColor NewCol)
{
	if (MyPicker.IsValid())
	{
		MyPicker->SetColor(NewCol);
	}
}

void UColorWheel::OnColorUpdated(FLinearColor NewCol)
{
	SelectedColor = NewCol;
	ColorChangedDelegate.Broadcast(this, NewCol);
}
void UColorWheel::OnColorCancelled(FLinearColor NewCol)
{
	// I'm assuming this is given the last color the user 'picked' before cancelling.
	// I guess I'll pass it along to the delegate just in case...
	ColorCancelledDelegate.Broadcast(this, NewCol);
}

void UColorWheel::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyPicker.Reset();
}

void UColorWheel::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	ColorAttr = OPTIONAL_BINDING(FLinearColor, SelectedColor);

	if (MyPicker.IsValid())
		MyPicker->SetColor(SelectedColor);
}

TSharedRef<SWidget> UColorWheel::RebuildWidget()
{
	FColorPickerArgs Args;

	ColorAttr = OPTIONAL_BINDING(FLinearColor, SelectedColor);

	///*bool*/								Args.bIsModal						/*=*/	;
	///*TSharedPtr<SWidget>*/					Args.ParentWidget					/*=*/	;
	/*bool*/								Args.bUseAlpha						= AlphaSelectable;
	/*bool*/								Args.bOnlyRefreshOnMouseUp			= false;
	/*bool*/								Args.bOnlyRefreshOnOk				= false;
	///*bool*/								Args.bExpandAdvancedSection			/*=*/	;
	///*bool*/								Args.bOpenAsMenu					/*=*/	;
	///*TAttribute<float>*/					Args.DisplayGamma					/*=*/	;
	///*TOptional<bool>*/						Args.sRGBOverride					/*=*/	;
	///*const TArray<FColor*>**/				Args.ColorArray						/*=*/	;
	///*const TArray<FLinearColor*>**/		Args.LinearColorArray				/*=*/	;
	///*const TArray<FColorChannels>**/		Args.ColorChannelsArray				/*=*/	;
	/*FOnLinearColorValueChanged*/			Args.OnColorCommitted				= FOnLinearColorValueChanged::CreateUObject(this, &UColorWheel::OnColorUpdated);
	///*FOnLinearColorValueChanged*/			Args.PreColorCommitted				/*=*/	;
	///*FOnWindowClosed*/						Args.OnColorPickerWindowClosed		/*=*/	;
	/*FOnColorPickerCancelled*/				Args.OnColorPickerCancelled			= FOnColorPickerCancelled::CreateUObject(this, &UColorWheel::OnColorCancelled);
	///*FSimpleDelegate*/						Args.OnInteractivePickBegin			/*=*/	;
	///*FSimpleDelegate*/						Args.OnInteractivePickEnd			/*=*/	;
	/*FLinearColor*/						Args.InitialColorOverride			= ColorAttr.Get();//SelectedColor;

	MyPicker = SNew(SLenientColorPicker)
					.TargetColorAttribute(Args.InitialColorOverride)
					.TargetFColors(Args.ColorArray ? *Args.ColorArray : TArray<FColor*>())
					.TargetLinearColors(Args.LinearColorArray ? *Args.LinearColorArray : TArray<FLinearColor*>())
					.TargetColorChannels(Args.ColorChannelsArray ? *Args.ColorChannelsArray : TArray<FColorChannels>())
					.UseAlpha(Args.bUseAlpha)
					.ExpandAdvancedSection(Args.bExpandAdvancedSection)
					.OnlyRefreshOnMouseUp(Args.bOnlyRefreshOnMouseUp && !Args.bIsModal)
					.OnlyRefreshOnOk(Args.bOnlyRefreshOnOk || Args.bIsModal)
					.OnColorCommitted(Args.OnColorCommitted)
					.PreColorCommitted(Args.PreColorCommitted)
					.OnColorPickerCancelled(Args.OnColorPickerCancelled)
					.OnInteractivePickBegin(Args.OnInteractivePickBegin)
					.OnInteractivePickEnd(Args.OnInteractivePickEnd)
					.DisplayGamma(Args.DisplayGamma);

	return MyPicker.ToSharedRef();
}

void UColorWheel::OnBindingChanged(const FName& Property)
{
	Super::OnBindingChanged(Property);

	if (Property == "SelectedColorDelegate")
	{
		ColorAttr = OPTIONAL_BINDING(FLinearColor, SelectedColor);
		MyPicker->Invalidate(EInvalidateWidget::LayoutAndVolatility);
	}
}

#if WITH_EDITOR
void UColorWheel::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (nullptr != PropertyChangedEvent.Property)
	{
		if (PropertyChangedEvent.Property->GetFName() == TEXT("SelectedColor"))
		{
			if (MyPicker.IsValid())
			{
				MyPicker->SetColor(SelectedColor);
			}
		}
		//else if (PropertyChangedEvent.Property->GetFName() == TEXT("AlphaSelectable"))
		//{
		//	if (MyPicker.IsValid())
		//	{
		//		MyPicker.Reset();
		//		RebuildWidget();
		//		//MyPicker->SetUseAlpha(AlphaSelectable);
		//	}
		//}
	}
}
#endif
