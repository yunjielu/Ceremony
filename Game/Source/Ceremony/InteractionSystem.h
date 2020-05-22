// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractionSystem.generated.h"

enum InteractionTypeE : int
{
	InteractionType_Flower = 0,
	InteractionType_Light,
	InteractionType_Applause,
	InteractionType_Max
};

USTRUCT()
struct FInteractionInfo
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY()
	int							type;

	UPROPERTY()
	int							count;

	UPROPERTY()
	FString						peerName;
};

USTRUCT()
struct FInteractionInfoMap
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY()
	TMap<FString, FInteractionInfo>			mInteractionInfoMap;
};

UCLASS()
class CEREMONY_API AInteractionSystem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractionSystem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnSentInteractionsNotify();
	UFUNCTION()
	void OnRecvInteractionsNotify();

private:
	UPROPERTY(ReplicatedUsing = OnSentInteractionsNotify)
	TArray<FInteractionInfoMap>			mSentInteractions;
	UPROPERTY(ReplicatedUsing = OnRecvInteractionsNotify)
	TArray<FInteractionInfoMap>			mRecvInteractions;
};
