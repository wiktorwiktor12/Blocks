// cope

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Brick.generated.h"

UENUM(Blueprintable)
	enum class EBrickShape : uint8
{
	None = 0,
	Ball = 1,
	Block = 2,
	Cylinder = 3,
	CylinderMesh = 4 // Special Rotated cylinder mesh because of the teleporter
};

UCLASS()
	class BLOCKS_API ABrick : public AActor
{
	GENERATED_BODY()
	
public:	
		// Sets default values for this actor's properties
		ABrick(const FObjectInitializer& ObjectInitializer);

	UPROPERTY()
		USceneComponent* RootScene;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UStaticMesh* MeshBall;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UStaticMesh* MeshBlock;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UStaticMesh* MeshCylinder;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UStaticMesh* MeshCylinderMesh;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly)
	//TSubclassOf<UStaticMesh> MeshBallClass;
	//
	//UPROPERTY(EditAnywhere, BlueprintReadOnly)
	//TSubclassOf<UStaticMesh> MeshBlockClass;
	//
	//UPROPERTY(EditAnywhere, BlueprintReadOnly)
	//TSubclassOf<UStaticMesh> MeshCylinderClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float Transparency = 0.0f;

	float LastTransparency = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		EBrickShape Shape;

	EBrickShape LastShape;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FLinearColor Color = FLinearColor(0.021219, 0.212231, 0.061246, 1);

	FLinearColor LastColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Referent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	virtual bool ShouldTickIfViewportsOnly() const override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void UpdateTransparency();

	void RefreshMaterial();

	//UFUNCTION(BlueprintImplementableEvent)
	void RefreshColor();
};
