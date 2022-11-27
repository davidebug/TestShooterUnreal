// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Player/ShooterCharacterMovement.h"

DECLARE_CYCLE_STAT(TEXT("Char Update Acceleration"), STAT_CharUpdateAcceleration, STATGROUP_Character);


//----------------------------------------------------------------------//
// UPawnMovementComponent
//----------------------------------------------------------------------//
UShooterCharacterMovement::UShooterCharacterMovement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


float UShooterCharacterMovement::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();

	const AShooterCharacter* ShooterCharacterOwner = Cast<AShooterCharacter>(PawnOwner);
	if (ShooterCharacterOwner)
	{
		if (ShooterCharacterOwner->IsTargeting())
		{
			MaxSpeed *= ShooterCharacterOwner->GetTargetingSpeedModifier();
		}
		if (ShooterCharacterOwner->IsRunning())
		{
			MaxSpeed *= ShooterCharacterOwner->GetRunningSpeedModifier();
		}
	}

	return MaxSpeed;
}

#pragma region NewAbilitiesImplementation

///////////////////////////////////////////
// Teleport Implementation

bool UShooterCharacterMovement::DoTeleport()
{
	AShooterCharacter* ShooterCharacterOwner = Cast<AShooterCharacter>(PawnOwner);

	if (ShooterCharacterOwner) {

		//Teleport at fixed distance
		FVector NewLocation = ShooterCharacterOwner->GetActorLocation();
		FVector Distance = ShooterCharacterOwner->GetActorForwardVector() * 1000;
		NewLocation += Distance;

		//Using of TeleportTo function for better smoothing
		if (ShooterCharacterOwner->TeleportTo(NewLocation,
			ShooterCharacterOwner->GetActorRotation()))
			return true;
	}

	return false;

}

void UShooterCharacterMovement::execSetTeleport(bool bTeleportInput)
{
	AShooterCharacter* ShooterCharacterOwner = Cast<AShooterCharacter>(PawnOwner);
	if (ShooterCharacterOwner) {
		ShooterCharacterOwner->bPressedTeleport = bTeleportInput;
		if (bTeleportInput) {
			DoTeleport();

			if (ShooterCharacterOwner->HasAuthority())
				ShooterCharacterOwner->OnTeleportDone();

		}

	}
}


///////////////////////////////////////////
// Jetpack Implementation

bool UShooterCharacterMovement::DoJetpack()
{
	AShooterCharacter* ShooterCharacterOwner = Cast<AShooterCharacter>(PawnOwner);

	if (ShooterCharacterOwner) {
		Velocity.Z = ShooterCharacterOwner->JetpackVelocity;
		return true;
	}

	return false;

}

void UShooterCharacterMovement::execSetJetpack(bool bJetpackOn)
{
	if (bJetpackOn) {
		SetMovementMode(MOVE_Falling);
		AirControl = 1;
	}
	else {
		AirControl = 0.05f;
	}

	AShooterCharacter* ShooterCharacterOwner = Cast<AShooterCharacter>(PawnOwner);
	if (ShooterCharacterOwner) {
		if (bJetpackOn)
			ShooterCharacterOwner->StartJetpack();
		else {
			ShooterCharacterOwner->StopJetpack();
		}
	}

}

///////////////////////////////////////////
// TimeRewind Implementation

void UShooterCharacterMovement::DoTimeRewind(float DeltaTime)
{
	AShooterCharacter* ShooterCharacterOwner = Cast<AShooterCharacter>(PawnOwner);
	FVector NewPosition = ShooterCharacterOwner->PopLastPositionSaved();
	FVector EndingPosition = FVector(0, 0, 0);

	if (NewPosition != EndingPosition) {
		FRotator Orientation = ShooterCharacterOwner->GetActorRotation();
		ShooterCharacterOwner->SetActorLocationAndRotation(NewPosition, Orientation);
	}
	else
		ShooterCharacterOwner->OnTimeRewindStop();
}

void UShooterCharacterMovement::execSetTimeRewind(bool bTimeRewind)
{
	if (bTimeRewind) {
		SetMovementMode(MOVE_Falling);
		AirControl = 0;
	}
	else {
		SetMovementMode(MOVE_Walking);
		AirControl = 0.05f;
	}

	AShooterCharacter* ShooterCharacterOwner = Cast<AShooterCharacter>(PawnOwner);
	if (ShooterCharacterOwner) {
		ShooterCharacterOwner->SetTimeRewind(bTimeRewind);
	}
}


#pragma endregion

#pragma region Abilities RPCs(old)

///// First implementation of networking through RPCs

/// JETPACK RPCs ////

bool UShooterCharacterMovement::ServerSetJetpackRPC_Validate(bool bJetpackOn)
{
	return true;
}

void UShooterCharacterMovement::ServerSetJetpackRPC_Implementation(bool bJetpackOn)
{
	execSetJetpack(bJetpackOn);
}

/// TELEPORT RPCs ///


bool UShooterCharacterMovement::ServerSetTeleportRPC_Validate(bool bTeleportInput)
{
	return true;
}

void UShooterCharacterMovement::ServerSetTeleportRPC_Implementation(bool bTeleportInput)
{
	execSetTeleport(bTeleportInput);
}

/// TIMEREWIND RPCs ///

bool UShooterCharacterMovement::ServerSetTimeRewindRPC_Validate(bool bTimeRewind)
{
	return true;
}

void UShooterCharacterMovement::ServerSetTimeRewindRPC_Implementation(bool bTimeRewind)
{
	execSetTimeRewind(bTimeRewind);
}



#pragma endregion


#pragma region NetworkPrediction


//////////////////////////////////////////
// UNPACKING Method
// NEW IMPLEMENTATION USING ONLY COMPRESSED FLAGS UNPACKING

void UShooterCharacterMovement::UpdateFromCompressedFlags(uint8 Flags)
{

	Super::UpdateFromCompressedFlags(Flags);

	// Unpack the SavedMove arrived
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(CharacterOwner);

	const bool bPressedTeleport = ((Flags & FSavedMove_ShooterCharacter::FLAG_Custom_0) != 0);

	// Jetpack is handled per Tick so is set the variable only
	const bool bJetpackOn = ((Flags & FSavedMove_ShooterCharacter::FLAG_Custom_1) != 0);

	// TimeRewind is handled per Tick so is set the variable only
	const bool bPressedTimeRewind = ((Flags & FSavedMove_ShooterCharacter::FLAG_Custom_2) != 0);

	if (CharacterOwner->GetLocalRole() == ROLE_Authority)
	{
		if (ShooterCharacter->bPressedTeleport != bPressedTeleport) {
			execSetTeleport(bPressedTeleport);
		}

		if (ShooterCharacter->bJetpackOn != bJetpackOn) {
			execSetJetpack(bJetpackOn);
		}

		if (ShooterCharacter->bPressedTimeRewind != bPressedTimeRewind) {
			execSetTimeRewind(bPressedTimeRewind);
		}
	}
}


/////////////////////////////////////////
//  NetworkPredictionData_Client Methods

FNetworkPredictionData_Client* UShooterCharacterMovement::GetPredictionData_Client() const
{

	checkSlow(CharacterOwner != NULL);

	// This method has been changed to return a ShooterCharacter Client
	if (ClientPredictionData == nullptr)
	{
		UShooterCharacterMovement* MutableThis = const_cast<UShooterCharacterMovement*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_ShooterCharacter(*this);
	}

	return ClientPredictionData;
}


FNetworkPredictionData_Client_ShooterCharacter::FNetworkPredictionData_Client_ShooterCharacter(const UShooterCharacterMovement& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr FNetworkPredictionData_Client_ShooterCharacter::AllocateNewMove()
{
	// New ShooterCharacter Move
	return FSavedMovePtr(new FSavedMove_ShooterCharacter);
}

////////////////////////////////////////
// SavedMove Methods

void FSavedMove_ShooterCharacter::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	// Method used to set a new move

	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(Character);
	if (ShooterCharacter)
	{
		// Set if teleport is pressed or not
		bPressedTeleport = ShooterCharacter->bPressedTeleport;

		if (bPressedTeleport) {
			//If teleport has been pressed and is saved in SavedMove, remove the local ability trigger
			ShooterCharacter->OnTeleportDone();
		}

		// Set if jetpack is on or not, delegates the rest to ShooterCharacter's Tick
		bJetpackOn = ShooterCharacter->bJetpackOn;


		// Set if time rewind is active or not, delegates the rest to ShooterCharacter's Tick
		bPressedTimeRewind = ShooterCharacter->bPressedTimeRewind;

	}
}

void FSavedMove_ShooterCharacter::PrepMoveFor(ACharacter* Character)
{
	// Move used to make the correction
	Super::PrepMoveFor(Character);

	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(Character);
	UShooterCharacterMovement* ShooterCharacterMovement = Cast<UShooterCharacterMovement>(Character->GetCharacterMovement());
	if (ShooterCharacter)
	{
		if (ShooterCharacter->bPressedTeleport != bPressedTeleport) {
			ShooterCharacterMovement->execSetTeleport(bPressedTeleport);
		}

		if (ShooterCharacter->bJetpackOn != bJetpackOn) {
			ShooterCharacterMovement->execSetJetpack(bJetpackOn);
		}


		if (ShooterCharacter->bPressedTimeRewind != bPressedTimeRewind) {
			ShooterCharacterMovement->execSetTimeRewind(bPressedTimeRewind);
		}
	}
}


bool FSavedMove_ShooterCharacter::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
	if (bPressedTeleport != ((FSavedMove_ShooterCharacter*)&NewMove)->bPressedTeleport)
		return false;
	if (bJetpackOn != ((FSavedMove_ShooterCharacter*)&NewMove)->bJetpackOn)
		return false;
	if (bPressedTimeRewind != ((FSavedMove_ShooterCharacter*)&NewMove)->bPressedTimeRewind)
		return false;

	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}


void FSavedMove_ShooterCharacter::Clear()
{
	Super::Clear();
	bPressedTeleport = false;
	bJetpackOn = false;
	bPressedTimeRewind = false;
}

uint8 FSavedMove_ShooterCharacter::GetCompressedFlags() const
{
	// Method that incapsulates all the moves

	uint8 Result = 0;

	if (bPressedJump)
	{
		Result |= FLAG_JumpPressed;
	}

	if (bWantsToCrouch)
	{
		Result |= FLAG_WantsToCrouch;
	}

	// ADDED MOVES: Used custom flags

	if (bPressedTeleport)
	{
		Result |= FLAG_Custom_0;
	}
	if (bJetpackOn)
	{
		Result |= FLAG_Custom_1;
	}
	if (bPressedTimeRewind)
	{
		Result |= FLAG_Custom_2;
	}

	return Result;
}

#pragma endregion