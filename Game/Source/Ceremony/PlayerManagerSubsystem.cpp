// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerManagerSubsystem.h"

#include "CeremonyCharacter.h"

PlayerManagerSubsystem::PlayerManagerSubsystem()
{
}

PlayerManagerSubsystem::~PlayerManagerSubsystem()
{
}

void PlayerManagerSubsystem::AddCharacter(ACeremonyCharacter* Character)
{
	if (!Character)
	{
		return;
	}

	mCharactersMap.Add(Character->GetId(), Character);
}

void PlayerManagerSubsystem::RemoveCharacter(const FString& Id)
{
	mCharactersMap.Remove(Id);
}

ACeremonyCharacter* PlayerManagerSubsystem::GetCharacterById(const FString& Id)
{
	ACeremonyCharacter** Found = mCharactersMap.Find(Id);
	return *Found;
}
