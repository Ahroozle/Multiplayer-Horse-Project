// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PaperFlipbookComponent.h"
#include "SkeletalSpriteFlipbookComponent.generated.h"

UCLASS(abstract, BlueprintType)
class MPHORSO_API USkeletalSpriteSkinSetBase : public UObject
{
	GENERATED_BODY()

public:

	virtual int GetNumRegions(int RegionSet) { return 1; };

	virtual int GetNumRegionSets() { return 1; }

	virtual void GetSkins(TMap<FName, UPaperFlipbook*>& ToPopulate, int RegionSet) {};

	UFUNCTION(BlueprintPure)
		static int GetSkinNumRegions(TSubclassOf<USkeletalSpriteSkinSetBase> SkinSet, int RegionSet) { if (SkinSet) { return SkinSet.GetDefaultObject()->GetNumRegions(RegionSet); } return 1; }

	UFUNCTION(BlueprintPure)
		static int GetNumRegionSets(TSubclassOf<USkeletalSpriteSkinSetBase> SkinSet) { if (SkinSet) { return SkinSet.GetDefaultObject()->GetNumRegionSets(); } return 1; }

	UFUNCTION(BlueprintCallable)
		static TSubclassOf<USkeletalSpriteSkinSetBase> LoadSkinSetFromName(FName ClassName)
	{
		if (!ClassName.IsNone())
		{
			UClass* SkinClass = FindObject<UClass>(ANY_PACKAGE, *ClassName.ToString());
			if (nullptr == SkinClass)
				SkinClass = LoadObject<UClass>(NULL, *ClassName.ToString());

			if (nullptr != SkinClass && SkinClass->IsChildOf(USkeletalSpriteSkinSetBase::StaticClass()))
				return SkinClass;
		}

		return nullptr;
	}

	UFUNCTION(BlueprintPure)
		static FName GetSaveableSkinName(TSubclassOf<USkeletalSpriteSkinSetBase> InClass) { if (InClass) { return InClass->GetFName(); } return FName(); }
};

UCLASS(Blueprintable)
class MPHORSO_API USkeletalSpriteSkinSet : public USkeletalSpriteSkinSetBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Flipbook Component", meta = (ValueMin = "1.0"))
		int NumRegions = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Flipbook Component")
		TMap<FName, class UPaperFlipbook*> Skins;

	virtual int GetNumRegions(int RegionSet) override { return NumRegions; }

	virtual void GetSkins(TMap<FName, UPaperFlipbook*>& ToPopulate, int RegionSet) override;
};

USTRUCT(BlueprintType)
struct MPHORSO_API FSkinSetRegions
{
	GENERATED_USTRUCT_BODY();

	// NOTE: 0th index should always the unregioned version of the skin!
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Flipbook Component")
		TArray<UPaperFlipbook*> Variants;
};

UCLASS(Blueprintable)
class MPHORSO_API USkeletalSpriteSkinSetRegioned : public USkeletalSpriteSkinSetBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Flipbook Component")
		TArray<int> NumRegionsInSets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Flipbook Component")
		TMap<FName, FSkinSetRegions> Skins;

	virtual int GetNumRegions(int RegionSet) override { return NumRegionsInSets[FMath::Min(RegionSet, NumRegionsInSets.Num() - 1)]; }

	virtual int GetNumRegionSets() override { return NumRegionsInSets.Num(); }

	virtual void GetSkins(TMap<FName, UPaperFlipbook*>& ToPopulate, int RegionSet) override;
};

/**
 * 
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class MPHORSO_API USkeletalSpriteFlipbookComponent : public UPaperFlipbookComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skeletal Sprite Flipbook Component")
		TSubclassOf<USkeletalSpriteSkinSetBase> SkinSet;
	UPROPERTY(EditAnywhere, Category = "Skeletal Sprite Flipbook Component")
		bool DiscardSkinsetAfterUse = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Flipbook Component")
		bool UsesRegions = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skeletal Sprite Flipbook Component")
		int CurrentRegionSet = 0;
	UPROPERTY(EditAnywhere, Category = "Skeletal Sprite Flipbook Component", meta = (EditCondition = "UsesRegions"))
		FLinearColor RegionColors[10] =
	{ {1.0f, 0.843f, 0.0f}, {0.117f, 0.564f, 1.0f}, {0.196f, 0.803f, 0.196f}, {0.854f, 0.439f, 0.839f}, {0.862f, 0.078f, 0.235f},
	  {0.498f, 1.0f, 0.831f}, {1.0f, 0.505f, 0.0f}, {0.740f, 1.0f, 0.603f}, {0.780f, 0.0f, 0.541f}, {0.298f, 0.407f, 1.0f} };

	UPROPERTY(BlueprintReadWrite, Category = "Skeletal Sprite Flipbook Component")
		TMap<FName, class UPaperFlipbook*> LocalSkins;
	
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

	// Region Vars
	UPROPERTY()
		UMaterialInstanceDynamic* DynMat = nullptr;
	UPROPERTY()
		UTexture2D* RegionColorTex = nullptr;

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
		void ChangeSkin(TSubclassOf<USkeletalSpriteSkinSetBase> NewSkin, int RegionSet = 0);

	UFUNCTION(BlueprintPure)
		void GetRegionColors(TArray<FLinearColor>& Colors) { Colors.Empty(); Colors.Append(RegionColors, 10); }

	UFUNCTION(BlueprintCallable)
		void SetRegionColors(const TArray<FLinearColor>& Colors);

	UFUNCTION()
		void UpdateRegions();

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
