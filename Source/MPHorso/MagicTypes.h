// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "MagicTypes.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSpellUICancelled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSpellUIFinished);

USTRUCT()
struct FArgFunc
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditDefaultsOnly)
		FString EventName;

	// Name of the "root" component of the arg's component tree; used to help duplicate the entire arg.
	UPROPERTY(EditDefaultsOnly)
		FString TemplateComponentName;

	// Names of components in the arg's component tree which require spherical billboarding.
	UPROPERTY(EditDefaultsOnly)
		TSet<FString> BillboardComponents;

	// Names of components in the arg's component tree which require cylindrical billboarding.
	UPROPERTY(EditDefaultsOnly)
		TSet<FString> PoleComponents;

};

UCLASS(Blueprintable, abstract)
class MPHORSO_API AMagicUI : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMagicUI(const FObjectInitializer& _init);

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FName SpellName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		UMaterialInterface* SpellEmblem;

	// Map of Magic Arg Types to Events to be invoked on requesting them.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TMap<FName, FArgFunc> ArgFuncs;

	UPROPERTY(BlueprintReadOnly)
		USceneComponent* NewestArg;

	// References to components that require spherical billboarding
	UPROPERTY(VisibleAnywhere)
		TArray<USceneComponent*> FullBillboardTargets;

	// References to components that require cylindrical billboarding
	UPROPERTY(VisibleAnywhere)
		TArray<USceneComponent*> PoleBillboardTargets;

	// References to components attached to actors, for removal once the spell is cast or cancelled.
	UPROPERTY(VisibleAnywhere)
		TArray<USceneComponent*> MovingTargets;

	UPROPERTY(BlueprintReadWrite)
		TArray<USceneComponent*> TargetMarkers;

	UPROPERTY()
		int UniqueNum = 0;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FSpellUICancelled OnSpellCancelled;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FSpellUIFinished OnSpellFinished;


	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void Init();

	UFUNCTION()
		void TickBillboards();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		bool CastSpell(AActor* Caster);

	UFUNCTION(BlueprintImplementableEvent, BlueprintPure)
		float GetCurrentStaminaCost();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		bool SetPaired(AMagicUI* Paired);

	UFUNCTION()
		void CopyArg(USceneComponent* RootComp, const FArgFunc& ArgFunc);

	UFUNCTION()
		void CopyHelper(USceneComponent* CurrentToCopy, USceneComponent* Parent, const FArgFunc& ArgFunc);

	UFUNCTION(BlueprintCallable)
		bool RequestArg(FName ArgType);

	// Call this and pass in NewestArg and your selected moving target in order to fasten the arg to them.
	UFUNCTION(BlueprintCallable)
		void RegisterMovingArg(USceneComponent* NewMovingArg, AActor* Target);

	// Call this function before the UI destroys itself to make sure that moving target markers are removed!
	UFUNCTION(BlueprintCallable)
		void RecallMovingArgs();

	UFUNCTION(BlueprintCallable)
		void MarkTargets(USceneComponent* ToCopy, TArray<AActor*> Targets);

	UFUNCTION()
		USceneComponent* MarkHelper(USceneComponent* ToCopy);

	UFUNCTION(BlueprintCallable)
		void HideMarks();

	UFUNCTION(BlueprintCallable)
		void RecallMarks();

	UFUNCTION(BlueprintCallable)
		void Die();
};


UCLASS()
class MPHORSO_API UMagicActionsLibrary : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContext"))
		static TSubclassOf<AMagicUI> LoadSpellSynchronous(UObject* WorldContext, FName MagicName);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContext"))
		static void LoadMultipleSpellsSynchronous(UObject* WorldContext, const TArray<FName>& MagicNames, TArray<TSubclassOf<AMagicUI>>& OutLoaded);

	UFUNCTION(BlueprintPure)
		static UMaterialInterface* GetMagicEmblem(TSubclassOf<AMagicUI> MagicClass);

	UFUNCTION(BlueprintPure)
		static UTexture* GetMagicEmblemTexture(TSubclassOf<AMagicUI> MagicClass);

	UFUNCTION(BlueprintPure)
		static FLinearColor GetMagicColor(TSubclassOf<AMagicUI> MagicClass);
};
