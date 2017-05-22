// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "StaticFuncLib.h"

#include "Sound/SoundCue.h"
#include "OneShotAudio.h"

#include "MPHorsoGameInstance.h"

#include "SkeletalSpriteAnimation.h"

#include "Kismet/GameplayStatics.h"

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
	UGameInstance* gameInst = WorldContext->GetWorld()->GetGameInstance();
	if (ValidateObject(gameInst, "UStaticFuncLib::RetrieveGameInstance: Couldn't get the game instance!", true))
	{
		UMPHorsoGameInstance* castedInst = Cast<UMPHorsoGameInstance>(gameInst);
		if (ValidateObject(castedInst, "UStaticFuncLib::RetrieveGameInstance: Couldn't cast game instance to proper type!", true))
			return castedInst;
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

AActor* UStaticFuncLib::SpawnActorProxy(AActor* WorldContext, TSubclassOf<AActor> ToSpawn, const FTransform& SpawnLoc)
{
	return WorldContext->GetWorld()->SpawnActor(ToSpawn, &SpawnLoc);
}

UParticleSystemComponent* UStaticFuncLib::SpawnEmitterAtLocationProxy(AActor* WorldContext, UParticleSystem* Template, FVector SpawnLoc, FRotator SpawnRot, bool AutoDestroy)
{
	return UGameplayStatics::SpawnEmitterAtLocation(WorldContext, Template, SpawnLoc, SpawnRot, AutoDestroy);
}

void UStaticFuncLib::GetAllActorsOfClassProxy(AActor* WorldContext, TSubclassOf<AActor> Class, TArray<AActor*>& OutFound)
{
	UGameplayStatics::GetAllActorsOfClass(WorldContext, Class, OutFound);
}

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
