// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "SkeletalSpriteFlipbookComponent.h"

#include "SkeletalSpriteAnimation.h"

#include "StaticFuncLib.h"

#include "OneShotAudio.h"


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
	if (CurrOffsAnim == NewOffsAnim && !CurrOffsAnim->TriggerRegardless)
		return;

	CurrOffsAnim = NewOffsAnim;
	OffsAnimIndex = Ind;
	CurrOffsTime = 0;

	if (nullptr != CurrOffsAnim)
	{
		FOffsetAnimData& RelevantData = CurrOffsAnim->Data[OffsAnimIndex];

		if (RelevantData.EditLocation || RelevantData.EditRotation)
		{
			FVector ResL = (RelevantData.EditLocation ? OriginalRelative.GetLocation() : RelativeLocation);
			FRotator ResR = (RelevantData.EditRotation ? OriginalRelative.GetRotation().Rotator() : RelativeRotation);

			SetRelativeLocationAndRotation(ResL, ResR);
		}
	}

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

	if (RelevantData.Keys.Num() < 1)
	{
		UStaticFuncLib::Print("USkeletalSpriteFlipbookComponent::TickOffsetAnim: Offset Animation " + CurrOffsAnim->GetName() +
							  "Has no keys for bone \'" + RelevantData.BoneName.ToString() + "\'!");

		ChangeOffsetAnim(nullptr, 0);
		return;
	}

	if (!(RelevantData.EditLocation || RelevantData.EditRotation))
	{
		ChangeOffsetAnim(nullptr, 0);
		return;
	}

	float SafeAnimLength = FMath::Max(0.0001f, RelevantData.AnimLength);

	if (RelevantData.Loops)
		CurrOffsTime = FMath::Fmod(CurrOffsTime + DeltaTime, SafeAnimLength);
	else
		CurrOffsTime = FMath::Clamp(CurrOffsTime + DeltaTime, 0.0f, SafeAnimLength);


	float ratioTime = CurrOffsTime / SafeAnimLength;

	int left = RelevantData.Keys.Num() - 1;
	while (left > 0 && RelevantData.Keys[left].Time > ratioTime)
		--left;
	int right = FMath::Clamp(left + 1, 0, RelevantData.Keys.Num() - 1);


	FVector ResultLoc;
	FRotator ResultRot;

	FOffsetAnimKey& leftRef = RelevantData.Keys[left];
	FOffsetAnimKey& rightRef = RelevantData.Keys[right];
	if (left == right || leftRef.Snap)
	{

		ResultLoc = (RelevantData.EditLocation ? OriginalRelative.GetLocation() + leftRef.LocOffs : RelativeLocation);
		ResultRot = (RelevantData.EditRotation ? (OriginalRelative.GetRotation() + leftRef.RotOffs.Quaternion()).Rotator() : RelativeRotation);

	}
	else
	{
		float internalRatio = (ratioTime - leftRef.Time) / (rightRef.Time - leftRef.Time);

		ResultLoc = (RelevantData.EditLocation ? OriginalRelative.GetLocation() + FMath::Lerp(leftRef.LocOffs, rightRef.LocOffs, internalRatio) : RelativeLocation);
		ResultRot = (RelevantData.EditRotation ? (OriginalRelative.GetRotation() + FMath::Lerp(leftRef.RotOffs, rightRef.RotOffs, internalRatio).Quaternion()).Rotator() : RelativeRotation);
	}

	if (RelevantData.SetDirectly)
	{
		RelativeLocation = ResultLoc;
		RelativeRotation = ResultRot;
	}
	else
		SetRelativeLocationAndRotation(ResultLoc, ResultRot);

	if (!RelevantData.Loops && CurrOffsTime == RelevantData.AnimLength)
		ChangeOffsetAnim(nullptr, 0);

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
	auto SpawnedAud = UStaticFuncLib::PlaySound(this, GetOwner()->GetActorTransform(), sound, pitch);

	APawn* SuperOwner = Cast<APawn>(GetOwner()->GetOwner());
	if (nullptr == SuperOwner || SuperOwner->GetController() != UGameplayStatics::GetPlayerController(this, 0))
	{
		FSoundAttenuationSettings NewAtten;

		NewAtten.bAttenuateWithLPF = true;
		NewAtten.bEnableOcclusion = true;
		NewAtten.DistanceAlgorithm = EAttenuationDistanceModel::LogReverse;

		SpawnedAud->audioComp->AdjustAttenuation(NewAtten);
		SpawnedAud->audioComp->Play();

		SpawnedAud->AttachToActor(GetOwner(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, true));
	}
}
void USkeletalSpriteFlipbookComponent::ServerPlaySoundProxy_Implementation(class USoundCue* sound, float pitch) { MulticastPlaySoundProxy(sound, pitch); }
bool USkeletalSpriteFlipbookComponent::ServerPlaySoundProxy_Validate(class USoundCue* sound, float pitch) { return true; }
