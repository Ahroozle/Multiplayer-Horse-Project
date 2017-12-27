// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "SkeletalSpriteAnimation.generated.h"


USTRUCT(BlueprintType)
struct FOffsetAnimKey
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Animation|Offset Animation")
		FVector LocOffs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Animation|Offset Animation")
		FRotator RotOffs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, ClampMax = 1), Category = "Skeletal Sprite Animation|Offset Animation")
		float Time;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Animation|Offset Animation")
		bool Snap;
};

USTRUCT(BlueprintType)
struct FOffsetAnimData
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Animation|Offset Animation")
		FName BoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Animation|Offset Animation")
		bool EditLocation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Animation|Offset Animation")
		bool EditRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Animation|Offset Animation")
		bool SetDirectly = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Animation|Offset Animation")
		TArray<FOffsetAnimKey> Keys;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Animation|Offset Animation")
		float AnimLength; // Time anim runs for

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Animation|Offset Animation")
		bool Loops;
};

UCLASS(Blueprintable)
class MPHORSO_API UOffsetAnimation : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Animation")
		bool TriggerRegardless;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Animation|Offset Animation")
		TArray<FOffsetAnimData> Data;
};


USTRUCT(BlueprintType)
struct FSkinAnimData
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Animation|Skin Animation")
		FName BoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Animation|Skin Animation")
		FName SkinName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Animation|Skin Animation")
		int FrameOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Animation|Skin Animation")
		bool Loops;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Animation|Skin Animation|Sound")
		class USoundCue* SoundToPlay;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Animation|Skin Animation|Sound")
		int FrameToPlaySound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Animation|Skin Animation|Sound")
		float SoundPitch = 1.0f;
};

/**
 * 
 */
UCLASS(Blueprintable)
class MPHORSO_API USkinAnimation : public UObject
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Animation")
		bool TriggerRegardless;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Animation|Skin Animation")
		TArray<FSkinAnimData> Data;
};
