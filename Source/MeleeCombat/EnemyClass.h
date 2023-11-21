
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyClass.generated.h"


UCLASS()
class MELEECOMBAT_API AEnemyClass : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyClass();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent)
	void ShowHealthBar();

	void ShowHealthBar_Implementation();

	UFUNCTION(BlueprintImplementableEvent)
	void HideHealthBar();

	void Die();

	void PlayHitMontage(FName Section, float PlayRate = 1.0f);

	void ResetHitReactTimer();


	/** Called when something overlaps with the agro sphere */
	UFUNCTION()
		void AgroSphereOverlap(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable)
	void SetStunned(bool Stunned);

	UFUNCTION()
	void CombatRangeOverlap(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult);

	UFUNCTION()
	void CombatRangeEndOverlap(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void PlayAttackMontage(FName Section, float PlayRate);

	UFUNCTION(BlueprintPure)
	FName GetAttackSectionName();
    
	UFUNCTION()
	void OnRightWeaponOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex,
		bool bFromSweep, 
		const FHitResult& SweepResult);


	UFUNCTION(BlueprintCallable)
	void ActivateRightWeapon();

	UFUNCTION(BlueprintCallable)
	void DeactivateRightWeapon();


	void DoDamage(class AMeleeCharacter* Victim);
	void SpawnBlood(AMeleeCharacter* Victim, FName SocketName);

	// Attempt to stun character
	void StunCharacter(AMeleeCharacter* Victim);

	void ResetCanAttack();

	UFUNCTION(BlueprintCallable)
	void FinishDeath();

	UFUNCTION()
	void DestroyEnemy();

private:
	/** Particles to spawn when hit by bullets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* ImpactParticles;

	/** Sound to play when hit by bullets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* ImpactSound;

	/** Current health of the enemy */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float Health;

	/** Maximum health of the enemy */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float MaxHealth;

	/** Time to display health bar once shot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HealthBarDisplayTime;

	FTimerHandle HealthBarTimer;

	/** Montage containing Hit and Death animations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HitMontage;

	FTimerHandle HitReactTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HitReactTimeMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HitReactTimeMax;

	bool bCanHitReact;

	/** Behavior tree for the AI Character */
	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true"))
	class UBehaviorTree* BehaviorTree;

	/** Point for the enemy to move to */
	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	FVector PatrolPoint;

	/** Second point for the enemy to move to */
	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	FVector PatrolPoint2;

	class AEnemyController* EnemyController;

	/** Overlap sphere for when the enemy becomes hostile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AgroSphere;

	/** True when playing the get hit animation */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bStunned;

	/** Chance of being stunned. 0: no stun chance, 1: 100% stun chance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float StunChance;

	/** True when in attack range; time to attack! */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bInAttackRange;

	/** Sphere for attack range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	USphereComponent* CombatRangeSphere;

	/** Montage containing different attacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* AttackMontage;

	/** The four attack montage section names */
	/*FName AttackLFast;
	FName AttackRFast;*/
	FName AttackL;
	FName AttackR;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* RightWeaponCollision;

	/** Base damage for enemy */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float BaseDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName RightWeaponSocket;

	/** True when Enemy can attack */
	UPROPERTY(VisibleAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bCanAttack;

	FTimerHandle AttackWaitTimer;

	/** Minimum wait time between attacks */
	UPROPERTY(EditAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float AttackWaitTime;

	/** Death anim montage for the enemy */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* DeathMontage;

	bool bDying;

	FTimerHandle DeathTimer;

	/** Time after death until Destroy */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float DeathTime;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	FORCEINLINE float GetHealth() { return Health; }
	FORCEINLINE float GetMaxHealth() { return MaxHealth; }
	FORCEINLINE UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }
};
