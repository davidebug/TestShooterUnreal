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

		virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

public:
		////////////////////////////////////////////////////
		//New Abilities added to UCharacterMovementComponent

	UFUNCTION(BlueprintCallable)
		void SetJetpack(bool bJetpackOn);

	UFUNCTION(BlueprintCallable)
		void SetTeleport(bool bTeleportInput);

		virtual bool DoJetpack();

		virtual bool DoTeleport();


#pragma region Abilities RPCs

		//Jetpack
		void execSetJetpack(bool bJetpackOn);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
		void ServerSetJetpackRPC(bool bJetpackOn);

	UFUNCTION(Client, Reliable, BlueprintCallable)
		void ClientSetJetpackRPC(bool bJetpackOn);

		//Teleport
		void execSetTeleport(bool bTeleportInput);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
		void ServerSetTeleportRPC(bool bTeleportInput);

	UFUNCTION(Client, Reliable, BlueprintCallable)
		void ClientSetTeleportRPC(bool bTeleportInput);

#pragma endregion

};


#pragma region NetworkPredictionClient

class FNetworkPredictionData_Client_ShooterCharacter : public FNetworkPredictionData_Client_Character {

public:

	typedef FNetworkPredictionData_Client_Character Super;

	FNetworkPredictionData_Client_ShooterCharacter(const UShooterCharacterMovement& ClientMovement);

	virtual FSavedMovePtr AllocateNewMove() override;
};




class FSavedMove_ShooterCharacter : public FSavedMove_Character{

public:

	typedef FSavedMove_Character Super;

	virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
	
	virtual void Clear() override;
	
	virtual uint8 GetCompressedFlags() const override;
	
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;

	virtual void PrepMoveFor(ACharacter* Character) override;

	uint32 bTeleportInput : 1;

	uint32 bJetpackOn : 1;

};

#pragma endregion
