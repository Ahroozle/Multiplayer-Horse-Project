// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "MPHorsoInputHelperLibrary.h"

#include "StaticFuncLib.h"


void UMPHorsoInputHelperLibrary::GetBindingsForKey(FKey Key, TArray<FInputActionKeyMapping>& ActionBindings, TArray<FInputAxisKeyMapping>& AxisBindings)
{
	const UInputSettings* GrabbedSettings = GetDefault<UInputSettings>();
	if (nullptr != GrabbedSettings)
	{
		ActionBindings = GrabbedSettings->ActionMappings;
		AxisBindings = GrabbedSettings->AxisMappings;
	}
	else
		UStaticFuncLib::Print("UMPHorsoInputHelperLibrary::GetBindingsForKey: Couldn't get the default Input Settings!", true);
}

void UMPHorsoInputHelperLibrary::GetKeysForInputAction(FName ActionName, TArray<FInputActionKeyMapping>& Bindings, APlayerController* Focus)
{
	Bindings = Focus->PlayerInput->GetKeysForAction(ActionName);
}

void UMPHorsoInputHelperLibrary::GetAxisBindings(TArray<FInputAxisKeyMapping>& AxisBindings)
{
	const UInputSettings* GrabbedSettings = GetDefault<UInputSettings>();
	if (nullptr != GrabbedSettings)
		AxisBindings = GrabbedSettings->AxisMappings;
	else
		UStaticFuncLib::Print("UMPHorsoInputHelperLibrary::GetAxisBindings: Couldn't get the default Input Settings!", true);
}

void UMPHorsoInputHelperLibrary::GetActionBindings(TArray<FInputActionKeyMapping>& ActionBindings)
{
	const UInputSettings* GrabbedSettings = GetDefault<UInputSettings>();
	if (nullptr != GrabbedSettings)
		ActionBindings = GrabbedSettings->ActionMappings;
	else
		UStaticFuncLib::Print("UMPHorsoInputHelperLibrary::GetActionBindings: Couldn't get the default Input Settings!", true);
}

bool UMPHorsoInputHelperLibrary::RebindAxisKey(FInputAxisKeyMapping Original, FInputAxisKeyMapping New)
{
	UInputSettings* GrabbedSettings = GetMutableDefault<UInputSettings>();
	if (nullptr != GrabbedSettings)
	{
		TArray<FInputAxisKeyMapping>& AxisBindings = GrabbedSettings->AxisMappings;

		auto AxisPred = [&Original](FInputAxisKeyMapping& a) {return (a.AxisName == Original.AxisName) && (a.Key == Original.Key); };

		FInputAxisKeyMapping* Found = nullptr;
		if (nullptr != (Found = AxisBindings.FindByPredicate(AxisPred)))
		{
			Found->Key = New.Key;
			Found->Scale = New.Scale;

			GrabbedSettings->SaveKeyMappings();

			for (TObjectIterator<UPlayerInput> Iter; Iter; ++Iter)
				Iter->ForceRebuildingKeyMaps(true);

			return true;
		}
		else
			UStaticFuncLib::Print("UMPHorsoInputHelperLibrary::RebindAxisKey: Couldn't find Axis Mapping for \'" +
								  Original.AxisName.ToString() + "\' with Key \'" + Original.Key.ToString() + "\'!", true);
	}
	else
		UStaticFuncLib::Print("UMPHorsoInputHelperLibrary::RebindAxisKey: Couldn't get the default Input Settings!", true);

	return false;
}

bool UMPHorsoInputHelperLibrary::RebindActionKey(FInputActionKeyMapping Original, FInputActionKeyMapping New)
{
	UInputSettings* GrabbedSettings = GetMutableDefault<UInputSettings>();
	if (nullptr != GrabbedSettings)
	{
		TArray<FInputActionKeyMapping>& ActionBindings = GrabbedSettings->ActionMappings;

		auto ActionPred = [&Original](FInputActionKeyMapping& a) {return (a.ActionName == Original.ActionName) && (a.Key == Original.Key); };

		FInputActionKeyMapping* Found = nullptr;
		if (nullptr != (Found = ActionBindings.FindByPredicate(ActionPred)))
		{
			Found->Key = New.Key;
			Found->bShift = New.bShift;
			Found->bCtrl = New.bCtrl;
			Found->bAlt = New.bAlt;
			Found->bCmd = New.bCmd;

			GrabbedSettings->SaveKeyMappings();

			for (TObjectIterator<UPlayerInput> Iter; Iter; ++Iter)
				Iter->ForceRebuildingKeyMaps(true);

			return true;
		}
		else
			UStaticFuncLib::Print("UMPHorsoInputHelperLibrary::RebindActionKey: Couldn't find Action Mapping for \'" +
				Original.ActionName.ToString() + "\' with Key \'" + Original.Key.ToString() + "\'!", true);
	}
	else
		UStaticFuncLib::Print("UMPHorsoInputHelperLibrary::RebindActionKey: Couldn't get the default Input Settings!", true);

	return false;
}

FInputAxisKeyMapping UMPHorsoInputHelperLibrary::GetAxisFromEvent(const FKeyEvent& KeyEvent)
{
	FInputAxisKeyMapping Axis;

	// Only need to do the Key because the other default values are already done for me.
	Axis.Key = KeyEvent.GetKey();

	return Axis;
}

FInputActionKeyMapping UMPHorsoInputHelperLibrary::GetActionFromEvent(const FKeyEvent& KeyEvent)
{
	FInputActionKeyMapping Action;

	Action.Key = KeyEvent.GetKey();

	Action.bAlt = KeyEvent.IsAltDown();
	Action.bCtrl = KeyEvent.IsControlDown();
	Action.bShift = KeyEvent.IsShiftDown();
	Action.bCmd = KeyEvent.IsCommandDown();

	return Action;
}

void UMPHorsoInputHelperLibrary::GetAxisMappingInternals(const FInputAxisKeyMapping& AxisMapping, FName& AxisName, FKey& Key, float& Scale)
{
	AxisName = AxisMapping.AxisName;
	Key = AxisMapping.Key;
	Scale = AxisMapping.Scale;
}

void UMPHorsoInputHelperLibrary::GetActionMappingInternals(const FInputActionKeyMapping& ActionMapping, FName& ActionName, FKey& Key, bool& Shift, bool& Ctrl, bool& Alt, bool& Cmd)
{
	ActionName = ActionMapping.ActionName;
	Key = ActionMapping.Key;
	Shift = ActionMapping.bShift;
	Ctrl = ActionMapping.bCtrl;
	Alt = ActionMapping.bAlt;
	Cmd = ActionMapping.bCmd;
}

void UMPHorsoInputHelperLibrary::SetAxisMappingInternals(FInputAxisKeyMapping& AxisMapping, FName AxisName, FKey Key, float Scale)
{
	AxisMapping.AxisName = AxisName;
	AxisMapping.Key = Key;
	AxisMapping.Scale = Scale;
}

void UMPHorsoInputHelperLibrary::SetActionMappingInternals(FInputActionKeyMapping& ActionMapping, FName ActionName, FKey Key, bool Shift, bool Ctrl, bool Alt, bool Cmd)
{
	ActionMapping.ActionName = ActionName;
	ActionMapping.Key = Key;
	ActionMapping.bShift = Shift;
	ActionMapping.bCtrl = Ctrl;
	ActionMapping.bAlt = Alt;
	ActionMapping.bCmd = Cmd;
}

FString UMPHorsoInputHelperLibrary::Conv_KeyToString(FKey InKey) { return InKey.ToString(); }

bool UMPHorsoInputHelperLibrary::IsGamepadConnected()
{
	auto GenericApp = FSlateApplication::Get().GetPlatformApplication();

	return GenericApp->IsGamepadAttached();
}
