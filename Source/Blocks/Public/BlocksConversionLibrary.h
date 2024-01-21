// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EasyXMLParseManager.h"
#include "Brick.h"
#include "EasyXMLElement.h"
#include "EasyXMLAttribute.h"
#include "ColorTable.h"
#if WITH_EDITOR
	#include "Editor/GroupActor.h"
#endif
#include "BlocksConversionLibrary.generated.h"

struct FRVectorRotator
{
	FVector Loc;
	FQuat Rot;
};

/**
 * 
 */
UCLASS(Blueprintable)
class BLOCKS_API UBlocksConversionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	static TSubclassOf<ABrick> StaticBrickClass;

	static TArray<ABrick*> StaticBrickCache;

	static FRVectorRotator ConvertCoordinateFrameToTransform(UEasyXMLElement* CoordinateFrame);

	static FVector GetSize(UEasyXMLElement* Part);

	static EBrickShape GetShape(UEasyXMLElement* properties);

	static FString GetName(UEasyXMLElement* properties);

	static FLinearColor GetBrickColor(UEasyXMLElement* properties);

	static float GetTransparency(UEasyXMLElement* properties);

	static UEasyXMLElement* GetCoordinateFrame(UEasyXMLElement* properties);

	static UEasyXMLElement* GetProperties(UEasyXMLElement* Part);

	static void HandleCylinderMesh(UEasyXMLElement* CylinderMesh);

	static AActor* HandleDecal(UEasyXMLElement* Part);

	static TArray<AActor*> HandleModel(UEasyXMLElement* Part);

	static TArray<AActor*> HandleSpawnLocation(UEasyXMLElement* Part);

	static ABrick* HandlePart(UEasyXMLElement* Part);

	static TArray<AActor*> ParseItems(TArray<UEasyXMLElement*>& Items);

	static UEasyXMLElement* GetWorkspace(UEasyXMLElement* RootElement);

	UFUNCTION(BlueprintCallable, Category="Conversion")
	static void ConvertRBXL(FString FilePath, TSubclassOf<ABrick> BrickClass);
};
