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

		UE_LOG(LogTemp, Warning, TEXT("TELEPORT APPLIED"));

		//Using of TeleportTo function for better smoothing
		if (ShooterCharacterOwner->TeleportTo(NewLocation,
			ShooterCharacterOwner->GetActorRotation()))
			return true;
	}

	return false;

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


void UShooterCharacterMovement::SetJetpack(bool bJetpackOn)
{
	if (bJetpackOn) {
		SetMovementMode(MOVE_Falling);
		AirControl = 1;
	}
	else {
		AirControl = 0.05f;
	}
}



void UShooterCharacterMovement::SetTimeRewind(bool bTimeRewind)
{
	AShooterCharacter* ShooterCharacterOwner = Cast<AShooterCharacter>(PawnOwner);
	if (bTimeRewind) {
		SetMovementMode(MOVE_Falling);
		AirControl = 0;
	}
	else {
		SetMovementMode(MOVE_Walking);
		AirControl = 0.05f;
	}
}

void UShooterCharacterMovement::DoTimeRewind(float DeltaTime)
{
	AShooterCharacter* ShooterCharacterOwner = Cast<AShooterCharacter>(PawnOwner);
	FVector NewPosition = ShooterCharacterOwner->PopLastPositionSaved();
	FVector ActualPosition = GetActorLocation();
	if(NewPosition != GetActorLocation()){
		FRotator Orientation = ShooterCharacterOwner->GetActorRotation();
		ShooterCharacterOwner->SetActorLocationAndRotation(NewPosition,Orientation);
	}
	else
		ShooterCharacterOwner->OnTimeRewindStop();
}


//////////////////////////////////////
// UNPACKING Method

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
		// Applies the input abilities contained in the move
		ShooterCharacter->bPressedTeleport = bPressedTeleport;
		if (bPressedTeleport) {
			DoTeleport();
			ShooterCharacter->OnTeleportTriggered();
		}

		if (ShooterCharacter->bJetpackOn != bJetpackOn) {
			ShooterCharacter->bJetpackOn = bJetpackOn;
			if (bJetpackOn)
				ShooterCharacter->OnJetpackStart();
			else
				ShooterCharacter->OnJetpackStop();
		}

		if (ShooterCharacter->bPressedTimeRewind != bPressedTimeRewind) {
			if (bPressedTimeRewind)
				ShooterCharacter->OnTimeRewindStart();
			else
				ShooterCharacter->OnTimeRewindStop();

		}
	}
}


#pragma region NetworkPrediction

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
			//If teleport has been pressed and is contained in SavedMove, remove the local ability trigger
			ShooterCharacter->OnTeleportTriggered();
		}

		// Set if jetpack is on or not, delegates the rest to ShooterCharacter's Tick
		bJetpackOn = ShooterCharacter->bJetpackOn;


		// Set if time rewind is active or not, delegates the rest to ShooterCharacter's Tick
		bPressedTimeRewind = ShooterCharacter->bPressedTimeRewind;

	}
}

void FSavedMove_ShooterCharacter::PrepMoveFor(ACharacter* Character)
{
	// Method used to make the correction

	Super::PrepMoveFor(Character);
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(Character);
	if (ShooterCharacter)
	{
		
		
		UShooterCharacterMovement* ShooterCharacterMovement = 
			Cast<UShooterCharacterMovement>(ShooterCharacter->GetCharacterMovement());

		//Set teleport on and execute it for the correction
		ShooterCharacter->bPressedTeleport = bPressedTeleport;
		if (bPressedTeleport) {
			ShooterCharacterMovement->DoTeleport();
			ShooterCharacter->OnTeleportTriggered();
		}

		//Set jetpack then leave it to ShooterCharacter's Tick
		if (ShooterCharacter->bJetpackOn != bJetpackOn) {
			ShooterCharacter->bJetpackOn = bJetpackOn;
			if (bJetpackOn)
				ShooterCharacter->OnJetpackStart();
			else
				ShooterCharacter->OnJetpackStop();
		}

		//Set Time Rewind then leave it to ShooterCharacter's Tick
		if (ShooterCharacter->bPressedTimeRewind != bPressedTimeRewind) {
			if (bPressedTimeRewind)
				ShooterCharacter->OnTimeRewindStart();
			else
				ShooterCharacter->OnTimeRewindStop();
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
		UE_LOG(LogTemp, Warning, TEXT("FLAG: PRESSED JUMP"));
		Result |= FLAG_JumpPressed;
	}

	if (bWantsToCrouch)
	{
		Result |= FLAG_WantsToCrouch;
	}

	// ADDED MOVES: Used custom flags

	if (bPressedTeleport)
	{
		UE_LOG(LogTemp, Warning, TEXT("FLAG: SET TELEPORT"));
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