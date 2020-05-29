// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionSystem.h"


#include "CeremonyCharacter.h"
#include "UnrealNetwork.h"

// Sets default values
AInteractionSystem::AInteractionSystem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);
	bAlwaysRelevant = true;

	mSentInteractions.SetNum(InteractionType_Max);
	mRecvInteractions.SetNum(InteractionType_Max);
}

// Called when the game starts or when spawned
void AInteractionSystem::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AInteractionSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AInteractionSystem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// DOREPLIFETIME_CONDITION(AInteractionSystem, Var2, COND_OwnerOnly);
	DOREPLIFETIME(AInteractionSystem, mSentInteractions);
	DOREPLIFETIME(AInteractionSystem, mRecvInteractions);
}

void AInteractionSystem::OnSentInteractionsNotify()
{
	UE_LOG(LogTemp, Warning, TEXT("OnSentInteractionsNotify"));
}

void AInteractionSystem::OnRecvInteractionsNotify()
{
	UE_LOG(LogTemp, Warning, TEXT("OnRecvInteractionsNotify"));
}

void AInteractionSystem::DoInteractions(ACeremonyCharacter* TargetCharacter, InteractionTypeE Type, int count)
{
	if (!TargetCharacter)
	{
		return;
	}

	FInteractionInfo &Info = mSentInteractions[Type].mInteractionInfoMap.FindOrAdd(TargetCharacter->GetName());
	Info.type = Type;
	Info.count += count;
	Info.peerName = TargetCharacter->GetName();
	ACeremonyCharacter *outActor = CastChecked<ACeremonyCharacter>(this->GetOwner());
	TargetCharacter->GetInteractionSystem()->ReceiveInteractions(outActor, Type, count);
}

void AInteractionSystem::DoFlower(ACeremonyCharacter* TargetCharacter, int count)
{
	DoInteractions(TargetCharacter, InteractionType_Flower, count);
}

void AInteractionSystem::DoLight(ACeremonyCharacter* TargetCharacter, int count)
{
	DoInteractions(TargetCharacter, InteractionType_Light, count);
}

void AInteractionSystem::DoApplause(ACeremonyCharacter* TargetCharacter, int count)
{
	DoInteractions(TargetCharacter, InteractionType_Applause, count);
}

void AInteractionSystem::ReceiveInteractions(ACeremonyCharacter* SourceCharacter, InteractionTypeE Type, int count)
{
	if (!SourceCharacter)
	{
		return;
	}

	FInteractionInfo &Info = mRecvInteractions[Type].mInteractionInfoMap.FindOrAdd(SourceCharacter->GetName());
	Info.type = Type;
	Info.count += count;
	Info.peerName = SourceCharacter->GetName();
}

