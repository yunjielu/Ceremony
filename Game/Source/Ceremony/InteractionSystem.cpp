// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionSystem.h"

#include "UnrealNetwork.h"

// Sets default values
AInteractionSystem::AInteractionSystem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);

	mSentInteractions.Reset(InteractionType_Max);
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

}

void AInteractionSystem::OnRecvInteractionsNotify()
{

}

