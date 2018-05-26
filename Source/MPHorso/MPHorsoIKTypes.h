// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/SceneComponent.h"
#include "MPHorsoIKTypes.generated.h"


USTRUCT(BlueprintType)
struct FMPHorsoIKJoint
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(BlueprintReadWrite)
		USceneComponent* SceneComponent;

	UPROPERTY(BlueprintReadOnly)
		FVector Forward = FVector::ZeroVector;

};

USTRUCT(BlueprintType)
struct FMPHorsoIKChain
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(BlueprintReadOnly)
		TArray<FMPHorsoIKJoint> Joints;

	UPROPERTY()
		FTransform PreviousTargetTransform;

};

USTRUCT(BlueprintType)
struct FMPHorsoIKAlignAxes
{
	GENERATED_USTRUCT_BODY();

	// What axis of the scenecomponent should this joint align to its forward?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TEnumAsByte<EAxis::Type> ForwardAxis;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool NegateForward = false;

	// What axis of the scenecomponent should this joint use as its up vector?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TEnumAsByte<EAxis::Type> UpAxis;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool NegateUp = false;

	// What world direction should the joint use to orient its up vector?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector WorldUp;
};

// Remember to call InitChains() in the construction script of the containing actor!
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MPHORSO_API UMPHorsoIKRoot : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMPHorsoIKRoot();

protected:

	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Can this root scenecomponent be moved around by the IK calculations?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool Anchored = true;

	UPROPERTY(BlueprintReadWrite)
		TArray<FMPHorsoIKChain> Chains;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FMPHorsoIKAlignAxes DefaultAlignAxes;

	// special align axes for components, by component name.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TMap<FName, FMPHorsoIKAlignAxes> SpecialAlignAxes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		USceneComponent* Target;

	UPROPERTY()
		FVector OriginalRelativePosition;


	UFUNCTION(BlueprintCallable)
		void InitChains();

	UFUNCTION()
		void MonitorChains();

	void UpdateChain(FMPHorsoIKChain& Chain);
	
};
