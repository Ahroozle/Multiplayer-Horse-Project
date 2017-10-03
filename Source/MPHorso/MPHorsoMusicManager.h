// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "MPHorsoMusicManager.generated.h"


// TODO : TAKE VOLUME SETTINGS INTO ACCOUNT
//
// also things like:
//			-create volume settings (Master, BGM, Ambience, and SFX separate) and tie them in appropriately.

/*
	Manager class for all Ambience and BGM!
*/
UCLASS(Blueprintable)
class MPHORSO_API AMPHorsoMusicManager : public AActor
{
	GENERATED_BODY()

	// 4 here to let me crossfade between cues.

	UPROPERTY(EditDefaultsOnly)
		UAudioComponent* BGM_A;
	UPROPERTY(EditDefaultsOnly)
		UAudioComponent* BGM_B;

	UPROPERTY(EditDefaultsOnly)
		UAudioComponent* Ambience_A;
	UPROPERTY(EditDefaultsOnly)
		UAudioComponent* Ambience_B;

public:	
	// Sets default values for this actor's properties
	AMPHorsoMusicManager(const FObjectInitializer& _init);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite)
		UAudioComponent* Current_BGM_Component;
	UPROPERTY(BlueprintReadWrite)
		UAudioComponent* Current_Ambience_Component;


	UFUNCTION(BlueprintCallable, Category = "Music Manager", meta = (DisplayName = "Set BGM Volume"))
		void SetBGMVolume(float NewVolume);
	UFUNCTION(BlueprintCallable, Category = "Music Manager", meta = (DisplayName = "Set Ambience Volume"))
		void SetAmbiVolume(float NewVolume);

	UFUNCTION(BlueprintCallable, Category = "Music Manager", meta = (DisplayName = "Cut BGM To"))
		void CutBGMTo(USoundCue* NewBGM);
	UFUNCTION(BlueprintCallable, Category = "Music Manager", meta = (DisplayName = "Cut Ambience To"))
		void CutAmbiTo(USoundCue* NewAmbience);

	UFUNCTION(BlueprintCallable, Category = "Music Manager", meta = (DisplayName = "Crossfade BGM To"))
		void CrossfadeBGMTo(USoundCue* NewBGM, float FadeTime = 1.0f);
	UFUNCTION(BlueprintCallable, Category = "Music Manager", meta = (DisplayName = "Crossfade Ambience To"))
		void CrossfadeAmbiTo(USoundCue* NewAmbience, float FadeTime = 1.0f);
	
};


/*
	
*/
UCLASS(BlueprintType)
class MPHORSO_API AMusicTrigger : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMusicTrigger(const FObjectInitializer& _init);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	UPROPERTY(EditAnywhere)
		bool ChangesBGM = false;
	UPROPERTY(EditAnywhere, meta = (EditCondition = "ChangesBGM"))
		USoundCue* BGM;
	UPROPERTY(EditAnywhere, meta = (EditCondition = "ChangesBGM"))
		bool CutsBGM;
	UPROPERTY(EditAnywhere, meta = (EditCondition = "!CutsBGM", ClampMin = "0.0", UIMin = "0.0"))
		float BGM_FadeSpeed = 1.0f;

	UPROPERTY(EditAnywhere)
		bool ChangesAmbience = false;
	UPROPERTY(EditAnywhere, meta = (EditCondition = "ChangesAmbience"))
		USoundCue* Ambience;
	UPROPERTY(EditAnywhere, meta = (EditCondition = "ChangesAmbience"))
		bool CutsAmbience;
	UPROPERTY(EditAnywhere, meta = (EditCondition = "!CutsAmbience", ClampMin = "0.0", UIMin = "0.0"))
		float Ambience_FadeSpeed = 1.0f;

	AMPHorsoMusicManager* RetrievedMusicManager;

};


UENUM(BlueprintType)
enum class EMusicParamType : uint8
{
	EMus_Bool		UMETA(DisplayName = "Bool Parameter"),
	EMus_Float		UMETA(DisplayName = "Float Parameter"),
	EMus_Int		UMETA(DisplayName = "Int Parameter"),
	EMus_Wave		UMETA(DisplayName = "Wave Parameter")
};

USTRUCT(BlueprintType)
struct FMusicParamStruct
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere)
		EMusicParamType ParameterType;

	UPROPERTY(EditAnywhere)
		bool BoolValue;

	UPROPERTY(EditAnywhere)
		float FloatValue;

	UPROPERTY(EditAnywhere)
		int IntValue;

	UPROPERTY(EditAnywhere)
		USoundWave* WaveValue;
};

/*

*/
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), BlueprintType)
class MPHORSO_API UMusicModifierComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UMusicModifierComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere)
		TMap<FName, FMusicParamStruct> MusicParamsBGM;
	UPROPERTY(EditAnywhere)
		TMap<FName, FMusicParamStruct> MusicParamsAmbience;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float Weight = 1;

};

UCLASS(BlueprintType)
class MPHORSO_API AMusicModifierTrigger : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMusicModifierTrigger(const FObjectInitializer& _init);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

	void SetParametersForAud(UAudioComponent* TargetAud, const TMap<FName, FMusicParamStruct>& ParamList);

	void ModifyMusic();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<UMusicModifierComponent*> ModifierPoints;

	UPROPERTY()
		APawn* StoredObserver;
	UPROPERTY()
		FTimerHandle TimerHandle;

	AMPHorsoMusicManager* RetrievedMusicManager;
	FVector LastObsLoc;

};
