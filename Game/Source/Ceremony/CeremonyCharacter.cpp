// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CeremonyCharacter.h"

#include "PlayerManagerSubsystem.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SpatialNetDriver.h"

#include "UnrealNetwork.h"

//////////////////////////////////////////////////////////////////////////
// ACeremonyCharacter

ACeremonyCharacter::ACeremonyCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

												// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	mScene = SCENE_LOBBY;

	GetTeleportToMeetingRoom();
	GetTeleportToToLobby();

	UE_LOG(LogTemp, Warning, TEXT("Constructor of ACeremonyCharacter, name:%s, unique id:[%d]"), *GetName(), GetUniqueID());
}

ACeremonyCharacter::~ACeremonyCharacter()
{
	UE_LOG(LogTemp, Warning, TEXT("Deconstructor of ACeremonyCharacter, name:%s, unique id:[%d]"), *GetName(), GetUniqueID());
}

void ACeremonyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ACeremonyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		mInteractionSystem = GetWorld()->SpawnActor<AInteractionSystem>(AInteractionSystem::StaticClass(), GetActorLocation(), FRotator::ZeroRotator);
		if (mInteractionSystem)
		{
			mInteractionSystem->SetOwner(this);
		}
	}

	PlayerManagerSubsystem* playerManager = GetGameInstance()->GetSubsystem<PlayerManagerSubsystem>();
	// playerManager->AddCharacter(this);

	UE_LOG(LogTemp, Warning, TEXT("ACeremonyCharacter::BeginPlay, name:%s, unique id:[%d]"), *GetName(), GetUniqueID());
}


void ACeremonyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	PlayerManagerSubsystem* playerManager = GetGameInstance()->GetSubsystem<PlayerManagerSubsystem>();
	// playerManager->RemoveCharacter(this->GetId());

	UE_LOG(LogTemp, Warning, TEXT("ACeremonyCharacter::EndPlay, name:%s, unique id:[%d], reason:[%d]"), *GetName(), GetUniqueID(), EndPlayReason);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ACeremonyCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ACeremonyCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACeremonyCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ACeremonyCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ACeremonyCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ACeremonyCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ACeremonyCharacter::TouchStopped);
}

void ACeremonyCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void ACeremonyCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void ACeremonyCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ACeremonyCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ACeremonyCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ACeremonyCharacter::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

AStaticMeshActor* ACeremonyCharacter::GetTeleportToMeetingRoom()
{
	if (mTeleportToMeetingRoom)
	{
		return mTeleportToMeetingRoom;
	}

	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), "TeleportToMeetingRoom", Actors);
	for (AActor* tmpActor : Actors)
	{
		mTeleportToMeetingRoom = Cast<AStaticMeshActor>(tmpActor);
		return mTeleportToMeetingRoom;
	}

	return nullptr;
}

AStaticMeshActor* ACeremonyCharacter::GetTeleportToToLobby()
{
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), "TeleportToLobby", Actors);
	for (AActor* tmpActor : Actors)
	{
		mTeleportToLobby = Cast<AStaticMeshActor>(tmpActor);
		return mTeleportToLobby;
	}

	return nullptr;
}

void ACeremonyCharacter::TeleportToMeetingRoom()
{
	if (!HasAuthority())
	{
		return;
	}

	if (mScene == SCENE_MEETING_ROOM)
	{
		return;
	}

	if (!mTeleportToLobby)
	{
		return;
	}

	TArray<AActor*> Actors;
	mTeleportToLobby->GetAllChildActors(Actors, true);
	UE_LOG(LogTemp, Warning, TEXT("name:%s"), *mTeleportToLobby->GetName());
	for (AActor* tmpActor : Actors)
	{
		UE_LOG(LogTemp, Warning, TEXT("sub name:%s"), *tmpActor->GetName());
		
		for (auto tmpTag: tmpActor->Tags)
		{
			// UE_LOG(LogTemp, Warning, TEXT("tag name:%s"), *tmpTag.ToString());
		}
	}

	FVector loc = mTeleportToLobby->GetActorLocation();
	loc.Y += 260;
	this->SetActorLocation(loc);
	mScene = SCENE_MEETING_ROOM;
}

void ACeremonyCharacter::TeleportToLobby()
{
	if (!HasAuthority())
	{
		return;
	}

	if (mScene == SCENE_LOBBY)
	{
		return;
	}

	if (!mTeleportToMeetingRoom)
	{
		return;
	}

	FVector loc = mTeleportToMeetingRoom->GetActorLocation();
	loc.Y += 260;
	this->SetActorLocation(loc);
	mScene = SCENE_LOBBY;
}

void ACeremonyCharacter::OnIdNotify()
{
}
