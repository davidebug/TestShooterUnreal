// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * Movement component meant for use with Pawns.
 */

#pragma once
#include "ShooterCharacterMovement.generated.h"

UCLASS()
class UShooterCharacterMovement : public UCharacterMovementComponent
{
	GENERATED_UCLASS_BODY()

	virtual float GetMaxSpeed() const override;

	virtual void ControlledCharacterMove(const FVector& InputVector, float DeltaSeconds) override;

public:
	////////////////////////////////////////////////////
	//New Abilities added to UCharacterMovementComponent
	virtual bool DoTeleport();

	virtual bool DoJetpack();
};
