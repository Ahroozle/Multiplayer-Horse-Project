// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PaperFlipbookComponent.h"
#include "SkeletalSpriteFlipbookComponent.generated.h"

/**
 * 
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class MPHORSO_API USkeletalSpriteFlipbookComponent : public UPaperFlipbookComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Flipbook Component")
		TMap<FName, class UPaperFlipbook*> Skins;
	
	UPROPERTY()
		class USkinAnimation* CurrSkinAnim = nullptr;
	UPROPERTY()
		int SkinAnimIndex;
	UPROPERTY()
		class UOffsetAnimation* CurrOffsAnim = nullptr;
	UPROPERTY()
		int OffsAnimIndex;

	// Sound Sync Vars
	UPROPERTY()
		int PrevFrame;

	// Offs Anim Vars
	UPROPERTY()
		FTransform OriginalRelative;
	UPROPERTY()
		float CurrOffsTime;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
		void ChangeSkinAnim(class USkinAnimation* NewSkinAnim, int Ind);
	UFUNCTION()
		void ChangeOffsetAnim(class UOffsetAnimation* NewOffsAnim, int Ind);


	void TickSkinSoundSync();
	void TickOffsetAnim(float DeltaTime);


	void PlaySoundProxy(class USoundCue* sound, float pitch);
	UFUNCTION(reliable, netmulticast)
		void MulticastPlaySoundProxy(class USoundCue* sound, float pitch);
		void MulticastPlaySoundProxy_Implementation(class USoundCue* sound, float pitch);
	UFUNCTION(reliable, server, WithValidation)
		void ServerPlaySoundProxy(class USoundCue* sound, float pitch);
		void ServerPlaySoundProxy_Implementation(class USoundCue* sound, float pitch);
		bool ServerPlaySoundProxy_Validate(class USoundCue* sound, float pitch);
};
