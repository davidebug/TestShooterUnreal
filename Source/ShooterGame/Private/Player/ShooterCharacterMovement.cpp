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

//Personalized function to manage movements (Base inputs are accelerations (Axes and Jump input))
//Replicates movements to server
void UShooterCharacterMovement::ControlledCharacterMove(const FVector& InputVector, float DeltaSeconds) {

	Super::ControlledCharacterMove(InputVector, DeltaSeconds);

	AShooterCharacter* ShooterCharacterOwner = Cast<AShooterCharacter>(PawnOwner);
	if (ShooterCharacterOwner->CheckTeleportInput()) {
	
		if (CharacterOwner->GetLocalRole() == ROLE_Authority)
		{
			DoTeleport();
		}
		else if (CharacterOwner->GetLocalRole() == ROLE_AutonomousProxy && IsNetMode(NM_Client))
		{
			ReplicateMoveToServer(DeltaSeconds, Acceleration);
		}
	}


}


//Feature 1 -- Teleport
bool UShooterCharacterMovement::DoTeleport()
{
	AShooterCharacter* ShooterCharacterOwner = Cast<AShooterCharacter>(PawnOwner);

	if (ShooterCharacterOwner) {
		
		ShooterCharacterOwner->OnTeleportDone();
		
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


//Feature 2 -- Jetpack

bool UShooterCharacterMovement::DoJetpack()
{
	AShooterCharacter* ShooterCharacterOwner = Cast<AShooterCharacter>(PawnOwner);
	if (ShooterCharacterOwner){
			Velocity.Z = ShooterCharacterOwner->JetpackVelocity;
			return true;
	}

	return false;

}



