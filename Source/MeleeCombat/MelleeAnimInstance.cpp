// Fill out your copyright notice in the Description page of Project Settings.


#include "MelleeAnimInstance.h"
#include "MeleeCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UMelleeAnimInstance ::UMelleeAnimInstance():
	Speed(0.f),
	bIsInAir(false),
	bIsAccelerating(false),
	MovementOffsetYaw(0.f),
	LastMovementOffsetYaw(0.f),
	CharacterRotation(FRotator(0.f)),
	CharacterRotationLastFrame(FRotator(0.f)),
	YawDelta(0.f)
{}

void UMelleeAnimInstance::UpdateAnimationProperties(float DeltaTime) {

	if (MeleeCharacter == nullptr) {

		MeleeCharacter = Cast<AMeleeCharacter>(TryGetPawnOwner());

	}
	if (MeleeCharacter) {
		// get the speed of the character from velocity

		bCrouching = MeleeCharacter->GetCrouching();
		bSprinting = MeleeCharacter->GetSprinting();

		FVector Velocity{ MeleeCharacter->GetVelocity() };
		Velocity.Z = 0;
		Speed = Velocity.Size();
		

		//is the character in the air
		bIsInAir = MeleeCharacter->GetCharacterMovement()->IsFalling();

		// is the character accelerating?
		if (MeleeCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f)
		{
		bIsAccelerating = true;
		}
		else
		{
		bIsAccelerating = false;
		}


		FRotator AimRotation = MeleeCharacter->GetBaseAimRotation();

		FRotator MovementRotation =
			UKismetMathLibrary::MakeRotFromX(
				MeleeCharacter->GetVelocity());

		if (MeleeCharacter->bUseControllerRotationYaw) {
			MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(
				MovementRotation,
				AimRotation).Yaw;
		}
		else {
			MovementOffsetYaw = 0;
		}
		// strafing is not required in melee combat
	
	}
	Lean(DeltaTime);
}

void UMelleeAnimInstance::NativeInitializeAnimation()
{

	MeleeCharacter = Cast<AMeleeCharacter>(TryGetPawnOwner());

}

void UMelleeAnimInstance::TurnInPlace()
{
	
}

void UMelleeAnimInstance::Lean(float DeltaTime)
{
	if (MeleeCharacter == nullptr) return;
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = MeleeCharacter->GetActorRotation();

	const FRotator Delta{ UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame) };

	const float Target{ Delta.Yaw / DeltaTime };

	const float Interp{ FMath::FInterpTo(YawDelta, Target, DeltaTime, 6.f) };

	YawDelta = FMath::Clamp(Interp, -90.f, 90.f);

}

