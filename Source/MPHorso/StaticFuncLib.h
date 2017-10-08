// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "StaticFuncLib.generated.h"

/**
 * 
 */
UCLASS()
class MPHORSO_API UStaticFuncLib : public UObject
{
	GENERATED_BODY()
	
public:
	
	static void Print(FString ToPrint, bool ToScreen = false);

	static bool ValidateObject(UObject* Obj, FString ToPrintIfInvalid, bool PrintToScreen = false);

	UFUNCTION(BlueprintCallable, meta = (WorldContext="WorldContext"))
		static class AOneShotAudio* PlaySound(UObject* WorldContext, const FTransform& GivenTransform, class USoundCue* sound, float pitch);

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContext"))
		static class UMPHorsoGameInstance* RetrieveGameInstance(UObject* WorldContext);

	//static class USkinAnimation* GetSkinAnim(UObject* WorldContext, TSubclassOf<class USkinAnimation> InstClass);
	//static class UOffsetAnimation* GetOffsAnim(UObject* WorldContext, TSubclassOf<class UOffsetAnimation> InstClass);

	UFUNCTION(BlueprintCallable)
		static void WordWrap(const FString& inStr, FString& outStr, int OptimalLineLength = 64);

	UFUNCTION(BlueprintCallable)
		static class AActor* SpawnActorProxy(class AActor* WorldContext, TSubclassOf<AActor> ToSpawn, const FTransform& SpawnLoc);

	UFUNCTION(BlueprintCallable)
		static class UParticleSystemComponent* SpawnEmitterAtLocationProxy(class AActor* WorldContext, UParticleSystem* Template, FVector SpawnLoc, FRotator SpawnRot, bool AutoDestroy = true);

	UFUNCTION(BlueprintCallable)
		static void GetAllActorsOfClassProxy(class AActor* WorldContext, TSubclassOf<class AActor> Class, TArray<class AActor*>& OutFound);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Construct Quadratic Bezier Points"))
		static void MakeQBezierPoints(const FVector& P0, const FVector& P1, const FVector& P2, int NumPoints, TArray<FVector>& OutPoints);

	UFUNCTION(BlueprintCallable, meta=(DisplayName="Construct Cubic Bezier Points"))
		static void MakeCBezierPoints(const FVector& P0, const FVector& P1, const FVector& P2, const FVector& P3, int NumPoints, TArray<FVector>& OutPoints);

	UFUNCTION(BlueprintPure)
		static bool GetViewportNormalizedMousePosition(class APlayerController* playerController, FVector2D& OutVNPos);

	UFUNCTION(BlueprintCallable)
		static class UActorComponent* CreateComponentFromClass(class AActor* OuterActor, TSubclassOf<class UActorComponent> Class);

	UFUNCTION(BlueprintCallable)
		static class AActor* CloneActor(class AActor* Original);

	UFUNCTION(BlueprintCallable)
		static void ExportToBitmap(const TArray<FColor>& pixels, FString fileName, int width, int height);

	UFUNCTION(BlueprintCallable)
		static class UUserWidget* CreateWidgetProxy(class APlayerController* Owner, TSubclassOf<class UUserWidget> WidgClass);

	UFUNCTION(BlueprintPure)
		static FLinearColor ColorFromHex(const FString& Hex);

	UFUNCTION(BlueprintPure)
		static FString GetGameDirectory();

	// C++ - only
	static TSharedPtr<FGenericWindow> GetMainNativeWindow();
	static TSharedPtr<FGenericWindow> GetNativeSubWindow(class UWidget* SubWindowContext);

	UFUNCTION(BlueprintCallable)
		static bool OpenFileDialog(
									const FString& DialogTitle,
									const FString& DefaultPath,
									const FString& DefaultFile,
									const FString& FileTypes,
									bool MultipleFiles,
									TArray<FString>& OutFilenames,
									class UWidget* SubWindowContext
								  );

	UFUNCTION(BlueprintCallable)
		static bool SaveFileDialog(
									const FString& DialogTitle,
									const FString& DefaultPath,
									const FString& DefaultFile,
									const FString& FileTypes,
									bool MultipleFiles,
									TArray<FString>& OutFilenames,
									class UWidget* SubWindowContext
								  );

	UFUNCTION(BlueprintCallable)
		static bool GetStringFromFile(FString& OutString, const FString& FilePath);

	UFUNCTION(BlueprintCallable)
		static bool SaveStringToFile(const FString& StringToSave, const FString& FilePath);

	UFUNCTION(BlueprintPure)
		static FString AddSpacesToCamelcase(const FString& InString);

	/*
		Maps the given Value from within the range of LinearRange to within the range of ExponentialRange,
		Along an exponential curve defined by Base^X.
	*/
	UFUNCTION(BlueprintPure)
		static float MapLinearRangetoExponentialRange(float Value, FVector2D LinearRange, FVector2D ExponentialRange, float Base);

	UFUNCTION(BlueprintPure)
		static float NearestPowerOfTwo(float Num);

	UFUNCTION(BlueprintPure)
		static float NextPowerOfTwo(float Num);

	UFUNCTION(BlueprintPure)
		static float PreviousPowerOfTwo(float Num);

	UFUNCTION(BlueprintPure)
		static FString ToRomanNumerals(int Number);

	UFUNCTION(BlueprintPure)
		static bool LostFocus(APlayerController* Player);
};
