// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "SkeletalSpriteFlipbookComponent.h"

#include "SkeletalSpriteAnimation.h"

#include "StaticFuncLib.h"


void USkeletalSpriteFlipbookComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (nullptr != CurrSkinAnim)
		TickSkinSoundSync();

	if (nullptr != CurrOffsAnim)
		TickOffsetAnim(DeltaTime);
}


void USkeletalSpriteFlipbookComponent::ChangeSkinAnim(class USkinAnimation* NewSkinAnim, int Ind)
{
	if (CurrSkinAnim == NewSkinAnim && !CurrSkinAnim->TriggerRegardless)
		return;

	SkinAnimIndex = Ind;

	//UStaticFuncLib::Print((nullptr == NewSkinAnim ? "null" : NewSkinAnim->GetClass()->GetName()), true);

	if (nullptr != NewSkinAnim)
	{
		FSkinAnimData& RelevantData = NewSkinAnim->Data[SkinAnimIndex];

		UPaperFlipbook* foundSkin = Skins.FindRef(RelevantData.SkinName);

		if (nullptr != foundSkin)
		{
			SetFlipbook(foundSkin);
			SetPlaybackPositionInFrames((PrevFrame = RelevantData.FrameOffset), false);
			SetLooping(RelevantData.Loops);
			Play();

			if(PrevFrame == RelevantData.FrameToPlaySound && nullptr != RelevantData.SoundToPlay)
				PlaySoundProxy(RelevantData.SoundToPlay, RelevantData.SoundPitch);
		}
		else
			UStaticFuncLib::Print("USkeletalSpriteFlipbookComponent::ChangeSkinAnim: Couldn't find a skin named \'" + RelevantData.SkinName.ToString() + "\'!", true);
	}
	CurrSkinAnim = NewSkinAnim;
}

void USkeletalSpriteFlipbookComponent::ChangeOffsetAnim(class UOffsetAnimation* NewOffsAnim, int Ind)
{
	CurrOffsTime = 0;
	SetRelativeTransform(OriginalRelative);
	OffsAnimIndex = Ind;
	CurrOffsAnim = NewOffsAnim;
}


void USkeletalSpriteFlipbookComponent::TickSkinSoundSync()
{
	int currFrame = GetPlaybackPositionInFrames();

	FSkinAnimData& RelevantData = CurrSkinAnim->Data[SkinAnimIndex];
	if (PrevFrame != currFrame && currFrame == RelevantData.FrameToPlaySound)
	{
		PlaySoundProxy(RelevantData.SoundToPlay, RelevantData.SoundPitch);

		if (!RelevantData.Loops)
			ChangeSkinAnim(nullptr, 0);
	}

	PrevFrame = currFrame;
}

void USkeletalSpriteFlipbookComponent::TickOffsetAnim(float DeltaTime)
{
	FOffsetAnimData& RelevantData = CurrOffsAnim->Data[OffsAnimIndex];

	if (RelevantData.Loops)
		CurrOffsTime = FMath::Fmod(CurrOffsTime + DeltaTime, RelevantData.AnimLength);
	else
		CurrOffsTime = FMath::Clamp(CurrOffsTime + DeltaTime, 0.0f, RelevantData.AnimLength);


	float ratioTime = CurrOffsTime / RelevantData.AnimLength;

	int left = 0;
	while (left < RelevantData.Keys.Num() && RelevantData.Keys[left].Time > ratioTime)
		++left;
	int right = FMath::Clamp(left + 1, 0, RelevantData.Keys.Num() - 1);


	FOffsetAnimKey& leftRef = RelevantData.Keys[left];
	FOffsetAnimKey& rightRef = RelevantData.Keys[right];
	if (left == right || rightRef.Snap)
	{
		SetRelativeLocation(OriginalRelative.GetLocation() + leftRef.LocOffs);
		SetRelativeRotation(OriginalRelative.GetRotation() + leftRef.RotOffs.Quaternion());

		if (!RelevantData.Loops)
			ChangeOffsetAnim(nullptr, 0);
	}
	else
	{
		float internalRatio = (ratioTime - leftRef.Time) / (rightRef.Time - leftRef.Time);
		SetRelativeLocation(OriginalRelative.GetLocation() + FMath::Lerp(leftRef.LocOffs, rightRef.LocOffs, internalRatio));
		SetRelativeRotation(OriginalRelative.GetRotation() + FMath::Lerp(leftRef.RotOffs, rightRef.RotOffs, internalRatio).Quaternion());
	}
}

void USkeletalSpriteFlipbookComponent::PlaySoundProxy(class USoundCue* sound, float pitch)
{
	//ENetRole derp = GetOwnerRole();
	//UStaticFuncLib::Print((derp==0?"None":(derp==1?"SimProx":(derp==2?"AutoProx":(derp==3?"Auth":"INVALID")))), true);

	if (GetOwnerRole() == ROLE_Authority /*&& GEngine->GetNetMode(GetWorld()) != NM_DedicatedServer*/)
		MulticastPlaySoundProxy(sound, pitch);
}
void USkeletalSpriteFlipbookComponent::MulticastPlaySoundProxy_Implementation(class USoundCue* sound, float pitch)
{
	UStaticFuncLib::PlaySound(this, GetComponentTransform(), sound, pitch);
}
void USkeletalSpriteFlipbookComponent::ServerPlaySoundProxy_Implementation(class USoundCue* sound, float pitch) { MulticastPlaySoundProxy(sound, pitch); }
bool USkeletalSpriteFlipbookComponent::ServerPlaySoundProxy_Validate(class USoundCue* sound, float pitch) { return true; }
