// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ShooterTypes.h"
#include "ShooterCharacter.generated.h"

class UNiagaraSystem;
class USoundBase;
class UNiagaraComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnShooterCharacterEquipWeapon, AShooterCharacter*, AShooterWeapon* /* new */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnShooterCharacterUnEquipWeapon, AShooterCharacter*, AShooterWeapon* /* old */);

UCLASS(Abstract)
class AShooterCharacter : public ACharacter
{
	GENERATED_UCLASS_BODY()

	virtual void BeginDestroy() override;

	virtual void BeginPlay() override;

	/** spawn inventory, setup initial variables */
	virtual void PostInitializeComponents() override;

	/** Update the character. (Running, health etc). */
	virtual void Tick(float DeltaSeconds) override;

	/** cleanup inventory */
	virtual void Destroyed() override;

	/** update mesh for first person view */
	virtual void PawnClientRestart() override;

	/** [server] perform PlayerState related setup */
	virtual void PossessedBy(class AController* C) override;

	/** [client] perform PlayerState related setup */
	virtual void OnRep_PlayerState() override;

	/** [server] called to determine if we should pause replication this actor to a specific player */
	virtual bool IsReplicationPausedForConnection(const FNetViewer& ConnectionOwnerNetViewer) override;

	/** [client] called when replication is paused for this actor */
	virtual void OnReplicationPausedChanged(bool bIsReplicationPaused) override;

	/**
	* Add camera pitch to first person mesh.
	*
	*	@param	CameraLocation	Location of the Camera.
	*	@param	CameraRotation	Rotation of the Camera.
	*/
	void OnCameraUpdate(const FVector& CameraLocation, const FRotator& CameraRotation);

	/** get aim offsets */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	FRotator GetAimOffsets() const;

	/**
	* Check if pawn is enemy if given controller.
	*
	* @param	TestPC	Controller to check against.
	*/
	bool IsEnemyFor(AController* TestPC) const;

	//////////////////////////////////////////////////////////////////////////
	// Inventory

	/**
	* [server] add weapon to inventory
	*
	* @param Weapon	Weapon to add.
	*/
	void AddWeapon(class AShooterWeapon* Weapon);

	/**
	* [server] remove weapon from inventory
	*
	* @param Weapon	Weapon to remove.
	*/
	void RemoveWeapon(class AShooterWeapon* Weapon);

	/**
	* Find in inventory
	*
	* @param WeaponClass	Class of weapon to find.
	*/
	class AShooterWeapon* FindWeapon(TSubclassOf<class AShooterWeapon> WeaponClass);

	/**
	* [server + local] equips weapon from inventory
	*
	* @param Weapon	Weapon to equip
	*/
	void EquipWeapon(class AShooterWeapon* Weapon);

	//////////////////////////////////////////////////////////////////////////
	// Weapon usage

	/** [local] starts weapon fire */
	void StartWeaponFire();

	/** [local] stops weapon fire */
	void StopWeaponFire();

	/** check if pawn can fire weapon */
	bool CanFire() const;

	/** check if pawn can reload weapon */
	bool CanReload() const;

	/** [server + local] change targeting state */
	void SetTargeting(bool bNewTargeting);

	//////////////////////////////////////////////////////////////////////////
	// Movement

	/** [server + local] change running state */
	void SetRunning(bool bNewRunning, bool bToggle);

	//////////////////////////////////////////////////////////////////////////
	// Animations

	/** play anim montage */
	virtual float PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None) override;

	/** stop playing montage */
	virtual void StopAnimMontage(class UAnimMontage* AnimMontage) override;

	/** stop playing all montages */
	void StopAllAnimMontages();

	//////////////////////////////////////////////////////////////////////////
	// Input handlers

	/** setup pawn specific input handlers */
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	/**
	* Handle analog trigger for firing
	*
	* @param Val trigger input to apply
	*/
	void FireTrigger(float Val);

	/**
	* Move forward/back
	*
	* @param Val Movment input to apply
	*/
	void MoveForward(float Val);

	/**
	* Strafe right/left
	*
	* @param Val Movment input to apply
	*/
	void MoveRight(float Val);

	/**
	* Move Up/Down in allowed movement modes.
	*
	* @param Val Movment input to apply
	*/
	void MoveUp(float Val);

	/* Frame rate independent turn */
	void TurnAtRate(float Val);

	/* Frame rate independent lookup */
	void LookUpAtRate(float Val);

	/** player pressed start fire action */
	void OnStartFire();

	/** player released start fire action */
	void OnStopFire();

	/** player pressed targeting action */
	void OnStartTargeting();

	/** player released targeting action */
	void OnStopTargeting();

	/** player pressed next weapon action */
	void OnNextWeapon();

	/** player pressed prev weapon action */
	void OnPrevWeapon();

	/** player pressed reload action */
	void OnReload();

	/** player pressed jump action */
	void OnStartJump();

	/** player released jump action */
	void OnStopJump();

	/** player pressed run action */
	void OnStartRunning();

	/** player pressed toggled run action */
	void OnStartRunningToggle();

	/** player released run action */
	void OnStopRunning();

	/////////////////////////////////////////////////////////////////////////
	// ADDED ABILITIES

	/** Checks jump input and adds jetpack implementation */
	virtual void CheckJumpInput(float DeltaTime) override;

	/** Checks teleport input */
	bool CheckTeleportInput() const;

	/** Applies the teleport when Teleport key is pressed */
	void OnTeleportPressed();

	/** Sets the teleport as Done */
	void OnTeleportDone();

	/** Handles the Jetpack and middle air movement when double jump is triggered */
	void OnJetpackChange(bool newVal);

	/** Starts Jetpacks with sound and fx */
	void StartJetpack();


	/** Stops jetpack with sound and fx */
	void StopJetpack();

	/** Checks if there is enough energy for jetpacking */
	bool CanJetpack();

	/** Time Rewind ability start */
	void OnTimeRewindStart();

	/** Time Rewind ability is stopped */
	void OnTimeRewindStop();

	/** Updates saved positions */
	void UpdateSavedPositions();

	/** Gets the last position added in the array */
	FVector PopLastPositionSaved();

	/** Updates abilities cooldowns per second */
	void UpdateAbilitiesCooldowns(float DeltaSeconds);

	/** Updates abilities cooldowns per second */
	void StartTeleportCooldown();

	/** Updates abilities cooldowns per second */
	void StartTimeRewindCooldown();

	/** Updates abilities cooldowns per second */
	bool CanTeleport();

	/** Updates abilities cooldowns per second */
	bool CanTimeRewind();

	/** Sets time rewind ability ON or OFF */
	void SetTimeRewind(bool timeRewind);

	/** Updates Jetpack Sound */
	void UpdateJetpackSound();

	/** Init Jetpack FX */
	void ResetJetpackFXComponent();

	/** Hides player's Mesh in game */
	void HidePlayerInGame();

	/** Shows player's Mesh in game */
	void ShowPlayerInGame();



	//////////////////////////////////////////////////////////////////////////
	// Reading data

	/** get mesh component */
	USkeletalMeshComponent* GetPawnMesh() const;

	/** get currently equipped weapon */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	class AShooterWeapon* GetWeapon() const;

	/** Global notification when a character equips a weapon. Needed for replication graph. */
	SHOOTERGAME_API static FOnShooterCharacterEquipWeapon NotifyEquipWeapon;

	/** Global notification when a character un-equips a weapon. Needed for replication graph. */
	SHOOTERGAME_API static FOnShooterCharacterUnEquipWeapon NotifyUnEquipWeapon;

	/** get weapon attach point */
	FName GetWeaponAttachPoint() const;

	/** get total number of inventory items */
	int32 GetInventoryCount() const;

	/**
	* get weapon from inventory at index. Index validity is not checked.
	*
	* @param Index Inventory index
	*/
	class AShooterWeapon* GetInventoryWeapon(int32 index) const;

	/** get weapon taget modifier speed	*/
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	float GetTargetingSpeedModifier() const;

	/** get targeting state */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	bool IsTargeting() const;

	/** get firing state */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	bool IsFiring() const;

	/** get the modifier value for running speed */
	UFUNCTION(BlueprintCallable, Category = Pawn)
	float GetRunningSpeedModifier() const;

	/** get running state */
	UFUNCTION(BlueprintCallable, Category = Pawn)
	bool IsRunning() const;


	/** Tells if time is rewinding or not */
	UFUNCTION(BlueprintCallable, Category = Pawn)
	bool IsTimeRewinding() const;

	/** get camera view type */
	UFUNCTION(BlueprintCallable, Category = Mesh)
	virtual bool IsFirstPerson() const;

	/** get max health */
	int32 GetMaxHealth() const;

	/** check if pawn is still alive */
	bool IsAlive() const;

	/** returns percentage of health when low health effects should start */
	float GetLowHealthPercentage() const;

	/*
	* Get either first or third person mesh.
	*
	* @param	WantFirstPerson		If true returns the first peron mesh, else returns the third
	*/
	USkeletalMeshComponent* GetSpecifcPawnMesh(bool WantFirstPerson) const;

	/** Update the team color of all player meshes. */
	void UpdateTeamColorsAllMIDs();
private:

	/** pawn mesh: 1st person view */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* Mesh1P;
protected:

	/** socket or bone name for attaching weapon mesh */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	FName WeaponAttachPoint;

	/** default inventory list */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	TArray<TSubclassOf<class AShooterWeapon> > DefaultInventoryClasses;

	/** weapons in inventory */
	UPROPERTY(Transient, Replicated)
	TArray<class AShooterWeapon*> Inventory;

	/** currently equipped weapon */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentWeapon)
	class AShooterWeapon* CurrentWeapon;

	/** Replicate where this pawn was last hit and damaged */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_LastTakeHitInfo)
	struct FTakeHitInfo LastTakeHitInfo;

	/** Time at which point the last take hit info for the actor times out and won't be replicated; Used to stop join-in-progress effects all over the screen */
	float LastTakeHitTimeTimeout;

	/** modifier for max movement speed */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	float TargetingSpeedModifier;

	/** current targeting state */
	UPROPERTY(Transient, Replicated)
	uint8 bIsTargeting : 1;

	/** modifier for max movement speed */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	float RunningSpeedModifier;

	/** current running state */
	UPROPERTY(Transient, Replicated)
	uint8 bWantsToRun : 1;

	/** from gamepad running is toggled */
	uint8 bWantsToRunToggled : 1;

	/** current firing state */
	uint8 bWantsToFire : 1;

	/** Current number of positions not saved in the SavedPositions Array */
	int NotSavedPositions;

	/** when low health effects should start */
	float LowHealthPercentage;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	float BaseTurnRate;

	/** Base lookup rate, in deg/sec. Other scaling may affect final lookup rate. */
	float BaseLookUpRate;

	/** material instances for setting team color in mesh (3rd person view) */
	UPROPERTY(Transient)
	TArray<UMaterialInstanceDynamic*> MeshMIDs;

	/** animation played on death */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* DeathAnim;

	/** sound played on death, local player only */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	USoundCue* DeathSound;

	/** effect played on respawn */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	UParticleSystem* RespawnFX;

	/** sound played on respawn */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	USoundCue* RespawnSound;

	/** sound played when health is low */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	USoundCue* LowHealthSound;

	/** sound played when running */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	USoundCue* RunLoopSound;

	/** sound played when stop running */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	USoundCue* RunStopSound;

	/** sound played when targeting state changes */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	USoundCue* TargetingSound;

	/** used to manipulate with run loop sound */
	UPROPERTY()
	UAudioComponent* RunLoopAC;

	/** hook to looped low health sound used to stop/adjust volume */
	UPROPERTY()
	UAudioComponent* LowHealthWarningPlayer;

	/** handles sounds for running */
	void UpdateRunSounds();

	/** handle mesh visibility and updates */
	void UpdatePawnMeshes();

	/** handle mesh colors on specified material instance */
	void UpdateTeamColors(UMaterialInstanceDynamic* UseMID);

	/** Responsible for cleaning up bodies on clients. */
	virtual void TornOff();

private:

	/** Whether or not the character is moving (based on movement input). */
	bool IsMoving();

	//////////////////////////////////////////////////////////////////////////
	// Damage & death

public:

	/** Identifies if pawn is in its dying state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Health)
	uint32 bIsDying : 1;

	// Current health of the Pawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Health)
	float Health;

	///////////////////////////////////////////////////////////
	// Useful variables for added abilities

	/** Tells if teleport has been triggered or not */
	UPROPERTY(BlueprintReadOnly, Category = Character)
		uint32 bPressedTeleport : 1;

	/** Tells if Time Rewind has been triggered and is active or not */
	UPROPERTY(BlueprintReadOnly, Category = Character)
		uint32 bPressedTimeRewind : 1;

	/** Jetpack current energy pool */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
		int JetpackCurrentEnergy;

	/** Jetpack maximum energy pool */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
		int JetpackMaxEnergy;

	/** Tells if Jetpack is active or not */
	UPROPERTY(BlueprintReadOnly, Category = Character)
		uint32 bJetpackOn : 1;

	/** Jetpack velocity*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
		float JetpackVelocity;

	/** Teleport cooldown*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
		float TeleportCooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
		float CurrentTeleportCooldown;

	/** Time rewind cooldown*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
		float TimeRewindCooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
		float CurrentTimeRewindCooldown;

	/** Saved old positions of the character used for time rewinding */
	UPROPERTY(BlueprintReadOnly, Category = Character)
		TArray<FVector> SavedPositionsArray;

	/** Maximum number of saved old positions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
		int MaxPositionsSaved;

	/** Interval of frames in which the character saves his positions in SavedPositionsArray
	The Higher the interval is, the faster will be the speed of Time Rewind */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
		int SavedPositionsInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
		UNiagaraSystem* NS_JetpackFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
		UNiagaraComponent* NC_JetpackFXComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
		USoundBase* SB_JetpackSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
		UNiagaraSystem* NS_AbilityEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
		USoundBase* SB_TeleportSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
		USoundBase* SB_TimeRewindSound;



	////////////////////////////////////////////////////////////
	/** Take damage, handle death */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	/** Pawn suicide */
	virtual void Suicide();

	/** Kill this pawn */
	virtual void KilledBy(class APawn* EventInstigator);

	/** Returns True if the pawn can die in the current state */
	virtual bool CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser) const;

	/**
	* Kills pawn.  Server/authority only.
	* @param KillingDamage - Damage amount of the killing blow
	* @param DamageEvent - Damage event of the killing blow
	* @param Killer - Who killed this pawn
	* @param DamageCauser - the Actor that directly caused the damage (i.e. the Projectile that exploded, the Weapon that fired, etc)
	* @returns true if allowed
	*/
	virtual bool Die(float KillingDamage, struct FDamageEvent const& DamageEvent, class AController* Killer, class AActor* DamageCauser);

	// Die when we fall out of the world.
	virtual void FellOutOfWorld(const class UDamageType& dmgType) override;

	/** Called on the actor right before replication occurs */
	virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker) override;



protected:
	/** notification when killed, for both the server and client. */
	virtual void OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser);

	/** play effects on hit */
	virtual void PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser);

	/** switch to ragdoll */
	void SetRagdollPhysics();

	/** sets up the replication for taking a hit */
	void ReplicateHit(float Damage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser, bool bKilled);

	/** play hit or death on client */
	UFUNCTION()
	void OnRep_LastTakeHitInfo();

	//////////////////////////////////////////////////////////////////////////
	// Inventory

	/** updates current weapon */
	void SetCurrentWeapon(class AShooterWeapon* NewWeapon, class AShooterWeapon* LastWeapon = NULL);

	/** current weapon rep handler */
	UFUNCTION()
	void OnRep_CurrentWeapon(class AShooterWeapon* LastWeapon);

	/** [server] spawns default inventory */
	void SpawnDefaultInventory();

	/** [server] remove all weapons from inventory and destroy them */
	void DestroyInventory();

	/** equip weapon */
	UFUNCTION(reliable, server, WithValidation)
	void ServerEquipWeapon(class AShooterWeapon* NewWeapon);

	/** update targeting state */
	UFUNCTION(reliable, server, WithValidation)
	void ServerSetTargeting(bool bNewTargeting);

	/** update targeting state */
	UFUNCTION(reliable, server, WithValidation)
	void ServerSetRunning(bool bNewRunning, bool bToggle);

	/** Builds list of points to check for pausing replication for a connection*/
	void BuildPauseReplicationCheckPoints(TArray<FVector>& RelevancyCheckPoints);




protected:

	/** Returns Mesh1P subobject **/
	FORCEINLINE USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
};


