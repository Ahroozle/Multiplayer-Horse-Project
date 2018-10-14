// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "StaticFuncLib.h"

#include "Sound/SoundCue.h"
#include "OneShotAudio.h"

#include "MPHorsoGameInstance.h"

#include "SkeletalSpriteAnimation.h"

#include "Kismet/GameplayStatics.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Slate/SceneViewport.h"
#include "Components/Button.h"

#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"

//#include "RHI.h"


#if WITH_EDITOR
#define USTATICFUNCLIB_ALLOW_PRINT_TO_SCREEN true
#else
#define USTATICFUNCLIB_ALLOW_PRINT_TO_SCREEN false
#endif


void UStaticFuncLib::Print(FString ToPrint, bool ToScreen)
{
	UE_LOG(MyLog, Error, TEXT("%s"), *ToPrint);

	if (ToScreen && USTATICFUNCLIB_ALLOW_PRINT_TO_SCREEN)
		GEngine->AddOnScreenDebugMessage(-1, 4, FColor::Red, ToPrint);
}

bool UStaticFuncLib::ValidateObject(UObject* Obj, FString ToPrintIfInvalid, bool PrintToScreen)
{
	if (nullptr == Obj)
	{
		Print(ToPrintIfInvalid, PrintToScreen);
		return false;
	}
	return true;
}

AOneShotAudio* UStaticFuncLib::PlaySound(UObject* WorldContext, const FTransform& GivenTransform, class USoundCue* sound, float pitch)
{
	AOneShotAudio* spawned = WorldContext->GetWorld()->SpawnActor<AOneShotAudio>(AOneShotAudio::StaticClass(), GivenTransform);
	//UStaticFuncLib::Print(FString(WorldContext->GetWorld()->GetNetMode() == NM_Client ? "Client: " : "Server: ") + "Sound", true);
	if (spawned)
		spawned->Init(sound, pitch);
	return spawned;
}

UMPHorsoGameInstance* UStaticFuncLib::RetrieveGameInstance(UObject* WorldContext)
{
	if (ValidateObject(WorldContext, "UStaticFuncLib::RetrieveGameInstance: World Context Object was null!", true))
	{
		UGameInstance* gameInst = WorldContext->GetWorld()->GetGameInstance();
		if (ValidateObject(gameInst, "UStaticFuncLib::RetrieveGameInstance: Couldn't get the game instance!", true))
		{
			UMPHorsoGameInstance* castedInst = Cast<UMPHorsoGameInstance>(gameInst);
			if (ValidateObject(castedInst, "UStaticFuncLib::RetrieveGameInstance: Couldn't cast game instance to proper type!", true))
				return castedInst;
		}
	}
	return nullptr;
}

//USkinAnimation* UStaticFuncLib::GetSkinAnim(UObject* WorldContext, TSubclassOf<USkinAnimation> InstClass)
//{
//	UMPHorsoGameInstance* gameInst = RetrieveGameInstance(WorldContext);
//	if (ValidateObject(gameInst, "UStaticFuncLib::GetSkinAnim: Couldn't retrieve the game instance!", true))
//		return gameInst->GetSkinAnim(InstClass);
//	return nullptr;
//}
//
//UOffsetAnimation* UStaticFuncLib::GetOffsAnim(UObject* WorldContext, TSubclassOf<UOffsetAnimation> InstClass)
//{
//	UMPHorsoGameInstance* gameInst = RetrieveGameInstance(WorldContext);
//	if (ValidateObject(gameInst, "UStaticFuncLib::GetSkinAnim: Couldn't retrieve the game instance!", true))
//		return gameInst->GetOffsAnim(InstClass);
//	return nullptr;
//}

void UStaticFuncLib::WordWrap(const FString& inStr, FString& outStr, int OptimalLineLength)
{
	outStr = inStr;

	if (outStr.Len() > OptimalLineLength * 3 || (outStr.Len() > OptimalLineLength * 2 && outStr.Contains(". ")))
	{
		int currchar = OptimalLineLength;
		while (currchar < outStr.Len())
		{
			if (outStr[currchar] == ' ')
			{
				outStr[currchar] = '\n';
				currchar += OptimalLineLength;
			}
			else
				++currchar;
		}
	}
}

//AActor* UStaticFuncLib::SpawnActorProxy(AActor* WorldContext, TSubclassOf<AActor> ToSpawn, const FTransform& SpawnLoc)
//{
//	return WorldContext->GetWorld()->SpawnActor(ToSpawn, &SpawnLoc);
//}
//
//UParticleSystemComponent* UStaticFuncLib::SpawnEmitterAtLocationProxy(AActor* WorldContext, UParticleSystem* Template, FVector SpawnLoc, FRotator SpawnRot, bool AutoDestroy)
//{
//	return UGameplayStatics::SpawnEmitterAtLocation(WorldContext, Template, SpawnLoc, SpawnRot, AutoDestroy);
//}
//
//void UStaticFuncLib::GetAllActorsOfClassProxy(AActor* WorldContext, TSubclassOf<AActor> Class, TArray<AActor*>& OutFound)
//{
//	UGameplayStatics::GetAllActorsOfClass(WorldContext, Class, OutFound);
//}

void UStaticFuncLib::MakeQBezierPoints(const FVector& P0, const FVector& P1, const FVector& P2, int NumPoints, TArray<FVector>& OutPoints)
{
	//// Naive Impl, ~1100 ops per 100 points
	//
	//const float dT = 1.0f / (NumPoints - 1); // change in time

	//float T = 0; // currTime

	//// nothin to see here officer just a trainwreck :^)
	//float iT; // Inverse time variable
	//for (int currStep = 0; currStep < NumPoints; ++currStep)
	//{
	//	iT = 1 - T;

	//	// B(t) = ((1-t)^2 * P0) + (2t * (1-t) * P1) + (t^2 * P2)
	//	OutPoints.Add(((iT*iT)*P0) + (2*T*iT*P1) + (T*T*P2));

	//	T += dT;
	//}
	

	// Forward Difference impl, ~300 ops per 100 points

	const float dT = 1.0f / (NumPoints - 1); // change in time

	// Coefficients for the polynomial, gotten by plugging the equation ( ((1-t)^2 * P0) + (2t * (1-t) * P1) + (t^2 * P2) )
	// into Wolfram Alpha's Taylor Series Calculator.
	const FVector a = P0;
	const FVector b = 2 * (P1 - P0);
	const FVector c = P0 - (2 * P1) + P2;

	// NOTE: for some reason you apply the coeffs in backwards order
	// i.e. instead of the formula being Ax^2 + Bx + C
	// it is Cx^2 + Bx + A
	//
	// This was explained literally nowhere so I just felt like I should write it down here
	// so I don't flop around in agony for 2 days straight again
	FVector S = a;					// P(t) @ t=0
	FVector U = c*dT*dT + b*dT;		// First Diff of P(t) (equ. 2aht + ah^2 + bh) @ t=0
	FVector V = 2 * c*dT*dT;		// Second Diff of P(t) (equ 2ah^2) @t=0

	OutPoints.Add(P0);
	for (int currStep = 1; currStep < NumPoints; ++currStep)
	{
		S += U;
		U += V;

		OutPoints.Add(S);
	}

}

void UStaticFuncLib::MakeCBezierPoints(const FVector& P0, const FVector& P1, const FVector& P2, const FVector& P3, int NumPoints, TArray<FVector>& OutPoints)
{
	//// Naive Impl, ~1800 ops per 100 points
	//
	//const float dT = 1.0f / (NumPoints - 1); // change in time

	//float T = 0; // currTime

	//// another trainwreck because it's 1 am and I'm lazy
	//float iT; // Inverse time variable
	//for (int currStep = 0; currStep < NumPoints; ++currStep)
	//{
	//	iT = 1 - T;

	//	// B(t) = ((1-t)^3 * P0) + (3t * (1-t)^2 * P1) + (3t^2 * (1-t) * P2) + (t^3 * P3)
	//	OutPoints.Add(((iT*iT*iT)*P0) + (3*T*(iT*iT)*P1) + (3*(T*T)*iT*P2) + ((T*T*T)*P3));

	//	T += dT;
	//}


	// Unreal's forward-diff method (length calcs culled), ~400 ops per 100 points
	// you can find the original as FVector::EvaluateBezier in UnrealMath.cpp

	// var q is the change in t between successive evaluations.
	const float dT = 1.f / (NumPoints - 1); // q is dependent on the number of GAPS = POINTS-1

	// coefficients of the cubic polynomial that we're FDing -
	const FVector a = P0;
	const FVector b = 3 * (P1 - P0);
	const FVector c = 3 * (P2 - 2 * P1 + P0);
	const FVector d = P3 - 3 * P2 + 3 * P1 - P0;

	// initial values of the poly and the 3 diffs -
	FVector S = a;								// the poly value
	FVector U = b*dT + c*dT*dT + d*dT*dT*dT;	// 1st order diff (quadratic)
	FVector V = 2*c*dT*dT + 6*d*dT*dT*dT;		// 2nd order diff (linear)
	FVector W = 6*d*dT*dT*dT;					// 3rd order diff (constant)

	OutPoints.Add(P0);	// first point on the curve is always P0.
	for (int32 i = 1; i < NumPoints; ++i)
	{
		// calculate the next value and update the deltas
		S += U;			// update poly value
		U += V;			// update 1st order diff value
		V += W;			// update 2st order diff value
						// 3rd order diff is constant => no update needed.

		OutPoints.Add(S);
	}

}

bool UStaticFuncLib::GetViewportNormalizedMousePosition(APlayerController* playerController, FVector2D& OutVNPos)
{
	if (nullptr != playerController)
	{
		const ULocalPlayer* localPlayer = playerController->GetLocalPlayer();

		if (nullptr != localPlayer)
		{
			FViewport* viewport = localPlayer->ViewportClient->Viewport;

			if (nullptr != viewport)
			{
				localPlayer->ViewportClient->GetMousePosition(OutVNPos);

				OutVNPos.X /= viewport->GetSizeXY().X;
				OutVNPos.Y /= viewport->GetSizeXY().Y;

				return true;
			}
		}
	}
	return false;

}

UActorComponent* UStaticFuncLib::CreateComponentFromClass(AActor* OuterActor, TSubclassOf<UActorComponent> Class)
{
	UActorComponent* NewComp = NewObject<UActorComponent>(OuterActor, Class, Class.GetDefaultObject()->GetFName());

	if (nullptr != NewComp)
		NewComp->RegisterComponent();

	return NewComp;
}

AActor* UStaticFuncLib::CloneActor(AActor* Original)
{
	FActorSpawnParameters spawnparams;

	spawnparams.Template = Original;

	AActor* Spawned = Original->GetWorld()->SpawnActor<AActor>(Original->GetClass(), spawnparams);

	Spawned->SetReplicates(false);
	Spawned->SetReplicates(true);

	return Spawned;
}

void UStaticFuncLib::ExportToBitmap(const TArray<FColor>& pixels, FString fileName, int width, int height)
{
	FFileHelper::CreateBitmap(*(FPaths::GameDir() + fileName + ".bmp"), width, height, pixels.GetData());
}

//class UUserWidget* UStaticFuncLib::CreateWidgetProxy(class APlayerController* Owner, TSubclassOf<class UUserWidget> WidgClass)
//{
//	return UWidgetBlueprintLibrary::Create(Owner, WidgClass, Owner);
//}

FLinearColor UStaticFuncLib::ColorFromHex(const FString& Hex, bool sRGB)
{
	FColor StartCol = FColor::FromHex(Hex);

	FLinearColor EndCol;
	if (sRGB)
	{
		float red = StartCol.R / 255.0f;
		float green = StartCol.G / 255.0f;
		float blue = StartCol.B / 255.0f;
		float alpha = StartCol.A / 255.0f;

		red = red <= 0.04045f ? red / 12.92f : FMath::Pow((red + 0.055f) / 1.055f, 2.4f);
		green = green <= 0.04045f ? green / 12.92f : FMath::Pow((green + 0.055f) / 1.055f, 2.4f);
		blue = blue <= 0.04045f ? blue / 12.92f : FMath::Pow((blue + 0.055f) / 1.055f, 2.4f);

		EndCol = { red, green, blue, alpha };
	}
	else
		EndCol = FLinearColor(StartCol.R / 255.0f, StartCol.G / 255.0f, StartCol.B / 255.0f, StartCol.A / 255.0f);

	return EndCol;
}

FString UStaticFuncLib::HexFromColor(FLinearColor Color, bool sRGB)
{
	return Color.ToFColor(sRGB).ToHex();
}

FString UStaticFuncLib::GetGameDirectory() { return FPaths::GameDir(); }

TSharedPtr<FGenericWindow> UStaticFuncLib::GetMainNativeWindow()
{
	GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle();

	if (nullptr != GEngine->GameViewport)
	{
		TSharedPtr<SWindow> RetrievedWind = GEngine->GameViewport->GetWindow();

		if (RetrievedWind.IsValid())
			return RetrievedWind->GetNativeWindow();
	}

	return nullptr;
}

TSharedPtr<FGenericWindow> UStaticFuncLib::GetNativeSubWindow(UWidget* SubWindowContext)
{
	if (nullptr != SubWindowContext)
	{
		TSharedPtr<SWindow> RetrievedWind = FSlateApplication::Get().FindWidgetWindow(SubWindowContext->TakeWidget());

		if (RetrievedWind.IsValid())
			return RetrievedWind->GetNativeWindow();
	}

	return nullptr;
}

bool UStaticFuncLib::OpenFileDialog(
									const FString& DialogTitle,
									const FString& DefaultPath,
									const FString& DefaultFile,
									const FString& FileTypes,
									bool MultipleFiles,
									TArray<FString>& OutFilenames,
									UWidget* SubWindowContext
								   )
{
	TSharedPtr<FGenericWindow> NativeWind = GetMainNativeWindow();

	if (!NativeWind.IsValid())
		NativeWind = GetNativeSubWindow(SubWindowContext);

	if (NativeWind.IsValid())
	{
		const void* OSWind = NativeWind->GetOSWindowHandle();

		return FDesktopPlatformModule::Get()->OpenFileDialog(OSWind, DialogTitle, DefaultPath, DefaultFile, FileTypes, (MultipleFiles ? 0x0 : 0x1), OutFilenames);
	}

	return false;
}

bool UStaticFuncLib::SaveFileDialog(
									const FString& DialogTitle,
									const FString& DefaultPath,
									const FString& DefaultFile,
									const FString& FileTypes,
									bool MultipleFiles,
									TArray<FString>& OutFilenames,
									UWidget* SubWindowContext
								   )
{
	TSharedPtr<FGenericWindow> NativeWind = GetMainNativeWindow();

	if (!NativeWind.IsValid())
		NativeWind = GetNativeSubWindow(SubWindowContext);

	if (NativeWind.IsValid())
	{
		const void* OSWind = NativeWind->GetOSWindowHandle();

		return FDesktopPlatformModule::Get()->SaveFileDialog(OSWind, DialogTitle, DefaultPath, DefaultFile, FileTypes, (MultipleFiles ? 0x0 : 0x1), OutFilenames);
	}

	return false;
}

bool UStaticFuncLib::GetStringFromFile(FString& OutString, const FString& FilePath)
{
	return FFileHelper::LoadFileToString(OutString, *FilePath);
}

bool UStaticFuncLib::SaveStringToFile(const FString& StringToSave, const FString& FilePath)
{
	return FFileHelper::SaveStringToFile(StringToSave, *FilePath);
}

FString UStaticFuncLib::AddSpacesToCamelcase(const FString& InString)
{
	if (InString.IsEmpty())
		return "";

	const TCHAR spacechar = FString(" ")[0];

	FString NewStr;
	for (auto iter = InString.CreateConstIterator(); iter; ++iter)
	{
		if (iter - 1)
		{
			if ((*iter != spacechar && *(iter - 1) != spacechar))
			{
				if ((TChar<TCHAR>::IsDigit(*iter) && TChar<TCHAR>::IsAlpha(*(iter - 1))) || (TChar<TCHAR>::IsUpper(*iter) && TChar<TCHAR>::IsLower(*(iter - 1))))
					NewStr += " ";
			}
		}
		NewStr += *iter;
	}
	return NewStr;
}

float UStaticFuncLib::MapLinearRangetoExponentialRange(float Value, FVector2D LinearRange, FVector2D ExponentialRange, float Base)
{
	return ((ExponentialRange.Y - ExponentialRange.X) - FMath::LogX(Base, (LinearRange.Y - LinearRange.X) / (Value - LinearRange.X))) + ExponentialRange.X;
}

float UStaticFuncLib::NearestPowerOfTwo(float Num)
{
	return FMath::Pow(2, FMath::RoundToFloat(FMath::Log2(Num)));
}

float UStaticFuncLib::NextPowerOfTwo(float Num)
{
	return FMath::Pow(2, FMath::CeilToFloat(FMath::Log2(Num)));
}

float UStaticFuncLib::PreviousPowerOfTwo(float Num)
{
	return FMath::Pow(2, FMath::FloorToFloat(FMath::Log2(Num)));
}

FString UStaticFuncLib::ToRomanNumerals(int Number)
{
	TArray<FString> RomanArr =
	{
		"M","CM","D","CD","C","XC","L","XL","X","IX","V","IV","I"
	};

	TArray<int> ValueArr =
	{
		1000,900,500,400,100,90,50,40,10,9,5,4,1
	};

	FString ResultString;

	for (int currNumeral = 0; currNumeral < RomanArr.Num(); ++currNumeral)
	{
		while (Number%ValueArr[currNumeral] < Number)
		{
			ResultString += RomanArr[currNumeral];
			Number -= ValueArr[currNumeral];
		}
	}

	return ResultString;
}

bool UStaticFuncLib::LostFocus(APlayerController* Player)
{
	ULocalPlayer* LocPlayer = Player->GetLocalPlayer();

	return !LocPlayer->ViewportClient->Viewport || !LocPlayer->ViewportClient->Viewport->IsForegroundWindow();
}

float UStaticFuncLib::CalcWaterSurface(FVector Point, float Scale, float Time)
{
	float z = 0.0;
	z += (sin(Point.X * 1.0 / Scale + Time * 1.0) + sin(Point.X * 2.3 / Scale + Time * 1.5) + sin(Point.X * 3.3 / Scale + Time * 0.4)) / 3.0;
	z += (sin(Point.Y * 0.2 / Scale + Time * 1.8) + sin(Point.Y * 1.8 / Scale + Time * 1.8) + sin(Point.Y * 2.8 / Scale + Time * 0.8)) / 3.0;
	return z;
}

void UStaticFuncLib::MakeComplementaryColorScheme(FLinearColor Root, TArray<FLinearColor>& OutColors, const TArray<FVector2D>& InSatVals)
{
	FLinearColor RootHSV = Root.LinearRGBToHSV();

	float Hue = RootHSV.R;
	float Sat = (InSatVals.Num() > 0 ? InSatVals[0].X : RootHSV.G);
	float Val = (InSatVals.Num() > 0 ? InSatVals[0].Y : RootHSV.B);

	OutColors.Add(FLinearColor(Hue, Sat, Val).HSVToLinearRGB());

	if (InSatVals.Num() > 1)
	{
		Sat = InSatVals[1].X;
		Val = InSatVals[1].Y;
	}
	else if (InSatVals.Num() > 0)
	{
		Sat = InSatVals.Last().X;
		Val = InSatVals.Last().Y;
	}

	FLinearColor ComplementHSV(FMath::Fmod(Hue + 180, 360), Sat, Val);

	OutColors.Add(ComplementHSV.HSVToLinearRGB());
}

void UStaticFuncLib::MakeAnalogousColorScheme(FLinearColor Root, int NumColors, float AngleHSV, TArray<FLinearColor>& OutColors,
	const TArray<FVector2D>& InSatVals)
{
	FLinearColor RootHSV = Root.LinearRGBToHSV();

	float Hue = RootHSV.R;
	float Sat = (InSatVals.Num() > 0 ? InSatVals[0].X : RootHSV.G);
	float Val = (InSatVals.Num() > 0 ? InSatVals[0].Y : RootHSV.B);

	OutColors.Add(FLinearColor(Hue, Sat, Val).HSVToLinearRGB());

	if (NumColors > 1)
	{
		while (--NumColors)
		{
			if (InSatVals.Num() > OutColors.Num())
			{
				Sat = InSatVals[OutColors.Num()].X;
				Val = InSatVals[OutColors.Num()].Y;
			}
			else if (InSatVals.Num() > 0)
			{
				Sat = InSatVals.Last().X;
				Val = InSatVals.Last().Y;
			}

			Hue = FMath::Fmod(Hue + AngleHSV, 360);
			OutColors.Add(FLinearColor(Hue, Sat, Val).HSVToLinearRGB());
		}
	}
}

void UStaticFuncLib::MakeTriadicColorScheme(FLinearColor Root, TArray<FLinearColor>& OutColors, const TArray<FVector2D>& InSatVals)
{
	FLinearColor RootHSV = Root.LinearRGBToHSV();

	float Hue = RootHSV.R;
	float Sat = (InSatVals.Num() > 0 ? InSatVals[0].X : RootHSV.G);
	float Val = (InSatVals.Num() > 0 ? InSatVals[0].Y : RootHSV.B);

	OutColors.Add(FLinearColor(Hue, Sat, Val).HSVToLinearRGB());

	if (InSatVals.Num() > 1)
	{
		Sat = InSatVals[1].X;
		Val = InSatVals[1].Y;
	}
	else if (InSatVals.Num() > 0)
	{
		Sat = InSatVals.Last().X;
		Val = InSatVals.Last().Y;
	}

	OutColors.Add(FLinearColor(FMath::Fmod(Hue + 120, 360), Sat, Val).HSVToLinearRGB());

	if (InSatVals.Num() > 2)
	{
		Sat = InSatVals[2].X;
		Val = InSatVals[2].Y;
	}
	else if (InSatVals.Num() > 0)
	{
		Sat = InSatVals.Last().X;
		Val = InSatVals.Last().Y;
	}

	OutColors.Add(FLinearColor(FMath::Fmod(Hue + 240, 360), Sat, Val).HSVToLinearRGB());
}

void UStaticFuncLib::MakeSplitComplementaryColorScheme(FLinearColor Root, float SplitAngleHSV, TArray<FLinearColor>& OutColors,
	const TArray<FVector2D>& InSatVals)
{
	FLinearColor RootHSV = Root.LinearRGBToHSV();

	float Hue = RootHSV.R;
	float Sat = (InSatVals.Num() > 0 ? InSatVals[0].X : RootHSV.G);
	float Val = (InSatVals.Num() > 0 ? InSatVals[0].Y : RootHSV.B);

	OutColors.Add(FLinearColor(Hue, Sat, Val).HSVToLinearRGB());

	if (InSatVals.Num() > 1)
	{
		Sat = InSatVals[1].X;
		Val = InSatVals[1].Y;
	}
	else if (InSatVals.Num() > 0)
	{
		Sat = InSatVals.Last().X;
		Val = InSatVals.Last().Y;
	}

	OutColors.Add(FLinearColor(FMath::Fmod(Hue + 180 + SplitAngleHSV, 360), Sat, Val).HSVToLinearRGB());

	if (InSatVals.Num() > 2)
	{
		Sat = InSatVals[2].X;
		Val = InSatVals[2].Y;
	}
	else if (InSatVals.Num() > 0)
	{
		Sat = InSatVals.Last().X;
		Val = InSatVals.Last().Y;
	}

	OutColors.Add(FLinearColor(FMath::Fmod(Hue + 180 - SplitAngleHSV, 360), Sat, Val).HSVToLinearRGB());

}

void UStaticFuncLib::MakeTetradicColorScheme(FLinearColor Root, float SplitAngleHSV, TArray<FLinearColor>& OutColors,
	const TArray<FVector2D>& InSatVals)
{
	FLinearColor RootHSV = Root.LinearRGBToHSV();

	float Hue = RootHSV.R;
	float Sat = (InSatVals.Num() > 0 ? InSatVals[0].X : RootHSV.G);
	float Val = (InSatVals.Num() > 0 ? InSatVals[0].Y : RootHSV.B);

	OutColors.Add(FLinearColor(FMath::Fmod(Hue  + SplitAngleHSV, 360), Sat, Val).HSVToLinearRGB());

	if (InSatVals.Num() > 1)
	{
		Sat = InSatVals[1].X;
		Val = InSatVals[1].Y;
	}
	else if (InSatVals.Num() > 0)
	{
		Sat = InSatVals.Last().X;
		Val = InSatVals.Last().Y;
	}

	OutColors.Add(FLinearColor(FMath::Fmod(Hue - SplitAngleHSV, 360), Sat, Val).HSVToLinearRGB());

	if (InSatVals.Num() > 2)
	{
		Sat = InSatVals[2].X;
		Val = InSatVals[2].Y;
	}
	else if (InSatVals.Num() > 0)
	{
		Sat = InSatVals.Last().X;
		Val = InSatVals.Last().Y;
	}

	OutColors.Add(FLinearColor(FMath::Fmod(Hue + 180 + SplitAngleHSV, 360), Sat, Val).HSVToLinearRGB());

	if (InSatVals.Num() > 3)
	{
		Sat = InSatVals[3].X;
		Val = InSatVals[3].Y;
	}
	else if (InSatVals.Num() > 0)
	{
		Sat = InSatVals.Last().X;
		Val = InSatVals.Last().Y;
	}

	OutColors.Add(FLinearColor(FMath::Fmod(Hue + 180 - SplitAngleHSV, 360), Sat, Val).HSVToLinearRGB());
}

void UStaticFuncLib::MakeSquareColorScheme(FLinearColor Root, TArray<FLinearColor>& OutColors, const TArray<FVector2D>& InSatVals)
{
	FLinearColor RootHSV = Root.LinearRGBToHSV();

	float Hue = RootHSV.R;
	float Sat = (InSatVals.Num() > 0 ? InSatVals[0].X : RootHSV.G);
	float Val = (InSatVals.Num() > 0 ? InSatVals[0].Y : RootHSV.B);

	OutColors.Add(FLinearColor(Hue, Sat, Val).HSVToLinearRGB());

	if (InSatVals.Num() > 1)
	{
		Sat = InSatVals[1].X;
		Val = InSatVals[1].Y;
	}
	else if (InSatVals.Num() > 0)
	{
		Sat = InSatVals.Last().X;
		Val = InSatVals.Last().Y;
	}

	OutColors.Add(FLinearColor(FMath::Fmod(Hue + 90, 360), Sat, Val).HSVToLinearRGB());

	if (InSatVals.Num() > 2)
	{
		Sat = InSatVals[2].X;
		Val = InSatVals[2].Y;
	}
	else if (InSatVals.Num() > 0)
	{
		Sat = InSatVals.Last().X;
		Val = InSatVals.Last().Y;
	}

	OutColors.Add(FLinearColor(FMath::Fmod(Hue + 180, 360), Sat, Val).HSVToLinearRGB());

	if (InSatVals.Num() > 3)
	{
		Sat = InSatVals[3].X;
		Val = InSatVals[3].Y;
	}
	else if (InSatVals.Num() > 0)
	{
		Sat = InSatVals.Last().X;
		Val = InSatVals.Last().Y;
	}

	OutColors.Add(FLinearColor(FMath::Fmod(Hue + 270, 360), Sat, Val).HSVToLinearRGB());
}

void UStaticFuncLib::MakeMonochromeColorScheme(FLinearColor Root, int NumColors, FVector2D IntervalRange, TArray<FLinearColor>& OutColors)
{
	FLinearColor RootHSV = Root.LinearRGBToHSV();

	float Hue = RootHSV.R;
	float Sat = RootHSV.G;
	float Val = RootHSV.B + FMath::FRandRange(IntervalRange.X, IntervalRange.Y);

	OutColors.Add(Root);

	if (NumColors > 1)
	{
		while (--NumColors)
		{
			OutColors.Add(FLinearColor(Hue, Sat, Val).HSVToLinearRGB());
			Val += FMath::FRandRange(IntervalRange.X, IntervalRange.Y);
		}
	}
}

void UStaticFuncLib::MakeRandomColorScheme(int NumColors, TArray<FLinearColor>& OutColors, const TArray<FVector2D>& InSatVals,
	float AnalogousHueAngle, float SplitCompHueAngle, float TetradicHueAngle, FVector2D MonochromeValRange)
{
	FLinearColor RandomRoot = FLinearColor(FMath::FRandRange(0.0f, 359.0f), 1, 1).HSVToLinearRGB();

	if (NumColors > 1)
	{
		enum ColorSchemesEnum
		{
			Complementary,
			Analog,
			Triad,
			Split,
			Tetrad,
			Square,
			Monochrome
		};

		TArray<ColorSchemesEnum> ValidSchemes;

		if (NumColors == 2) ValidSchemes = {Complementary, Analog, Monochrome};
		else if (NumColors == 3) ValidSchemes = {Analog, Triad, Split, Monochrome};
		else if (NumColors == 4) ValidSchemes = {Analog, Tetrad, Square, Monochrome};
		else ValidSchemes = {Analog, Monochrome};

		switch (ValidSchemes[FMath::RandRange(0, ValidSchemes.Num() - 1)])
		{
		case Complementary:
			MakeComplementaryColorScheme(RandomRoot, OutColors, InSatVals);
			break;

		case Analog:
			MakeAnalogousColorScheme(RandomRoot, NumColors, AnalogousHueAngle, OutColors, InSatVals);
			break;

		case Triad:
			MakeTriadicColorScheme(RandomRoot, OutColors, InSatVals);
			break;

		case Split:
			MakeSplitComplementaryColorScheme(RandomRoot, SplitCompHueAngle, OutColors, InSatVals);
			break;

		case Tetrad:
			MakeTetradicColorScheme(RandomRoot, TetradicHueAngle, OutColors, InSatVals);
			break;

		case Square:
			MakeSquareColorScheme(RandomRoot, OutColors, InSatVals);
			break;

		default:
			Print("UStaticFuncLib::MakeRandomColorScheme: Invalid color scheme rolled! Remember to add support for it in this function.", true);
		case Monochrome:
			MakeMonochromeColorScheme(RandomRoot, NumColors, MonochromeValRange, OutColors);
			break;
		}
	}
	else
		OutColors.Add(RandomRoot);
}

void UStaticFuncLib::MakeRandomHorseColorScheme(TArray<FLinearColor>& OutColors, int BodyColors, int HairColors, int EyeColors)
{
	// TODO ALLOW USER OVERRIDES

	if (BodyColors > 0 && HairColors > 0 && EyeColors > 0)
	{
		// Get root (body) Hue
		float BodyHue = FMath::FRandRange(0, 359.9f);
		float EyeHue, HairHue;

		if (FMath::RandBool()) 
		{
			if (FMath::RandBool())
			{
				// Split compliment color scheme.

				float ComplementAngle = FMath::FRandRange(10, 40) * (FMath::RandBool() ? -1 : 1);

				EyeHue = FMath::Fmod(BodyHue + ComplementAngle + 180, 360);
				HairHue = FMath::Fmod(BodyHue + (ComplementAngle * 2), 360);
			}
			else
			{

				// Semi-tetrad color scheme.

				float TetAngle = FMath::FRandRange(10, 70);

				HairHue = FMath::Fmod(BodyHue + TetAngle, 360);
				EyeHue = FMath::Fmod(BodyHue + 180, 360);
			}
		}
		else
		{
			// Analogous color scheme.

			float MoveAngle = FMath::FRandRange(6, 14);

			TArray<float> Analogs =
			{
				BodyHue,
				BodyHue + MoveAngle,
				BodyHue + (MoveAngle * 2),
				BodyHue + (MoveAngle * 3),
				BodyHue + (MoveAngle * 4)
			};

			int RandInd = FMath::RandRange(0, Analogs.Num() - 1);

			BodyHue = Analogs[RandInd];
			Analogs.RemoveAt(RandInd);

			RandInd = FMath::RandRange(0, Analogs.Num() - 1);

			HairHue = Analogs[RandInd];
			Analogs.RemoveAt(RandInd);

			RandInd = FMath::RandRange(0, Analogs.Num() - 1);

			EyeHue = Analogs[RandInd];
			Analogs.RemoveAt(RandInd);
		}


		TArray<float> BodyHues = { BodyHue }, HairHues = { HairHue }, EyeHues = { EyeHue };

		if (BodyColors > 1)
		{
			// Analogous Extras

			while (--BodyColors)
				BodyHues.Add(FMath::Fmod(BodyHue + (FMath::RandRange(1, 5) * 6.0f), 360));
		}

		if (HairColors > 1)
		{
			float FocusHue;
			if (FMath::RandBool())
			{
				if (FMath::RandBool())
					FocusHue = HairHue; // Analogous Extras
				else
					FocusHue = FMath::Fmod(HairHue + (60.0f * (FMath::RandBool() ? -1 : 1)), 360); // Semi-Triad Extras
			}
			else
				FocusHue = FMath::Fmod(HairHue + 180, 360); // Complementary Extras

			while (--HairColors)
				HairHues.Add(FMath::Fmod(BodyHue + (FMath::RandRange(1, 10) * 6.0f), 360));
		}

		if (EyeColors > 1)
		{
			// Analogous Extras

			while (--EyeColors)
				EyeHues.Add(FMath::Fmod(EyeHue + (FMath::RandRange(1, 5) * 6.0f), 360));
		}


		// Mess with the Saturations and Values a bit. REMEMBER: Sat + Val >= 1! Makes sure colors stay bright!

		TArray<FLinearColor> BodCols, HairCols, EyeCols;

		float Sat, Val;

		for (float& currHue : BodyHues)
		{
			//if (FMath::RandBool())
			//{
			//	if (FMath::RandBool())
			//		Val = FMath::FRandRange(FMath::Max(1 - (Sat = FMath::FRand()), 0.5f), 1);
			//	else
			//		Sat = FMath::FRandRange(1 - (Val = FMath::FRandRange(.2f, 1)), 1);
			//}
			//else
			//	Sat = Val = 1;
			//
			//BodCols.Add(FLinearColor(currHue, Sat, Val).HSVToLinearRGB());

			Sat = FMath::FRandRange(0.1f, 0.9f);
			if (FMath::RandBool())
				Val = FMath::FRandRange(0.5f, 1.0f);
			else
				Val = 1;

			BodCols.Add(FLinearColor(currHue, Sat, Val).HSVToLinearRGB());

		}

		for (float& currHue : HairHues)
		{
			if (FMath::RandBool())
			{
				if (FMath::RandBool())
					Val = FMath::FRandRange(1 - (Sat = FMath::FRand()), 1);
				else
					Sat = FMath::FRandRange(1 - (Val = FMath::FRand()), 1);
			}
			else
				Sat = Val = 1;

			HairCols.Add(FLinearColor(currHue, Sat, Val).HSVToLinearRGB());
		}

		for (float& currHue : EyeHues)
		{
			if (FMath::RandBool())
			{
				if (FMath::RandBool())
					Val = FMath::FRandRange(1 - (Sat = FMath::FRandRange(0.3f,1)), 1);
				else
					Sat = FMath::FRandRange(FMath::Max(1 - (Val = FMath::FRand()), 0.2f), 1);
			}
			else
				Sat = Val = 1;

			EyeCols.Add(FLinearColor(currHue, Sat, Val).HSVToLinearRGB());
		}

		OutColors.Append(BodCols);
		OutColors.Append(HairCols);
		OutColors.Append(EyeCols);
	}
}

UTexture2D* UStaticFuncLib::MakeTextureForColorScheme(const TArray<FLinearColor>& Colors)
{
	UTexture2D* NewTex = UTexture2D::CreateTransient(Colors.Num(), 1);

	NewTex->Filter = TextureFilter::TF_Nearest;

	auto& MipZero = NewTex->PlatformData->Mips[0];

	FColor* MipData = static_cast<FColor*>(MipZero.BulkData.Lock(LOCK_READ_WRITE));

	for (int i = 0; i < Colors.Num(); ++i)
		MipData[i] = Colors[i].ToFColor(true);

	MipZero.BulkData.Unlock();
	NewTex->UpdateResource();

	return NewTex;
}

FVector2D UStaticFuncLib::ViewportToAbsolute(UObject* WorldContextObject, FVector2D ViewportPosition)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject);
	if (World && World->IsGameWorld())
	{
		if (UGameViewportClient* ViewportClient = World->GetGameViewport())
		{
			if (FSceneViewport* Viewport = ViewportClient->GetGameViewport())
			{
				FVector2D ViewportSize;
				ViewportClient->GetViewportSize(ViewportSize);

				FVector2D PixelPosition = ViewportPosition * UWidgetLayoutLibrary::GetViewportScale(ViewportClient);

				FVector2D AbsoluteDesktopCoordinate = Viewport->ViewportToVirtualDesktopPixel(PixelPosition / ViewportSize);

				return AbsoluteDesktopCoordinate;
			}
		}
	}

	return FVector2D(0, 0);
}

FVector2D UStaticFuncLib::ViewportToLocal(UObject* WorldContextObject, const FGeometry& Geometry, FVector2D ViewportPosition)
{
	FVector2D AbsoluteCoordinate = ViewportToAbsolute(WorldContextObject, ViewportPosition);
	return Geometry.AbsoluteToLocal(AbsoluteCoordinate);
}

void UStaticFuncLib::ReleaseButton(UButton* Button)
{
	TSharedPtr<SWidget> RetrievedCached = Button->GetCachedWidget();

	if (RetrievedCached.IsValid())
	{
		FPointerEvent Dummy(0, {}, {}, {}, EKeys::LeftMouseButton, 0, FModifierKeysState());
		RetrievedCached->OnMouseButtonUp(Button->GetCachedGeometry(), Dummy);
	}
}

void UStaticFuncLib::LeaveButton(class UButton* Button)
{
	TSharedPtr<SWidget> RetrievedCached = Button->GetCachedWidget();

	if (RetrievedCached.IsValid())
	{
		FPointerEvent Dummy(0, {}, {}, {}, EKeys::LeftMouseButton, 0, FModifierKeysState());
		RetrievedCached->OnMouseLeave(Dummy);
	}
}

FLinearColor UStaticFuncLib::GetColorAtScreenPos(FVector2D ScreenPos)
{
	//float Gamma = 1;//DisplayGamma.Get(2.2f) / 2.2f; // TODO somehow get the actual gamma (although everything seems to really want to stop me)
	return FPlatformMisc::GetScreenPixelColor(ScreenPos/*, Gamma*/);
}

FLinearColor UStaticFuncLib::GetColorAtCursorPos()
{
	return FPlatformMisc::GetScreenPixelColor(FSlateApplication::Get().GetCursorPos()/*, Gamma*/);
}

void UStaticFuncLib::SetToPreTick(UWidget* Widget, FString FuncName)
{
	FSlateApplication::Get().OnPreTick().AddUFunction(Widget, *FuncName);
}

void UStaticFuncLib::ClearAllFromPreTick(UWidget* Widget)
{
	FSlateApplication::Get().OnPreTick().RemoveAll(Widget);
}

void UStaticFuncLib::ForceCursorQuery()
{
	FSlateApplication::Get().QueryCursor();
}

UTexture2D* UStaticFuncLib::MakeTextureFromRenderTarget(UTextureRenderTarget2D* Target, TArray<FLinearColor>& OutColors, bool InvertOpacity)
{
	if (nullptr != Target)
	{
		UTexture2D* ConstructedTex = Target->ConstructTexture2D((UObject*)GetTransientPackage(), "NewRT_Tex", EObjectFlags::RF_NoFlags);

		auto& MipZero = ConstructedTex->PlatformData->Mips[0];

		if (ConstructedTex->Source.GetFormat() == TSF_BGRA8)
		{
			FColor* MipData = static_cast<FColor*>(MipZero.BulkData.Lock(LOCK_READ_WRITE));

			int TotalCells = (ConstructedTex->GetSizeY() - 1) * ConstructedTex->GetSurfaceWidth() + (ConstructedTex->GetSizeX() - 1);

			for (int i = 0; i <= TotalCells; ++i)
			{
				if (InvertOpacity)
					MipData[i].A = 255 - MipData[i].A;

				OutColors.Add(MipData[i]);
			}
		}
		else if (ConstructedTex->Source.GetFormat() == TSF_RGBA16F)
		{
			FFloat16Color* MipData = static_cast<FFloat16Color*>(MipZero.BulkData.Lock(LOCK_READ_WRITE));

			int TotalCells = (ConstructedTex->GetSizeY() - 1) * ConstructedTex->GetSurfaceWidth() + (ConstructedTex->GetSizeX() - 1);

			for (int i = 0; i <= TotalCells; ++i)
			{
				if (InvertOpacity)
					MipData[i].A = 1.0f - MipData[i].A;

				OutColors.Add({ MipData[i].R,MipData[i].G,MipData[i].B,MipData[i].A });
			}
		}

		if (MipZero.BulkData.IsLocked())
		{
			MipZero.BulkData.Unlock();
			ConstructedTex->UpdateResource();
		}

		return ConstructedTex;
	}

	return nullptr;
}

FString UStaticFuncLib::ViggishCipherEncrypt(FString Base, FString Key, FString Alphabet)
{
	FString ResultStr;
	int MsgLen = Base.Len() + 1;

	for (int i = 0; i < MsgLen; ++i)
	{
		int Row, Col;
		Alphabet.FindChar(Base[i], Col);
		Alphabet.FindChar(Key[i%MsgLen], Row);

		if (Row != INDEX_NONE && Col != INDEX_NONE)
			ResultStr += Alphabet[(Row + Col) % Alphabet.Len() + 1];
		else
			ResultStr += Base[i];

	}

	return ResultStr;
}

FString UStaticFuncLib::ViggishCipherDecrypt(FString Base, FString Key, FString Alphabet)
{
	FString ResultStr;
	int MsgLen = Base.Len() + 1;
	for (int i = 0; i < MsgLen; ++i)
	{
		int Row, Col;
		Alphabet.FindChar(Base[i], Col);
		Alphabet.FindChar(Key[i%MsgLen], Row);

		if (Row != INDEX_NONE && Col != INDEX_NONE)
			ResultStr += Alphabet[(Col - Row + Alphabet.Len() + 1) % Alphabet.Len() + 1];
		else
			ResultStr += Base[i];

	}

	return ResultStr;
}
