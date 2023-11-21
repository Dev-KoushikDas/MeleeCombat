// Fill out your copyright notice in the Description page of Project Settings.
#include "EnemyClass.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "MeleeCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
// Sets default values
AEnemyClass::AEnemyClass():
	Health(100.f),
	MaxHealth(100.f),
	HealthBarDisplayTime(4.f),
	bCanHitReact(true),
	HitReactTimeMin(.5f),
	HitReactTimeMax(3.f),
	bStunned(false),
	StunChance(0.5f),
	AttackL(TEXT("AttackL")),
	AttackR(TEXT("AttackR")),
	BaseDamage(20.f),
	RightWeaponSocket(TEXT("FX_weapon_tip")),
	bCanAttack(true),
	AttackWaitTime(1.f),
	bDying(false),
	DeathTime(4.f)
{
	 // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Create the Agro Sphere
	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());

	// Create the Combat Range Sphere
	CombatRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatRange"));
	CombatRangeSphere->SetupAttachment(GetRootComponent());

	RightWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Right Weapon Box"));
	RightWeaponCollision->SetupAttachment(GetMesh(),FName("RightWeaponSocket"));

}

// Called when the game starts or when spawned
void AEnemyClass::BeginPlay()
{
Super::BeginPlay();

AgroSphere->OnComponentBeginOverlap.AddDynamic(
	this,
	&AEnemyClass::AgroSphereOverlap);

CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(
	this,
	&AEnemyClass::CombatRangeOverlap);

CombatRangeSphere->OnComponentEndOverlap.AddDynamic(
	this,
	&AEnemyClass::CombatRangeEndOverlap);

//
//RightWeaponCollision->OnComponentBeginOverlap.AddDynamic(
//this,
//&AEnemyClass::OnRightWeaponOverlap);
//
//RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
//RightWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
//RightWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
//RightWeaponCollision->SetCollisionResponseToChannel(
//	ECollisionChannel::ECC_Pawn,
//	ECollisionResponse::ECR_Overlap);


GetMesh()->SetCollisionResponseToChannel(
	ECollisionChannel::ECC_Visibility,
	ECollisionResponse::ECR_Block);
// Ignore the camera for Mesh and Capsule
GetMesh()->SetCollisionResponseToChannel(
	ECollisionChannel::ECC_Camera,
	ECollisionResponse::ECR_Ignore);
GetCapsuleComponent()->SetCollisionResponseToChannel(
	ECollisionChannel::ECC_Camera,
	ECollisionResponse::ECR_Ignore
);

EnemyController = Cast<AEnemyController>(GetController());

if (EnemyController)
{
	EnemyController->GetBlackboardComponent()->SetValueAsBool(
	FName("CanAttack"),
	true);
}

const FVector WorldPatrolPoint = UKismetMathLibrary::TransformLocation(
	GetActorTransform(),
	PatrolPoint);

const FVector WorldPatrolPoint2 = UKismetMathLibrary::TransformLocation(
	GetActorTransform(),
	PatrolPoint2);

if (EnemyController)
{
	EnemyController->GetBlackboardComponent()->SetValueAsVector(
		TEXT("PatrolPoint"),
		WorldPatrolPoint);

	EnemyController->GetBlackboardComponent()->SetValueAsVector(
		TEXT("PatrolPoint2"),
		WorldPatrolPoint2);

	EnemyController->RunBehaviorTree(BehaviorTree);
}


}




void AEnemyClass::ShowHealthBar_Implementation()
{
	GetWorldTimerManager().ClearTimer(HealthBarTimer);
	GetWorldTimerManager().SetTimer(
		HealthBarTimer,
		this,
		&AEnemyClass::HideHealthBar,
		HealthBarDisplayTime);
}




void AEnemyClass::Die()
{
	if (bDying) return;
	bDying = true;

	HideHealthBar();

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
	}

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(
		FName("Dead"),
		true
		);
		EnemyController->StopMovement();
	}
}


void AEnemyClass::PlayHitMontage(FName Section, float PlayRate)
{
	if (bCanHitReact)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(HitMontage, PlayRate);
			AnimInstance->Montage_JumpToSection(Section, HitMontage);
		}

		bCanHitReact = false;
		const float HitReactTime{ FMath::FRandRange(HitReactTimeMin, HitReactTimeMax) };
		GetWorldTimerManager().SetTimer(
			HitReactTimer,
			this,
			&AEnemyClass::ResetHitReactTimer,
			HitReactTime);
	}
}

void AEnemyClass::ResetHitReactTimer()
{
	bCanHitReact = true;
}


void AEnemyClass::AgroSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr) return;

	auto Character = Cast<AMeleeCharacter>(OtherActor);
	if (Character)
	{
		if (EnemyController)
		{
			if (EnemyController->GetBlackboardComponent())
			{
				EnemyController->GetBlackboardComponent()->SetValueAsObject(
					TEXT("Target"),
					Character);
			}
		}
	}
}

void AEnemyClass::SetStunned(bool Stunned)
{
	bStunned = Stunned;

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(
			TEXT("Stunned"),
			Stunned);
	}
}

void AEnemyClass::CombatRangeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr) return;
	auto ShooterCharacter = Cast<AMeleeCharacter>(OtherActor);
	if (ShooterCharacter)
	{
		bInAttackRange = true;
		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(
				TEXT("InAttackRange"),
				true
			);
		}
	}
}

void AEnemyClass::CombatRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == nullptr) return;
	auto ShooterCharacter = Cast<AMeleeCharacter>(OtherActor);
	if (ShooterCharacter)
	{
		bInAttackRange = false;
		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(
				TEXT("InAttackRange"),
				false
			);
		}
	}

}

void AEnemyClass::PlayAttackMontage(FName Section, float PlayRate)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage);
		AnimInstance->Montage_JumpToSection(Section, AttackMontage);
	}
	bCanAttack = false;
	GetWorldTimerManager().SetTimer(
		AttackWaitTimer,
		this,
		&AEnemyClass::ResetCanAttack,
		AttackWaitTime
	);
	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(
			FName("CanAttack"),
			false);
	}
}

FName AEnemyClass::GetAttackSectionName()
{
	FName SectionName;
	const int32 Section{ FMath::RandRange(1, 2) };
	switch (Section)
	{
	case 1:
		SectionName = AttackL;
		break;
	case 2:
		SectionName = AttackR;
		break;
	//case 3:
	//	SectionName = AttackL;
	//	break;
	//case 4:
	//	SectionName = AttackR;
	//	break;
	}
	return SectionName;
}

void AEnemyClass::DoDamage(AMeleeCharacter* Victim)
{
	if (Victim == nullptr) return;

	UGameplayStatics::ApplyDamage(
		Victim,
		BaseDamage,
		EnemyController,
		this,
		UDamageType::StaticClass()
	);

	if (Victim->GetMeleeImpactSound())
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			Victim->GetMeleeImpactSound(),
			GetActorLocation());
	}
}

void AEnemyClass::SpawnBlood(AMeleeCharacter* Victim, FName SocketName)
{
	const USkeletalMeshSocket* TipSocket{ GetMesh()->GetSocketByName(SocketName) };

	if (TipSocket)
	{/*
		const FTransform SocketTransform{TipSocket->GetSocketTransform(GetMesh()) };
		if (Victim->GetBloodParticles())
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				Victim->GetBloodParticles(),
				SocketTransform
			);
		}*/
	}
}

void AEnemyClass::StunCharacter(AMeleeCharacter* Victim)
{
	if (Victim)
	{
		const float Stun{ FMath::FRandRange(0.f, 1.f) };
		if (Stun <= Victim->GetStunChance())
		{
			Victim->Stun();
		}
	}
}

void AEnemyClass::ResetCanAttack()
{
	bCanAttack = true;
	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(
			FName("CanAttack"),
			true);
	}
}

void AEnemyClass::FinishDeath()
{
	GetMesh()->bPauseAnims = true;

	GetWorldTimerManager().SetTimer(
		DeathTimer,
		this,
		&AEnemyClass::DestroyEnemy,
		DeathTime
	);
}

void AEnemyClass::DestroyEnemy()
{
	Destroy();
}

void AEnemyClass::OnRightWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//auto Character = Cast<AMeleeCharacter>(OtherActor);
	//if (Character)
	//{
	////	DoDamage(Character);
	////	SpawnBlood(Character, RightWeaponSocket);
	////	StunCharacter(Character);
	//}
}

void AEnemyClass::ActivateRightWeapon()
{
//	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemyClass::DeactivateRightWeapon()
{
//	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called every frame
void AEnemyClass::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AEnemyClass::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

float AEnemyClass::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// Set the Target Blackboard Key to agro the Character
	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsObject(
			FName("Target"),
			DamageCauser);
	}

	if (Health - DamageAmount <= 0.f)
	{
		Health = 0.f;
		Die();
	}
	else
	{
		Health -= DamageAmount;
	}

	if (bDying) return DamageAmount;

	ShowHealthBar();

	// Determine whether bullet hit stuns
	const float Stunned = FMath::FRandRange(0.f, 1.f);
	if (Stunned <= StunChance)
	{
		// Stun the Enemy
		PlayHitMontage(FName("HitReactFront"));
		SetStunned(true);
	}

	return DamageAmount;
}
