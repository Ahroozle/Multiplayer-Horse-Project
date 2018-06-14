// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "MagicTypes.h"

#include "StaticFuncLib.h"
#include "MPHorsoGameInstance.h"


AMagicUI::AMagicUI(const FObjectInitializer& _init) : Super(_init)
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	RootComponent = _init.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));
}

void AMagicUI::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITOR

	for (auto &currArgFunc : ArgFuncs)
	{
		bool Found = false;
		for (UActorComponent* currComp : GetComponents())
		{
			if (currComp->GetName() == currArgFunc.Value.TemplateComponentName)
			{
				Found = true;
				break;
			}
		}

		if (!Found)
			UStaticFuncLib::Print("AMagicUI::OnConstruction: Couldn't find a component named \'" +
				currArgFunc.Value.TemplateComponentName + "\' On the actor!", true);
	}

#endif

}

void AMagicUI::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle Dummy;
	GetWorld()->GetTimerManager().SetTimer(Dummy, this, &AMagicUI::TickBillboards, 0.01f, true);
}

void AMagicUI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMagicUI::TickBillboards()
{
	UMPHorsoGameInstance* GameInst = UStaticFuncLib::RetrieveGameInstance(this);

	if (nullptr != GameInst)
	{
		UCameraComponent* RelevantCam = GameInst->GetRelevantCamera();

		if (nullptr != RelevantCam)
		{
			FVector ForwardVec = -RelevantCam->GetForwardVector();

			for (USceneComponent* currFull : FullBillboardTargets)
				currFull->SetWorldRotation(FRotationMatrix::MakeFromYZ(ForwardVec, FVector::UpVector).Rotator());

			for (USceneComponent* currPole : PoleBillboardTargets)
				currPole->SetWorldRotation(FRotationMatrix::MakeFromZY(FVector::UpVector, ForwardVec).Rotator());
		}
	}
}

void AMagicUI::CopyArg(USceneComponent* RootComp, const FArgFunc& ArgFunc)
{
	CopyHelper(RootComp, nullptr, ArgFunc);

	NewestArg->SetVisibility(true, true);

	++UniqueNum;
}

void AMagicUI::CopyHelper(USceneComponent* CurrentToCopy, USceneComponent* Parent, const FArgFunc& ArgFunc)
{
	FName DupeName = *(CurrentToCopy->GetName() + "_Inst_" + FString::FromInt(UniqueNum));
	USceneComponent* Dupe = DuplicateObject<USceneComponent>(CurrentToCopy, CurrentToCopy->GetOuter(), DupeName);

	Dupe->RegisterComponent();

	if (ArgFunc.BillboardComponents.Contains(CurrentToCopy->GetName()))
		FullBillboardTargets.Add(Dupe);
	else if (ArgFunc.PoleComponents.Contains(CurrentToCopy->GetName()))
		PoleBillboardTargets.Add(Dupe);

	if (nullptr != Parent)
		Dupe->AttachToComponent(Parent, FAttachmentTransformRules::KeepRelativeTransform);
	else
		NewestArg = Dupe;

	auto& ChildArray = CurrentToCopy->GetAttachChildren();
	for (int i = 0; i < ChildArray.Num(); ++i)
	{
		USceneComponent* currChild = ChildArray[i];
		CopyHelper(currChild, Dupe, ArgFunc);
	}
}

bool AMagicUI::RequestArg(FName ArgType)
{
	FArgFunc* Found = ArgFuncs.Find(ArgType);

	if (nullptr != Found)
	{
		USceneComponent* ArgVisualTemplate = nullptr;

		for (UActorComponent* curr : GetComponents())
		{
			if (curr->GetName() == Found->TemplateComponentName)
			{
				ArgVisualTemplate = Cast<USceneComponent>(curr);
				break;
			}
		}

		if (nullptr != ArgVisualTemplate)
		{
			UFunction* RetrievedEvent = this->FindFunction(*(Found->EventName));

			if (nullptr != RetrievedEvent)
			{
				CopyArg(ArgVisualTemplate, *Found);

				this->ProcessEvent(RetrievedEvent, nullptr);

				return true;
			}
			else
				UStaticFuncLib::Print("AMagicUI::RequestArg: Couldn't find an event named \'" + Found->EventName + "\'!", true);
		}
		else
			UStaticFuncLib::Print("AMagicUI::RequestArg: Argtype \'" + ArgType.ToString()
				+ "\' does not have a corresponding visual template!", true);
	}
	else
		UStaticFuncLib::Print("AMagicUI::RequestArg: Couldn't find an argtype named \'" + ArgType.ToString() + "\'!", true);

	return false;
}

void AMagicUI::RegisterMovingArg(USceneComponent* NewMovingArg, AActor* Target)
{
	NewMovingArg->AttachToComponent(Target->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
	MovingTargets.Add(NewMovingArg);
}

void AMagicUI::RecallMovingArgs()
{
	for (USceneComponent* curr : MovingTargets)
		curr->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
}

void AMagicUI::MarkTargets(USceneComponent* ToCopy, TArray<AActor*> Targets)
{
	FAttachmentTransformRules AttachRules =
		FAttachmentTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false);

	int ExistingMarkerIndex = 0;
	for (AActor* currTarg : Targets)
	{
		USceneComponent* ToAttach = nullptr;
		if (ExistingMarkerIndex < TargetMarkers.Num())
			ToAttach = TargetMarkers[ExistingMarkerIndex];
		else
		{
			ToAttach = MarkHelper(ToCopy);

			TargetMarkers.Add(ToAttach);
			PoleBillboardTargets.Add(ToAttach);
		}

		ToAttach->SetVisibility(true, true);

		ToAttach->AttachToComponent(currTarg->GetRootComponent(), AttachRules);

		++ExistingMarkerIndex;
	}
}

USceneComponent* AMagicUI::MarkHelper(USceneComponent* ToCopy)
{
	USceneComponent* RetComp = DuplicateObject<USceneComponent>(ToCopy, ToCopy->GetOuter());
	RetComp->RegisterComponent();

	auto& ChildArray = ToCopy->GetAttachChildren();
	for (int i = 0; i < ChildArray.Num(); ++i)
	{
		USceneComponent* currChild = ChildArray[i];
		currChild = MarkHelper(currChild);
		currChild->AttachToComponent(RetComp, FAttachmentTransformRules::KeepRelativeTransform);
	}

	return RetComp;
}

void AMagicUI::HideMarks()
{
	for (USceneComponent* curr : TargetMarkers)
		curr->SetVisibility(false, true);
}

void AMagicUI::RecallMarks()
{
	for (USceneComponent* curr : TargetMarkers)
		curr->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
}

void AMagicUI::Die()
{
	RecallMovingArgs();
	RecallMarks();

	Destroy();
}


//TArray<TSubclassOf<USpellArchetype>> UMagicActionsLibrary::SpellArchetypes;
//
//void UMagicActionsLibrary::PopulateSpellArchetypes()
//{
//	for (TObjectIterator<UClass> iter; iter; ++iter)
//	{
//		if (iter->IsChildOf(USpellArchetype::StaticClass()) &&
//			*iter != USpellArchetype::StaticClass() &&
//			!iter->GetName().StartsWith("Skel_"))
//		{
//			//UStaticFuncLib::Print(iter->GetName(), true);
//			SpellArchetypes.Add(*iter);
//		}
//	}
//}
//
//FLinearColor UMagicActionsLibrary::GetSpellColor(const FName& SpellName)
//{
//	if (SpellArchetypes.Num() < 1)
//		PopulateSpellArchetypes();
//
//	for (auto &curr : SpellArchetypes)
//	{
//		if (curr.GetDefaultObject()->SpellName == SpellName)
//			return curr.GetDefaultObject()->SpellColor;
//	}
//
//	return FLinearColor::Black;
//}
//
//UTexture* UMagicActionsLibrary::GetSpellImage(const FName& SpellName)
//{
//	if (SpellArchetypes.Num() < 1)
//		PopulateSpellArchetypes();
//
//	for (auto &curr : SpellArchetypes)
//	{
//		if (curr.GetDefaultObject()->SpellName == SpellName)
//			return curr.GetDefaultObject()->SpellImage;
//	}
//
//	return nullptr;
//}
//
//bool UMagicActionsLibrary::GetSpell(const FName& SpellName, TSubclassOf<USpellArchetype>& RetrievedSpell, TSubclassOf<USpellUI>& RetrievedUI)
//{
//	if (SpellArchetypes.Num() < 1)
//		PopulateSpellArchetypes();
//
//	for (auto &curr : SpellArchetypes)
//	{
//		if (curr.GetDefaultObject()->SpellName == SpellName)
//		{
//			RetrievedSpell = curr;
//			RetrievedUI = curr.GetDefaultObject()->UI;
//			return true;
//		}
//	}
//
//	UStaticFuncLib::Print("UMagicActionsLibrary::GetSpell: Couldn't find Spell \'" + SpellName.ToString() + "\'!\n(Note: Did you make sure to add it to the appropriate MPHorsoGameInstanceBP array?)", true);
//
//	return false;
//}
//
//void UMagicActionsLibrary::ExecuteSpell(AActor* Caster, TSubclassOf<USpellArchetype> Spell, USpellUI* Args)
//{
//	Spell.GetDefaultObject()->Use(Caster, Args);
//}

TSubclassOf<AMagicUI> UMagicActionsLibrary::LoadSpellSynchronous(UObject* WorldContext, FName MagicName)
{
	UMPHorsoGameInstance* GameInst = UStaticFuncLib::RetrieveGameInstance(WorldContext);
	
	if (nullptr != GameInst)
	{
		TAssetSubclassOf<AMagicUI> MagicClass = GameInst->AllSpells.FindRef(MagicName);

		return MagicClass.LoadSynchronous();
	}

	return nullptr;
}

void UMagicActionsLibrary::LoadMultipleSpellsSynchronous(UObject* WorldContext,
	const TArray<FName>& MagicNames, TArray<TSubclassOf<AMagicUI>>& OutLoaded)
{
	OutLoaded.Empty();

	UMPHorsoGameInstance* GameInst = UStaticFuncLib::RetrieveGameInstance(WorldContext);

	if (nullptr != GameInst)
	{
		TAssetSubclassOf<AMagicUI> MagicClass;

		for (const FName& curr : MagicNames)
		{
			MagicClass = GameInst->AllSpells.FindRef(curr);
			OutLoaded.Add(MagicClass.LoadSynchronous());
		}
	}
}

UMaterialInterface* UMagicActionsLibrary::GetMagicEmblem(TSubclassOf<AMagicUI> MagicClass)
{
	if (nullptr != MagicClass.GetDefaultObject())
		return MagicClass.GetDefaultObject()->SpellEmblem;

	return nullptr;
}

UTexture* UMagicActionsLibrary::GetMagicEmblemTexture(TSubclassOf<AMagicUI> MagicClass)
{

	if (nullptr != MagicClass.GetDefaultObject())
	{
		UMaterialInterface* EmblemMat = MagicClass.GetDefaultObject()->SpellEmblem;

		UTexture* OutTex = nullptr;

		if (nullptr != EmblemMat)
			EmblemMat->GetTextureParameterValue("SpellImage", OutTex);

		return OutTex;
	}

	return nullptr;
}

FLinearColor UMagicActionsLibrary::GetMagicColor(TSubclassOf<AMagicUI> MagicClass)
{
	if (nullptr != MagicClass.GetDefaultObject())
	{
		UMaterialInterface* EmblemMat = MagicClass.GetDefaultObject()->SpellEmblem;

		FLinearColor OutCol = FLinearColor::Black;

		if (nullptr != EmblemMat)
			EmblemMat->GetVectorParameterValue("SpellColor", OutCol);

		return OutCol;
	}

	return FLinearColor::Black;
}
