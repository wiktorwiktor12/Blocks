// cope
#include "Brick.h"



// Sets default values
ABrick::ABrick(const FObjectInitializer& ObjectInitializer)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene0"));
	RootScene->SetMobility(EComponentMobility::Movable);
	RootScene->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	SetRootComponent(RootScene);
	RootScene->SetMobility(EComponentMobility::Movable);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->AttachToComponent(RootScene, FAttachmentTransformRules::KeepRelativeTransform);
	RootScene->SetMobility(EComponentMobility::Movable);
	Mesh->SetMobility(EComponentMobility::Movable);

	if (MeshBall && MeshBlock && MeshCylinder)
	{
		UStaticMesh* MeshToUse = nullptr;

		switch (Shape)
		{
			case EBrickShape::Ball:
				MeshToUse = MeshBall;
				break;
			case EBrickShape::Block:
				MeshToUse = MeshBlock;
				break;
			case EBrickShape::Cylinder:
				MeshToUse = MeshCylinder;
				break;
		}

		if (MeshToUse)
		{
			Mesh->SetStaticMesh(MeshToUse);
			//RefreshMaterial();
		}

		LastShape = Shape;
	}

	RefreshColor();
	LastColor = Color;
}

// Called when the game starts or when spawned
void ABrick::BeginPlay()
{
	Super::BeginPlay();
}

bool ABrick::ShouldTickIfViewportsOnly() const
{
	return true;

	if (GetWorld() != nullptr && GetWorld()->WorldType == EWorldType::Editor)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// Called every frame
void ABrick::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Shape != LastShape && MeshBall && MeshBlock && MeshCylinder)
	{
		UStaticMesh* MeshToUse = nullptr;

		switch (Shape)
		{
			case EBrickShape::Ball:
				MeshToUse = MeshBall;
				break;
			case EBrickShape::Block:
				MeshToUse = MeshBlock;
				break;
			case EBrickShape::Cylinder:
				MeshToUse = MeshCylinder;
				break;
			case EBrickShape::CylinderMesh:
				MeshToUse = MeshCylinderMesh;
				break;
		}

		if (MeshToUse)
		{
			Mesh->SetStaticMesh(MeshToUse);
			RefreshMaterial();
		}

		LastShape = Shape;
	}

	if (Color != LastColor)
	{
		RefreshMaterial();
		RefreshColor();
		LastColor = Color;
	}

	if (Transparency != LastTransparency)
	{
		UpdateTransparency();
		LastTransparency = Transparency;
	}
}

void ABrick::RefreshMaterial()
{
	//Mesh->EmptyOverrideMaterials();
	//for (int i = 0; i < Mesh->GetStaticMesh()->StaticMaterials.Num(); ++i)
	//{
	//	Mesh->SetMaterial(i, Mesh->GetStaticMesh()->StaticMaterials[i].MaterialInterface);
	//}
	//
	//for (int i = Mesh->GetStaticMesh()->StaticMaterials.Num() - 1; i <= 0; ++i)
	//{
	//	//auto oldmat = Mesh->GetStaticMesh()->GetMaterial(i);
	//	//Mesh->OverrideMaterials.Insert(oldmat, i);
	//	Mesh->CreateDynamicMaterialInstance(i,0);
	//}
}

void ABrick::UpdateTransparency()
{
	Mesh->SetScalarParameterValueOnMaterials("Transparency", Transparency);
}

void ABrick::RefreshColor()
{
	/*for (auto& mat : Mesh->GetMaterials())
	{
	auto dynamicMat = Cast<UMaterialInstanceDynamic>(mat);
	if (!dynamicMat) continue;

	dynamicMat->SetVectorParameterValue("brickcolor",Color);
	}*/
	Mesh->SetVectorParameterValueOnMaterials("brickcolor",FVector(Color.R,Color.G,Color.B));
}

