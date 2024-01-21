// Fill out your copyright notice in the Description page of Project Settings.


#include "BrickPlayerController.h"
#include "GameFramework/Character.h"

void ABrickPlayerController::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		if (K2_GetPawn())
		{
			// find out which way is right
			const FRotator Rotation = GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get right vector 
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
			K2_GetPawn()->AddMovementInput(Direction, Value);
		}
	}
}

void ABrickPlayerController::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		if (K2_GetPawn())
		{
			// find out which way is forward
			const FRotator Rotation = GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get forward vector
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			K2_GetPawn()->AddMovementInput(Direction, Value);
		}
	}
}

void ABrickPlayerController::Jump()
{
	if (ACharacter* CurrentPawn = Cast<ACharacter>(K2_GetPawn()))
	{
		CurrentPawn->Jump();
	}
}

void ABrickPlayerController::StopJump()
{
	if (ACharacter* CurrentPawn = Cast<ACharacter>(K2_GetPawn()))
	{
		CurrentPawn->StopJumping();
	}
}

void ABrickPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	check(InputComponent);

	if (InputComponent != NULL)
	{
		// Basis of movement
		InputComponent->BindAxis("MoveForward", this, &ABrickPlayerController::MoveForward);
		InputComponent->BindAxis("MoveRight", this, &ABrickPlayerController::MoveRight);

		InputComponent->BindAction("Jump", IE_Pressed, this, &ABrickPlayerController::Jump);
		InputComponent->BindAction("Jump", IE_Released, this, &ABrickPlayerController::StopJump);

		InputComponent->BindAxis("Turn", this, &ABrickPlayerController::AddYawInput);
		InputComponent->BindAxis("LookUp", this, &ABrickPlayerController::AddPitchInput);
	}
}