// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/SceneComponent.h"
#include "SkeletalSpriteBodyComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable )
class MPHORSO_API USkeletalSpriteBodyComponent : public USceneComponent
{
	GENERATED_BODY()

	TMap<FName, class USkeletalSpriteFlipbookComponent*> Bones;

public:	
	// Sets default values for this component's properties
	USkeletalSpriteBodyComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	
	UFUNCTION(BlueprintCallable, Category = "Skeletal Sprite Animation|Body")
		void PlayAnimation(TSubclassOf<class USkinAnimation> SpriteAnim, TSubclassOf<class UOffsetAnimation> OffsAnim);

	UFUNCTION(BlueprintCallable, Category = "Skeletal Sprite Animation|Body")
		bool GetBone(FName BoneName, USkeletalSpriteFlipbookComponent*& RetrievedBone) { return (nullptr != (RetrievedBone = Bones.FindRef(BoneName))); }

	//UFUNCTION(BlueprintCallable, Category = "Skeletal Sprite Animation|Body")

	
};
