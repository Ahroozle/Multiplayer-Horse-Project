// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "MPHorsoInputHelperLibrary.generated.h"


/**
 * 
 */
UCLASS()
class MPHORSO_API UMPHorsoInputHelperLibrary : public UObject
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get All Axis and Action Bindings For Key"))
		static void GetBindingsForKey(FKey Key, TArray<FInputActionKeyMapping>& ActionBindings, TArray<FInputAxisKeyMapping>& AxisBindings);


	UFUNCTION(BlueprintPure)
		static void GetKeysForInputAction(FName ActionName, TArray<FInputActionKeyMapping>& Bindings, APlayerController* Focus);


	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get All InputAxis Bindings"))
		static void GetAxisBindings(TArray<FInputAxisKeyMapping>& AxisBindings);
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get All InputAction Bindings"))
		static void GetActionBindings(TArray<FInputActionKeyMapping>& ActionBindings);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Rebind InputAxis Key"))
		static bool RebindAxisKey(FInputAxisKeyMapping Original, FInputAxisKeyMapping New);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Rebind InputAction Key"))
		static bool RebindActionKey(FInputActionKeyMapping Original, FInputActionKeyMapping New);


	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get InputAxis From KeyEvent"))
		static FInputAxisKeyMapping GetAxisFromEvent(const FKeyEvent& KeyEvent);
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get InputAction From KeyEvent"))
		static FInputActionKeyMapping GetActionFromEvent(const FKeyEvent& KeyEvent);


	// >tfw epic games exposes key mappings to blueprints but they cause asstons of warnings if you use them

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Safely Break InputAxis"))
		static void GetAxisMappingInternals(const FInputAxisKeyMapping& AxisMapping, FName& AxisName, FKey& Key, float& Scale);
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Safely Break InputAction"))
		static void GetActionMappingInternals(const FInputActionKeyMapping& ActionMapping, FName& ActionName, FKey& Key, bool& Shift, bool& Ctrl, bool& Alt, bool& Cmd);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Safely Make InputAxis"))
		static void SetAxisMappingInternals(FInputAxisKeyMapping& AxisMapping, FName AxisName, FKey Key, float Scale);
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Safely Make InputAction"))
		static void SetActionMappingInternals(FInputActionKeyMapping& ActionMapping, FName ActionName, FKey Key, bool Shift, bool Ctrl, bool Alt, bool Cmd);


	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (Key)", CompactNodeTitle = "->", BlueprintAutocast))
		static FString Conv_KeyToString(FKey InKey);


	UFUNCTION(BlueprintPure)
		static bool IsGamepadConnected();
};
