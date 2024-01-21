// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BlocksMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class BLOCKS_API UBlocksMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
public:
	//UBlocksMovementComponent(const FObjectInitializer& ObjectInitializer);

	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;

	virtual bool ShouldLimitAirControl(float DeltaTime, const FVector& FallAcceleration) const override;

	virtual bool IsExceedingMaxSpeed(float MaxSpeed) const override;
};
