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

void UShooterCharacterMovement::SetTeleport(bool bTeleportInput)
{
	


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
	execSetJetpack(bJetpackOn);

	if (!GetOwner() || !GetPawnOwner())
		return;

	if (!GetOwner()->HasAuthority() && GetPawnOwner()->IsLocallyControlled())
	{
		
		ServerSetJetpackRPC(bJetpackOn);
	}
	else if (GetOwner()->HasAuthority() && !GetPawnOwner()->IsLocallyControlled())
	{
		
		ClientSetJetpackRPC(bJetpackOn);
	}

}

void UShooterCharacterMovement::execSetJetpack(bool bJetpackOn)
{
	AShooterCharacter* ShooterCharacterOwner = Cast<AShooterCharacter>(PawnOwner);
	if (ShooterCharacterOwner) {
		ShooterCharacterOwner->bJetpackOn = bJetpackOn;
	}
	if (bJetpackOn) {
		SetMovementMode(MOVE_Falling);
		AirControl = 1;
	}
	else {
		AirControl = 0.05f;
	}
}

//////////////////////////////////////
// UNPACKING Method

void UShooterCharacterMovement::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	// Unpack the SavedMove arrived
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(CharacterOwner);
	ShooterCharacter->bPressedTeleport = ((Flags & FSavedMove_ShooterCharacter::FLAG_Custom_0) != 0);
	ShooterCharacter->bJetpackOn = ((Flags & FSavedMove_ShooterCharacter::FLAG_Custom_1) != 0);

	if (CharacterOwner->GetLocalRole() == ROLE_Authority)
	{
		//Applies the abilities contained in the move
		const bool bPressedTeleport = ShooterCharacter->bPressedTeleport;
		if (bPressedTeleport) {
			DoTeleport();
			ShooterCharacter->OnTeleportTriggered();
		}

	}

}


#pragma region Abilities RPCs

/// JETPACK RPCs ////

void UShooterCharacterMovement::ClientSetJetpackRPC_Implementation(bool bJetpackOn)
{
	execSetJetpack(bJetpackOn);
}

bool UShooterCharacterMovement::ServerSetJetpackRPC_Validate(bool bJetpackOn)
{
	return true;
}

void UShooterCharacterMovement::ServerSetJetpackRPC_Implementation(bool bJetpackOn)
{
	execSetJetpack(bJetpackOn);
}



#pragma endregion



#pragma region NetworkPredictionClient

////////  NETWORK PREDICTION DATA CLIENT ////////

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
	return FSavedMovePtr(new FSavedMove_ShooterCharacter);
}



//////// SAVED MOVE /////////

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
			UE_LOG(LogTemp, Warning, TEXT("MOVE: SET MOVE, TELEPORT"));
			ShooterCharacter->OnTeleportTriggered();
		}

		// Set if jetpack is on or not
		bJetpackOn = ShooterCharacter->bJetpackOn;
	}
}

void FSavedMove_ShooterCharacter::PrepMoveFor(ACharacter* Character)
{
	// Method used to make the correction

	Super::PrepMoveFor(Character);
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(Character);
	if (ShooterCharacter)
	{
		//Set jetpack on then leave it to the physics sim
		ShooterCharacter->bJetpackOn = bJetpackOn;

		//Set teleport on and execute it for the correction
		ShooterCharacter->bPressedTeleport = bPressedTeleport;
		if (bPressedTeleport) {
			(Cast<UShooterCharacterMovement>(ShooterCharacter->GetCharacterMovement()))->DoTeleport();
			ShooterCharacter->OnTeleportTriggered();
		}

			
	}
}


bool FSavedMove_ShooterCharacter::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
	if (bPressedTeleport != ((FSavedMove_ShooterCharacter*)&NewMove)->bPressedTeleport)
		return false;
	if (bJetpackOn != ((FSavedMove_ShooterCharacter*)&NewMove)->bJetpackOn)
		return false;

	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}


void FSavedMove_ShooterCharacter::Clear()
{
	Super::Clear();
	bPressedTeleport = false;
	bJetpackOn = false;
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

	return Result;
}

#pragma endregion