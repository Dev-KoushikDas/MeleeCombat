// Fill out your copyright notice in the Description page of Project Settings.

#include "MeleeCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/BoxComponent.h"
#include "EnemyClass.h"
#include "Kismet/GameplayStatics.h"
// Sets default values
AMeleeCharacter::AMeleeCharacter():
	//base rate for turning / look up
	BaseLookUpRate(45.f),
	BaseTurnRate(45.f),

	//turn rate for aiming/ not aiming
	HipTurnRate(90.f),
	HipLookUpRate(90.f),

	MouseHipTurnRate(1.0f),
	MouseHipLookUpRate(1.0f),
	MouseAimingLookUpRate(0.2f),
	MouseAimingTurnRate(0.2f),
	bCrouching(false),
	bSprinting(false),
	BaseMovementSpeed(650.f),
	SprintMovementSpeed(1000.f),
	Health(100.f),
	MaxHealth(100.f),
	StunChance(.25f),
	bDead(false),
	BaseDamage(20)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Crate a camera boom (pulls in towards the character if there is a collision) 
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 250.f;//The camera follows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true;//rotate the arm based on the controller
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

	//CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

	//create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Follow camera"));

	//Attach camera to the socket present at the end of boom
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	//camera does not rotate relative to arm
	FollowCamera->bUsePawnControlRotation = false;

	//***//Dont rotate when the camera rotates . Let the controller only effect the camera
	bUseControllerRotationPitch = false;
	//changed from false to true cause of target offset
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	//character moves in this direction of input
	//changed from true to false cause of target offset
	GetCharacterMovement()->bOrientRotationToMovement = true;

	//at this rotation rate
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	CharacterRightWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Weapon Box"));
	CharacterRightWeaponCollision->SetupAttachment(GetMesh(), FName("FX_weapon_tip"));
}

float AMeleeCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	return 0.0f;
}

// Called when the game starts or when spawned
void AMeleeCharacter::BeginPlay()
{
	Super::BeginPlay();

	//
	CharacterRightWeaponCollision->OnComponentBeginOverlap.AddDynamic(
	this,
	&AMeleeCharacter::CharacterOnRightWeaponOverlap);

	CharacterRightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CharacterRightWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CharacterRightWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CharacterRightWeaponCollision->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Pawn,
		ECollisionResponse::ECR_Overlap);

	GetMesh()->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Visibility,
		ECollisionResponse::ECR_Block);
	// EDIT
	// Ignore the camera for Mesh and Capsule
	GetMesh()->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Camera,
		ECollisionResponse::ECR_Ignore);

	GetCapsuleComponent()->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Camera,
		ECollisionResponse::ECR_Ignore
	);
}

void AMeleeCharacter::MoveForward(float Value)
{

	//checking if inherited Variable Controller is valid 
	//and if value is 0 there is no input from user
	if ((Controller != nullptr) && (Value != 0.0f))
	{/*	
		if (bSprinting) {
			Value = 5 * Value;
		}*/
		//find out which way is forward
		const FRotator Rotation{ Controller->GetControlRotation() };

		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		//creating a matrix out of yawRotation  then getting forward X axis from the rotation matrix
		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };

		// value can be 0, -1 and 1 for direction 
		AddMovementInput(Direction, Value);

	}
}

void AMeleeCharacter::MoveRight(float Value)
{

	if ((Controller != nullptr) && (Value != 0.0f))
	{	
		/*if (bSprinting) {
			Value = 2* Value;
		}*/
		//find out which way is forward
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };


		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };
		AddMovementInput(Direction, Value);

	}
}

void AMeleeCharacter::TurnAtRate(float Rate)
{

//	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());//deg/sec * sec/frame

}

void AMeleeCharacter::LookUpRate(float Rate)
{
//	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());//deg/sec * sec/frame
}

void AMeleeCharacter::Turn(float Value)
{

	float TurnScaleFactor{};
	TurnScaleFactor = MouseHipTurnRate;
	//

	AddControllerYawInput(Value * TurnScaleFactor);
	//AddControllerYawInput(Value * BaseTurnRate * GetWorld()->GetDeltaSeconds());//deg/sec * sec/frame
}

void AMeleeCharacter::LookUp(float Value)
{
	float LookUpScaleFactor{};
	
	LookUpScaleFactor = MouseHipLookUpRate;
	
	AddControllerPitchInput(Value * LookUpScaleFactor);

}

// Called every frame
void AMeleeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
void AMeleeCharacter::PlayPrimaryAttackMontage()
{
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && PrimaryAttackMontage)
	{
		AnimInstance->Montage_Play(PrimaryAttackMontage);
		AnimInstance->Montage_JumpToSection(FName("PrimaryAttack"));
	}
}
void AMeleeCharacter::PlayPrimaryAttackSound()
{
	// Play fire sound
	//if (EquippedWeapon->GetFireSound())
	//{
	//	UGameplayStatics::PlaySound2D(this, EquippedWeapon->GetFireSound());
	//}
}
void AMeleeCharacter::FireWeapon()
{
	//if (EquippedWeapon == nullptr) return;
//	00if (CombatState != ECombatState::ECS_Unoccupied) return;

	PlayPrimaryAttackSound();
	PlayPrimaryAttackMontage();

}

void AMeleeCharacter::FireButtonPressed()
{
	FireWeapon();
}

void AMeleeCharacter::CrouchButtonPressed()
{

	if (!GetCharacterMovement()->IsFalling()) {
		bCrouching = !bCrouching;
	}
}

void AMeleeCharacter::SprintButtonPressed()
{

	if (!GetCharacterMovement()->IsFalling()) {
		bSprinting= !bSprinting;
	}
	if (bSprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintMovementSpeed;

//GetCharacterMovement()->GroundFriction = CrouchingGroundFriction;
	}

}

void AMeleeCharacter::SprintButtonReleased()
{

	if (!GetCharacterMovement()->IsFalling()) {
		bSprinting = !bSprinting;
	}
	if(!bSprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
//GetCharacterMovement()->GroundFriction = BaseGroundFriction;
	}
}

void AMeleeCharacter::LockOn()
{
	bUseControllerRotationYaw = !bUseControllerRotationYaw;
}

void AMeleeCharacter::DoDamage(AEnemyClass* Victim)
{
	if (Victim == nullptr) return;

	UGameplayStatics::ApplyDamage(
		Victim,
		BaseDamage,
		GetController(),
		this,
		UDamageType::StaticClass()
	);
        
}


void AMeleeCharacter::CharacterOnRightWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AEnemyClass* Character = Cast<AEnemyClass>(OtherActor);
	if (Character)
	{
	//	DoDamage(Character);

		UGameplayStatics::ApplyDamage(
			Character,
			BaseDamage,
			GetController(),
			this,
			UDamageType::StaticClass()
		);
	
	}

}

void AMeleeCharacter::CharacterActivateRightWeapon()
{
	CharacterRightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

}




void AMeleeCharacter::CharacterDeactivateRightWeapon()
{

	CharacterRightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}



void AMeleeCharacter::Stun()
{
	if (Health <= 0.f) return;

	/*CombatState = ECombatState::ECS_Stunned;*/

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
	}
}

// Called to bind functionality to input
void AMeleeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMeleeCharacter::MoveForward);

	PlayerInputComponent->BindAxis("MoveRight", this, &AMeleeCharacter::MoveRight);


	PlayerInputComponent->BindAxis("TurnRate", this, &AMeleeCharacter::TurnAtRate);

	PlayerInputComponent->BindAxis("LookUpRate", this, &AMeleeCharacter::LookUpRate);



	/*
	 PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput );
		PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	*/


	PlayerInputComponent->BindAxis("Turn", this, &AMeleeCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMeleeCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AMeleeCharacter::FireButtonPressed);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMeleeCharacter::SprintButtonPressed);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMeleeCharacter::SprintButtonReleased);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMeleeCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("LockOn", IE_Pressed, this, &AMeleeCharacter::LockOn);


}

