// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameInstanceSubsystem.h"

class ACeremonyCharacter;

/**
 * 
 */
class CEREMONY_API PlayerManagerSubsystem : public UGameInstanceSubsystem
{
public:
	PlayerManagerSubsystem();
	~PlayerManagerSubsystem();

	void AddCharacter(ACeremonyCharacter* Character);
	void RemoveCharacter(const FString& Id);
	ACeremonyCharacter* GetCharacterById(const FString& Id);

private:
	TMap<FString, ACeremonyCharacter*>					mCharactersMap;
};
