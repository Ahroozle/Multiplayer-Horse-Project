// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "MPHorsoMusicManager.h"

#include "StaticFuncLib.h"
#include "MPHorsoGameInstance.h"
#include "MPHorsoSettingsSave.h"

#include "Kismet/KismetMathLibrary.h"


// Sets default values
AMPHorsoMusicManager::AMPHorsoMusicManager(const FObjectInitializer& _init) : Super(_init)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* currRoot = RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));

	FAttachmentTransformRules AttachRules(EAttachmentRule::KeepRelative, false);

	BGM_A = _init.CreateDefaultSubobject<UAudioComponent>(this, TEXT("BGM_A"));
	BGM_A->AttachToComponent(currRoot, AttachRules);
	BGM_A->SetRelativeLocation(FVector::ZeroVector);

	BGM_B = _init.CreateDefaultSubobject<UAudioComponent>(this, TEXT("BGM_B"));
	BGM_B->AttachToComponent(currRoot, AttachRules);
	BGM_B->SetRelativeLocation(FVector::ZeroVector);

	Current_BGM_Component = BGM_A;


	Ambience_A = _init.CreateDefaultSubobject<UAudioComponent>(this, TEXT("Ambience_A"));
	Ambience_A->AttachToComponent(currRoot, AttachRules);
	Ambience_A->SetRelativeLocation(FVector::ZeroVector);

	Ambience_B = _init.CreateDefaultSubobject<UAudioComponent>(this, TEXT("Ambience_B"));
	Ambience_B->AttachToComponent(currRoot, AttachRules);
	Ambience_B->SetRelativeLocation(FVector::ZeroVector);

	Current_Ambience_Component = Ambience_A;

}

// Called when the game starts or when spawned
void AMPHorsoMusicManager::BeginPlay()
{
	Super::BeginPlay();
	
	UMPHorsoGameInstance* gameInst = UStaticFuncLib::RetrieveGameInstance(this);

	if (nullptr != gameInst)
	{
		UMPHorsoSettingsSave* Settings = gameInst->GetSettingsSave();

		BGM_A->SetVolumeMultiplier(Settings->MasterVolume * Settings->BGM_Volume);
		BGM_B->SetVolumeMultiplier(Settings->MasterVolume * Settings->BGM_Volume);
		Ambience_A->SetVolumeMultiplier(Settings->MasterVolume * Settings->AmbienceVolume);
		Ambience_B->SetVolumeMultiplier(Settings->MasterVolume * Settings->AmbienceVolume);
	}
	else
		UStaticFuncLib::Print("AMPHorsoMusicManager::BeginPlay: Couldn't retrieve the game instance! "
							  "BGM and Ambience Volume will not be changed to fit settings as a result.", true);

}

// Called every frame
void AMPHorsoMusicManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMPHorsoMusicManager::SetBGMVolume(float NewVolume)
{
	UMPHorsoGameInstance* gameInst = UStaticFuncLib::RetrieveGameInstance(this);

	if (nullptr != gameInst)
	{
		UMPHorsoSettingsSave* Settings = gameInst->GetSettingsSave();

		BGM_A->SetVolumeMultiplier(Settings->MasterVolume * NewVolume);
		BGM_B->SetVolumeMultiplier(Settings->MasterVolume * NewVolume);
	}
	else
		UStaticFuncLib::Print("AMPHorsoMusicManager::SetBGMVolume: Couldn't retrieve the game instance! "
							  "BGM Volume will not be changed to fit settings as a result.", true);
}

void AMPHorsoMusicManager::SetAmbiVolume(float NewVolume)
{
	UMPHorsoGameInstance* gameInst = UStaticFuncLib::RetrieveGameInstance(this);

	if (nullptr != gameInst)
	{
		UMPHorsoSettingsSave* Settings = gameInst->GetSettingsSave();

		Ambience_A->SetVolumeMultiplier(Settings->MasterVolume * NewVolume);
		Ambience_B->SetVolumeMultiplier(Settings->MasterVolume * NewVolume);
	}
	else
		UStaticFuncLib::Print("AMPHorsoMusicManager::SetAmbiVolume: Couldn't retrieve the game instance! "
							  "Ambience Volume will not be changed to fit settings as a result.", true);
}

void AMPHorsoMusicManager::CutBGMTo(USoundCue* NewBGM)
{
	Current_BGM_Component->Stop();
	Current_BGM_Component->SetSound(NewBGM);
	Current_BGM_Component->Play();
}

void AMPHorsoMusicManager::CutAmbiTo(USoundCue* NewAmbience)
{
	Current_Ambience_Component->Stop();
	Current_Ambience_Component->SetSound(NewAmbience);
	Current_Ambience_Component->Play();
}

void AMPHorsoMusicManager::CrossfadeBGMTo(USoundCue* NewBGM, float FadeTime)
{
	Current_BGM_Component->FadeOut(FadeTime, 0.0f);

	Current_BGM_Component = (Current_BGM_Component == BGM_A ? BGM_B : BGM_A);
	Current_BGM_Component->SetSound(NewBGM);

	float VolMul = 1.0f;
	UMPHorsoGameInstance* gameInst = UStaticFuncLib::RetrieveGameInstance(this);

	if (nullptr != gameInst)
	{
		UMPHorsoSettingsSave* Settings = gameInst->GetSettingsSave();

		VolMul = Settings->MasterVolume * Settings->BGM_Volume;
	}
	else
		UStaticFuncLib::Print("AMPHorsoMusicManager::CrossfadeBGMTo: Couldn't retrieve the game instance! "
							  "BGM Volume will not be changed to fit settings as a result.", true);

	Current_BGM_Component->FadeIn(FadeTime, VolMul);
}

void AMPHorsoMusicManager::CrossfadeAmbiTo(USoundCue* NewAmbience, float FadeTime)
{
	Current_Ambience_Component->FadeOut(FadeTime, 0.0f);

	Current_Ambience_Component = (Current_Ambience_Component == Ambience_A ? Ambience_B : Ambience_A);
	Current_Ambience_Component->SetSound(NewAmbience);

	float VolMul = 1.0f;
	UMPHorsoGameInstance* gameInst = UStaticFuncLib::RetrieveGameInstance(this);

	if (nullptr != gameInst)
	{
		UMPHorsoSettingsSave* Settings = gameInst->GetSettingsSave();

		VolMul = Settings->MasterVolume * Settings->BGM_Volume;
	}
	else
		UStaticFuncLib::Print("AMPHorsoMusicManager::CrossfadeAmbiTo: Couldn't retrieve the game instance! "
							  "BGM Volume will not be changed to fit settings as a result.", true);

	Current_Ambience_Component->FadeIn(FadeTime, VolMul);
}


AMusicTrigger::AMusicTrigger(const FObjectInitializer& _init) : Super(_init)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* currRoot = RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));
}

void AMusicTrigger::BeginPlay()
{
	Super::BeginPlay();

	UMPHorsoGameInstance* gameInst = UStaticFuncLib::RetrieveGameInstance(this);

	if (nullptr != gameInst)
		RetrievedMusicManager = gameInst->GetMusicManager();
	else
		UStaticFuncLib::Print("AMusicTrigger::BeginPlay: Couldn't retrieve the game instance! "
							  "BGM and/or Ambience won't be changed as a result.", true);
}

void AMusicTrigger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMusicTrigger::NotifyActorBeginOverlap(AActor* OtherActor)
{
	AActor* OtherOwner = OtherActor->GetOwner();

	if (nullptr != OtherOwner)
	{
		APawn* CastedToPawn = Cast<APawn>(OtherOwner);

		if (nullptr != CastedToPawn && CastedToPawn->GetController() == UGameplayStatics::GetPlayerController(this, 0))
		{
			if (nullptr != RetrievedMusicManager)
			{
				if (ChangesBGM)
				{
					if (CutsBGM)
						RetrievedMusicManager->CutBGMTo(BGM);
					else
						RetrievedMusicManager->CrossfadeBGMTo(BGM, BGM_FadeSpeed);
				}

				if (ChangesAmbience)
				{
					if (CutsAmbience)
						RetrievedMusicManager->CutAmbiTo(Ambience);
					else
						RetrievedMusicManager->CrossfadeAmbiTo(Ambience, Ambience_FadeSpeed);
				}
			}
			else
				UStaticFuncLib::Print("AMusicTrigger::NotifyActorBeginOverlap: Couldn't retrieve the music manager! "
					"BGM and/or Ambience won't be changed as a result.", true);
		}
	}
}


UMusicModifierComponent::UMusicModifierComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...
}

// Called when the game starts
void UMusicModifierComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

}


// Called every frame
void UMusicModifierComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


AMusicModifierTrigger::AMusicModifierTrigger(const FObjectInitializer& _init) : Super(_init)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* currRoot = RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));
}

void AMusicModifierTrigger::BeginPlay()
{
	Super::BeginPlay();

	TArray<USceneComponent*> RetrievedChildren;
	RootComponent->GetChildrenComponents(true, RetrievedChildren);

	UMusicModifierComponent* casted;
	for (auto *currChild : RetrievedChildren)
	{
		casted = Cast<UMusicModifierComponent>(currChild);

		if (nullptr != casted)
			ModifierPoints.Add(casted);
	}

	if (ModifierPoints.Num() < 1)
	{
		UStaticFuncLib::Print("AMusicModifierTrigger::BeginPlay: No UMusicModifierComponents found! Removing self from play.", true);
		Destroy();
	}

	UMPHorsoGameInstance* gameInst = UStaticFuncLib::RetrieveGameInstance(this);

	if (nullptr != gameInst)
		RetrievedMusicManager = gameInst->GetMusicManager();
	else
		UStaticFuncLib::Print("AMusicModifierTrigger::BeginPlay: Couldn't retrieve the game instance! "
							  "BGM and/or Ambience won't be changed as a result.", true);
}

void AMusicModifierTrigger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMusicModifierTrigger::NotifyActorBeginOverlap(AActor* OtherActor)
{
	AActor* OtherOwner = OtherActor->GetOwner();

	if (nullptr != OtherOwner)
	{
		APawn* CastedToPawn = Cast<APawn>(OtherOwner);

		if (nullptr != CastedToPawn && CastedToPawn->GetController() == UGameplayStatics::GetPlayerController(this, 0))
		{
			StoredObserver = CastedToPawn;

			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AMusicModifierTrigger::ModifyMusic, 0.01f, true);
		}
	}
}

void AMusicModifierTrigger::NotifyActorEndOverlap(AActor* OtherActor)
{
	AActor* OtherOwner = OtherActor->GetOwner();

	if (nullptr != OtherOwner && OtherOwner == StoredObserver)
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}

void AMusicModifierTrigger::SetParametersForAud(UAudioComponent* TargetAud, const TMap<FName, FMusicParamStruct>& ParamList)
{
	for (auto &currParam : ParamList)
	{
		switch (currParam.Value.ParameterType)
		{
		case EMusicParamType::EMus_Bool:
			TargetAud->SetBoolParameter(currParam.Key, currParam.Value.BoolValue);
			break;
		case EMusicParamType::EMus_Float:
			TargetAud->SetFloatParameter(currParam.Key, currParam.Value.FloatValue);
			break;
		case EMusicParamType::EMus_Int:
			TargetAud->SetIntParameter(currParam.Key, currParam.Value.IntValue);
			break;
		case EMusicParamType::EMus_Wave:
			TargetAud->SetWaveParameter(currParam.Key, currParam.Value.WaveValue);
			break;
		default:
			break;
		}
	}
}

void AMusicModifierTrigger::ModifyMusic()
{
	if (ModifierPoints.Num() < 2)
	{
		if (nullptr != RetrievedMusicManager)
		{
			SetParametersForAud(RetrievedMusicManager->Current_BGM_Component, ModifierPoints[0]->MusicParamsBGM);
			SetParametersForAud(RetrievedMusicManager->Current_Ambience_Component, ModifierPoints[0]->MusicParamsBGM);
		}

		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	}
	else
	{
		FVector ObsLoc = StoredObserver->GetActorLocation();

		if (ObsLoc.Equals(LastObsLoc,0.0001f))
			return;

		LastObsLoc = ObsLoc;

		auto Lamb = [&ObsLoc](UMusicModifierComponent& a, UMusicModifierComponent& b)
		{
			return FVector::Dist(a.GetComponentLocation(), ObsLoc) < FVector::Dist(b.GetComponentLocation(), ObsLoc);
		};

		ModifierPoints.Sort(Lamb);

		FVector StartLoc = ModifierPoints[0]->GetComponentLocation();
		FVector EndLoc = ModifierPoints[1]->GetComponentLocation();

		FVector ClosestPoint = UKismetMathLibrary::FindClosestPointOnSegment(ObsLoc, StartLoc, EndLoc);

		float fromstartdist = (ClosestPoint - StartLoc).Size();
		float wholedist = (EndLoc - StartLoc).Size();

		float Ratio = FMath::Clamp((fromstartdist / wholedist) * (ModifierPoints[1]->Weight / ModifierPoints[0]->Weight), 0.0f, 1.0f);


		FMusicParamStruct* Found = nullptr;

		// Interpolating BGM Parameters
		TMap<FName, FMusicParamStruct> ConglomParams = ModifierPoints[0]->MusicParamsBGM;
		for (auto &curr : ModifierPoints[1]->MusicParamsBGM)
		{
			if (nullptr != (Found = ConglomParams.Find(curr.Key)))
			{
				if (curr.Value.ParameterType == Found->ParameterType)
				{
					switch (curr.Value.ParameterType)
					{
					case EMusicParamType::EMus_Bool:
						if (Ratio > 0.5f)
							Found->BoolValue = curr.Value.BoolValue;
						break;
					case EMusicParamType::EMus_Float:
						Found->FloatValue = FMath::Lerp(Found->FloatValue, curr.Value.FloatValue, Ratio);
						break;
					case EMusicParamType::EMus_Int:
						Found->IntValue = FMath::Lerp(Found->IntValue, curr.Value.IntValue, Ratio);
						break;
					case EMusicParamType::EMus_Wave:
						if (Ratio > 0.5f)
							Found->WaveValue = curr.Value.WaveValue;
						break;
					default:
						break;
					}
				}
				else
					UStaticFuncLib::Print("AMusicModifierTrigger::ModifyMusic: BGM Parameter \'" + curr.Key.ToString() +
										  "\' Did not match types between Modifier Points \'" + ModifierPoints[0]->GetName() +
										  "\' and \'" + ModifierPoints[1]->GetName() + "\'! Defaulting to the former's version.", true);
			}
			else
				ConglomParams.Add(curr.Key, curr.Value);
		}

		SetParametersForAud(RetrievedMusicManager->Current_BGM_Component, ConglomParams);


		Found = nullptr;

		// Interpolating Ambience Parameters
		ConglomParams = ModifierPoints[0]->MusicParamsAmbience;
		for (auto &curr : ModifierPoints[1]->MusicParamsAmbience)
		{
			if (nullptr != (Found = ConglomParams.Find(curr.Key)))
			{
				if (curr.Value.ParameterType == Found->ParameterType)
				{
					switch (curr.Value.ParameterType)
					{
					case EMusicParamType::EMus_Bool:
						if (Ratio > 0.5f)
							Found->BoolValue = curr.Value.BoolValue;
						break;
					case EMusicParamType::EMus_Float:
						Found->FloatValue = FMath::Lerp(Found->FloatValue, curr.Value.FloatValue, Ratio);
						break;
					case EMusicParamType::EMus_Int:
						Found->IntValue = FMath::Lerp(Found->IntValue, curr.Value.IntValue, Ratio);
						break;
					case EMusicParamType::EMus_Wave:
						if (Ratio > 0.5f)
							Found->WaveValue = curr.Value.WaveValue;
						break;
					default:
						break;
					}
				}
				else
					UStaticFuncLib::Print("AMusicModifierTrigger::ModifyMusic: Ambience Parameter \'" + curr.Key.ToString() +
										  "\' Did not match types between Modifier Points \'" + ModifierPoints[0]->GetName() +
										  "\' and \'" + ModifierPoints[1]->GetName() + "\'! Defaulting to the former's version.", true);
			}
			else
				ConglomParams.Add(curr.Key, curr.Value);
		}

		SetParametersForAud(RetrievedMusicManager->Current_Ambience_Component, ConglomParams);

	}
}
