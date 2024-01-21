// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BrickPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BLOCKS_API ABrickPlayerController : public APlayerController
{
	GENERATED_BODY()
	
	void MoveForward(float Value);
	void MoveRight(float Value);

	void Jump();
	void StopJump();

	virtual void SetupInputComponent() override;
};
