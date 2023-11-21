// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAnimInstance.h"
#include "EnemyClass.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"


UEnemyAnimInstance :: UEnemyAnimInstance():
	bIsAccelerating(false),
	MovementOffsetYaw(0.f)
{}

void UEnemyAnimInstance::UpdateAnimationProperties(float DeltaTime)
	

{
	if (Enemy == nullptr)
	{
		Enemy = Cast<AEnemyClass>(TryGetPawnOwner());
	}

	if (Enemy)
	{
		FVector Velocity{ Enemy->GetVelocity() };
		Velocity.Z = 0.f;
		Speed = Velocity.Size();



		if (Enemy->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f)
		{
			bIsAccelerating = true;
		}
		else
		{
			bIsAccelerating = false;
		}


		FRotator AimRotation = Enemy->GetBaseAimRotation();

		FRotator MovementRotation =
			UKismetMathLibrary::MakeRotFromX(
				Enemy->GetVelocity());


		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(
			MovementRotation,
			AimRotation).Yaw;
	}

}
