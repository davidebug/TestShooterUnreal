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

//Overrided function for added inputs checking
void UShooterCharacterMovement::ControlledCharacterMove(const FVector& InputVector, float DeltaSeconds) {

	AShooterCharacter* ShooterCharacterOwner = Cast<AShooterCharacter>(PawnOwner);
	ShooterCharacterOwner->CheckTeleportInput();

	Super::ControlledCharacterMove(InputVector, DeltaSeconds);



}


///////////////////////////////////////////
// Teleport Implementation

bool UShooterCharacterMovement::DoTeleport()
{
	AShooterCharacter* ShooterCharacterOwner = Cast<AShooterCharacter>(PawnOwner);

	if (ShooterCharacterOwner) {

		ShooterCharacterOwner->OnTeleportDone();

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
	execSetTeleport(bTeleportInput);

	if (!GetOwner() || !GetPawnOwner())
		return;

	if (!GetOwner()->HasAuthority() && GetPawnOwner()->IsLocallyControlled())
	{
		ServerSetTeleportRPC(bTeleportInput);
	}
	else if (GetOwner()->HasAuthority() && !GetPawnOwner()->IsLocallyControlled())
	{
		ClientSetTeleportRPC(bTeleportInput);
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



#pragma region Abilities RPCs

/// JETPACK RPCs ////
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


/// TELEPORT RPCs ///
void UShooterCharacterMovement::execSetTeleport(bool bTeleportInput)
{
	AShooterCharacter* ShooterCharacterOwner = Cast<AShooterCharacter>(PawnOwner);
	if (ShooterCharacterOwner) {
		ShooterCharacterOwner->bPressedTeleport = bTeleportInput;
	}
}

void UShooterCharacterMovement::ClientSetTeleportRPC_Implementation(bool bTeleportInput)
{
	execSetTeleport(bTeleportInput);
}

bool UShooterCharacterMovement::ServerSetTeleportRPC_Validate(bool bTeleportInput)
{
	return true;
}

void UShooterCharacterMovement::ServerSetTeleportRPC_Implementation(bool bTeleportInput)
{
	execSetTeleport(bTeleportInput);
}


#pragma endregion




#pragma region NetworkPredictionClient

////////  NETWORK PREDICTION DATA CLIENT ////////


FNetworkPredictionData_Client* UShooterCharacterMovement::GetPredictionData_Client() const
{
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


void FSavedMove_ShooterCharacter::Clear()
{
	UE_LOG(LogTemp, Warning, TEXT("CLEAR MOVE"));
	Super::Clear();
	bTeleportInput = false;
	bJetpackOn = false;
}

uint8 FSavedMove_ShooterCharacter::GetCompressedFlags() const
{
	return Super::GetCompressedFlags();
}

bool FSavedMove_ShooterCharacter::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
	if (bTeleportInput != ((FSavedMove_ShooterCharacter*)&NewMove)->bTeleportInput)
		return false;
	if (bJetpackOn != ((FSavedMove_ShooterCharacter*)&NewMove)->bJetpackOn)
		return false;

	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void FSavedMove_ShooterCharacter::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(Character);
	if (ShooterCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Move Created"));
		bTeleportInput = ShooterCharacter->bPressedTeleport;
		bJetpackOn = ShooterCharacter->bJetpackOn;
	}
}

void FSavedMove_ShooterCharacter::PrepMoveFor(ACharacter* Character)
{

	Super::PrepMoveFor(Character);
	UShooterCharacterMovement* CharacterMovement = Cast<UShooterCharacterMovement>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		CharacterMovement->execSetJetpack(bJetpackOn);
		CharacterMovement->execSetTeleport(bTeleportInput);
	}
}

#pragma endregion