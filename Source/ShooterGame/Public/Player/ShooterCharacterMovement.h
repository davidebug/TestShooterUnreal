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

		/* Method for unpacking the flags from a SavedMove */
		virtual void UpdateFromCompressedFlags(uint8 Flags) override;

		/* Gets the prediction data client (ShooterCharacter) */
		virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

public:

		////////////////////////////////////////////////////
		//New Abilities added to UCharacterMovementComponent

	UFUNCTION(BlueprintCallable)
		/* Sets the Jetpack values */
		void SetJetpack(bool bJetpackOn);

	UFUNCTION(BlueprintCallable)
		/* Executes the jetpack locally */
		virtual bool DoJetpack();

	UFUNCTION(BlueprintCallable)
		/* Executes the teleport locally */
		virtual bool DoTeleport();

	UFUNCTION(BlueprintCallable)
		/* Sets the TimeRewind values */
		virtual void SetTimeRewind(bool bTimeRewind);

	UFUNCTION(BlueprintCallable)
		/* Executes the time rewind locally */
		virtual void DoTimeRewind();
};


#pragma region NetworkPrediction

class FNetworkPredictionData_Client_ShooterCharacter : public FNetworkPredictionData_Client_Character {

public:

	typedef FNetworkPredictionData_Client_Character Super;

	/* Creates a new PredictionDataClient (for a Shooter Character type) */
	FNetworkPredictionData_Client_ShooterCharacter(const UShooterCharacterMovement& ClientMovement);

	/* Creates a new Saved Move */
	virtual FSavedMovePtr AllocateNewMove() override;
};




class FSavedMove_ShooterCharacter : public FSavedMove_Character{

public:

	typedef FSavedMove_Character Super;

	/* Sets and saves a new move for a possible correction */
	virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
	
	/* Clears the savedmove */
	virtual void Clear() override;
	
	/* Returns a byte with the abilities of the character */
	virtual uint8 GetCompressedFlags() const override;
	
	/* Method to check if the move can be replicated without changing behavior */
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;

	/* Uses the savedmove to make a correction */
	virtual void PrepMoveFor(ACharacter* Character) override;

	/* Variable that tells if the teleport input has been pressed */
	uint32 bPressedTeleport : 1;

	/* Variable that tells if the Jetpack is on or not */
	uint32 bJetpackOn : 1;

};

#pragma endregion
