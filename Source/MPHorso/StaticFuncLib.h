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

	static void NetPrint(UObject* WorldContext, FString ToPrint, bool ToScreen = false);

	static bool ValidateObject(UObject* Obj, FString ToPrintIfInvalid, bool PrintToScreen = false);

	UFUNCTION(BlueprintCallable, meta = (WorldContext="WorldContext"))
		static class AOneShotAudio* PlaySound(UObject* WorldContext, const FTransform& GivenTransform, class USoundCue* sound, float pitch);

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContext"))
		static class UMPHorsoGameInstance* RetrieveGameInstance(UObject* WorldContext);

	UFUNCTION(BlueprintCallable)
		static void WordWrap(const FString& inStr, FString& outStr, int OptimalLineLength = 64);

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

	UFUNCTION(BlueprintPure, meta = (AdvancedDisplay = 1))
		static FLinearColor ColorFromHex(const FString& Hex, bool sRGB);

	UFUNCTION(BlueprintPure)
		static FString HexFromColor(FLinearColor Color, bool sRGB);

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

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Calculate Global Water Surface At Point"))
		static float CalcWaterSurface(FVector Point, float Scale, float Time);


	// Preeeeeetty colooorrrrrssss

	UFUNCTION(BlueprintPure)
		static void MakeComplementaryColorScheme(FLinearColor Root, TArray<FLinearColor>& OutColors, const TArray<FVector2D>& InSatVals);

	UFUNCTION(BlueprintPure)
		static void MakeAnalogousColorScheme(FLinearColor Root, int NumColors, float AngleHSV, TArray<FLinearColor>& OutColors,
			const TArray<FVector2D>& InSatVals);

	UFUNCTION(BlueprintPure)
		static void MakeTriadicColorScheme(FLinearColor Root, TArray<FLinearColor>& OutColors, const TArray<FVector2D>& InSatVals);

	UFUNCTION(BlueprintPure)
		static void MakeSplitComplementaryColorScheme(FLinearColor Root, float SplitAngleHSV, TArray<FLinearColor>& OutColors,
			const TArray<FVector2D>& InSatVals);

	UFUNCTION(BlueprintPure)
		static void MakeTetradicColorScheme(FLinearColor Root, float SplitAngleHSV, TArray<FLinearColor>& OutColors,
			const TArray<FVector2D>& InSatVals);

	UFUNCTION(BlueprintPure)
		static void MakeSquareColorScheme(FLinearColor Root, TArray<FLinearColor>& OutColors, const TArray<FVector2D>& InSatVals);

	UFUNCTION(BlueprintPure)
		static void MakeMonochromeColorScheme(FLinearColor Root, int NumColors, FVector2D IntervalRange, TArray<FLinearColor>& OutColors);

	UFUNCTION(BlueprintPure, meta = (AdvancedDisplay = 4))
		static void MakeRandomColorScheme(int NumColors, TArray<FLinearColor>& OutColors, const TArray<FVector2D>& InSatVals,
			float AnalogousHueAngle, float SplitCompHueAngle, float TetradicHueAngle, FVector2D MonochromeValRange);

	UFUNCTION(BlueprintPure)
		static void MakeRandomHorseColorScheme(TArray<FLinearColor>& OutColors, int BodyColors = 1, int HairColors = 1, int EyeColors = 1);

	UFUNCTION(BlueprintCallable)
		static UTexture2D* MakeTextureForColorScheme(const TArray<FLinearColor>& Colors);


	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"))
		static FVector2D ViewportToAbsolute(UObject* WorldContextObject,  FVector2D ViewportPosition);

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"))
		static FVector2D ViewportToLocal(UObject* WorldContextObject, const FGeometry& Geometry, FVector2D ViewportPosition);

	UFUNCTION(BlueprintCallable)
		static void ReleaseButton(class UButton* Button);

	UFUNCTION(BlueprintCallable)
		static void LeaveButton(class UButton* Button);

	// Get color from screen position (NOTE: Returns a Linear RGB color!)
	UFUNCTION(BlueprintPure)
		static FLinearColor GetColorAtScreenPos(FVector2D ScreenPos);

	// Get color from cursor position (NOTE: Returns a Linear RGB color!)
	UFUNCTION(BlueprintPure)
		static FLinearColor GetColorAtCursorPos();

	// Set a function to run on slate's pre-tick. (NOTE: Expected signature is Func(float)!)
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Function to Slate Pre-Tick By Name"))
		static void SetToPreTick(class UWidget* Widget, FString FuncName);

	// Clear all functions on the specified object from slate's pre-tick.
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Clear All From Slate Pre-Tick On Object"))
		static void ClearAllFromPreTick(class UWidget* Widget);

	UFUNCTION(BlueprintCallable)
		static void ForceCursorQuery();


	UFUNCTION(BlueprintCallable)
		static UTexture2D* MakeTextureFromRenderTarget(UTextureRenderTarget2D* Target, TArray<FLinearColor>& OutColors, bool InvertOpacity = true);


	UFUNCTION(BlueprintPure, meta = (DisplayName = "Vigenere-ish Cipher Encrypt"))
		FString ViggishCipherEncrypt(FString Base, FString Key, FString Alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Vigenere-ish Cipher Decrypt"))
		FString ViggishCipherDecrypt(FString Base, FString Key, FString Alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");


	// Note: Can't be run in-editor right now. Will fix later.
	UFUNCTION(BlueprintCallable)
		static void ConstructVectorFieldFromUVs(FIntVector Res, FBox Bounds, UTexture2D* LocalPositions, UTexture2D* VectorValues, FString FilePath);


	UFUNCTION(BlueprintCallable)
		static void HideActorFromPlayerController(APlayerController* Controller, AActor* Actor) { Controller->HiddenActors.AddUnique(Actor); }

	UFUNCTION(BlueprintCallable)
		static bool LineTraceActor(AActor* Actor, struct FHitResult& OutHit, const FVector& Start, const FVector& End, ECollisionChannel TraceChannel, bool bTraceComplex)
		{
			FCollisionQueryParams Params(FName(), bTraceComplex);
			return Actor->ActorLineTraceSingle(OutHit, Start, End, TraceChannel, Params);
		}

};
