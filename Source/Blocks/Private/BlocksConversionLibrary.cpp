// Fill out your copyright notice in the Description page of Project Settings.


#include "BlocksConversionLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/PlayerStart.h"
#include "Components/CapsuleComponent.h"
#include <math.h>

DEFINE_LOG_CATEGORY_STATIC(LogBlockLibrary, Log, All);

TSubclassOf<ABrick> UBlocksConversionLibrary::StaticBrickClass = TSubclassOf<ABrick>(ABrick::StaticClass());
TArray<ABrick*> UBlocksConversionLibrary::StaticBrickCache = TArray<ABrick*>();

static void MatrixToEulerYawPitchRoll(float R00, float R01, float R02,
                                      float R10, float R11, float R12,
                                      float R20, float R21, float R22,
                                      float& yaw, float& pitch, float& roll)
{
	if ( R02 < 1.0f ) {
		if ( R02 > -1.0f ) {
			roll = FMath::Atan2( -R12, R22);
			yaw = (float) FMath::Asin(R02);
			pitch = FMath::Atan2( -R01, R00);
		} else {
			// WARNING.  Not unique.  XA - ZA = -atan2(r10,r11)
			roll = -FMath::Atan2(R10, R11);
			yaw = -(float)(PI / 2);
			pitch = 0.0f;
		}
	} else {
		// WARNING.  Not unique.  XAngle + ZAngle = atan2(r10,r11)
		roll = FMath::Atan2(R10, R11);
		yaw = (float)(PI / 2);
		pitch = 0.0f;
	}
}

static float WrapAngle(float angle)
{
	while (angle <= -180.f) angle += 360.f;
	while (angle > 180.f) angle -= 360.f;
	return angle;
}

//Credit to a maths site linked below for this function, special function is used due to UE4 and ROBLOX using different coordinate systems, Unreal engine (Z-Up) VS roblox (Y-Up) 
//https://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
static FQuat MatToQuat(FMatrix Mat)
{
	FQuat q;
	float trace = Mat.M[0][0] + Mat.M[1][1] + Mat.M[2][2]; // I removed + 1.0f; see discussion with Ethan
	if( trace > 0 ) {// I changed M_EPSILON to 0
		float s = 0.5f / FMath::Sqrt(trace+ 1.0f);
		q.W = 0.25f / s;
		q.X = ( Mat.M[2][1] - Mat.M[1][2] ) * s;
		q.Y = ( Mat.M[0][2] - Mat.M[2][0] ) * s;
		q.Z = ( Mat.M[1][0] - Mat.M[0][1] ) * s;
	} else {
		if ( Mat.M[0][0] > Mat.M[1][1] && Mat.M[0][0] > Mat.M[2][2] ) {
			float s = 2.0f * FMath::Sqrt( 1.0f + Mat.M[0][0] - Mat.M[1][1] - Mat.M[2][2]);
			q.W = (Mat.M[2][1] - Mat.M[1][2] ) / s;
			q.X = 0.25f * s;
			q.Y = (Mat.M[0][1] + Mat.M[1][0] ) / s;
			q.Z = (Mat.M[0][2] + Mat.M[2][0] ) / s;
		} else if (Mat.M[1][1] > Mat.M[2][2]) {
			float s = 2.0f * FMath::Sqrt( 1.0f + Mat.M[1][1] - Mat.M[0][0] - Mat.M[2][2]);
			q.W = (Mat.M[0][2] - Mat.M[2][0] ) / s;
			q.X = (Mat.M[0][1] + Mat.M[1][0] ) / s;
			q.Y = 0.25f * s;
			q.Z = (Mat.M[1][2] + Mat.M[2][1] ) / s;
		} else {
			float s = 2.0f * FMath::Sqrt( 1.0f + Mat.M[2][2] - Mat.M[0][0] - Mat.M[1][1] );
			q.W = (Mat.M[1][0] - Mat.M[0][1] ) / s;
			q.X = (Mat.M[0][2] + Mat.M[2][0] ) / s;
			q.Y = (Mat.M[1][2] + Mat.M[2][1] ) / s;
			q.Z = 0.25f * s;
		}
	}
	return q;
}

// Parse ROBLOX's CFrame into a FRVectorRotator
FRVectorRotator UBlocksConversionLibrary::ConvertCoordinateFrameToTransform(UEasyXMLElement* CoordinateFrame)
{
	FRVectorRotator Transform;

	float R00 = 0.0f;
	float R01 = 0.0f;
	float R02 = 0.0f;
	float R10 = 0.0f;
	float R11 = 0.0f;
	float R12 = 0.0f;
	float R20 = 0.0f;
	float R21 = 0.0f;
	float R22 = 0.0f;

	float X = 0.0f;
	float Y = 0.0f;
	float Z = 0.0f;

	for (UEasyXMLElement*& elm : CoordinateFrame->Children) // this is ugly as hell, but C++ does not support switch statements in strings, this could be optimized by using fnames for comparisons instead
	{
		if (elm->Name == L"X")
			X = elm->GetFloatValue() * 25;
		else if (elm->Name == L"Y")
			Z = elm->GetFloatValue() * 25;
		else if (elm->Name == L"Z")
			Y = elm->GetFloatValue() * 25;
		else if (elm->Name == L"R00")
			R00 = elm->GetFloatValue();
		else if (elm->Name == L"R01")
			R01 = elm->GetFloatValue();
		else if (elm->Name == L"R02")
			R02 = elm->GetFloatValue();
		else if (elm->Name == L"R10")
			R10 = elm->GetFloatValue();
		else if (elm->Name == L"R11")
			R11 = elm->GetFloatValue();
		else if (elm->Name == L"R12")
			R12 = elm->GetFloatValue();
		else if (elm->Name == L"R20")
			R20 = elm->GetFloatValue();
		else if (elm->Name == L"R21")
			R21 = elm->GetFloatValue();
		else if (elm->Name == L"R22")
			R22 = elm->GetFloatValue();
	}

	auto toDegrees = [&](float rad) -> float
	{
		return rad * 180.0 / 3.1415926535898;
	};

	FMatrix mat;
	mat.M[0][0] = R00;
	mat.M[0][1] = R01;
	mat.M[0][2] = R02;
	mat.M[1][0] = R10;
	mat.M[1][1] = R11;
	mat.M[1][2] = R12;
	mat.M[2][0] = R20;
	mat.M[2][1] = R21;
	mat.M[2][2] = R22;

	FQuat rightquat = MatToQuat(mat);
	rightquat = FQuat(rightquat.X, rightquat.Z, rightquat.Y, -rightquat.W);

	UE_LOG(LogBlockLibrary, Warning, L"R00: %f\n R01: %f\n R02: %f\n R10: %f\n R11: %f\n R12: %f\n R20: %f\n R21: %f\n R22: %f", R00, R01, R02, R10, R11, R12, R20, R21, R22);

	FRotator rot = rightquat.Rotator();

	UE_LOG(LogBlockLibrary,Warning, L"\nX: %f, Y: %f, Z: %f\n",rot.Roll,rot.Yaw,rot.Pitch);

	FVector loc;
	loc.X = X;
	loc.Y = Y;
	loc.Z = Z;

	Transform.Loc = loc;
	Transform.Rot = rightquat;

	return Transform;
}

//Parse the size of the Part
FVector UBlocksConversionLibrary::GetSize(UEasyXMLElement* Part)
{
	EEasyXMLParserFound eresult;
	FVector Result = FVector(1,1,1);

	//Get the size element
	UEasyXMLElement* SizeElement = nullptr;
	for (UEasyXMLElement*& elm : GetProperties(Part)->Children)
	{
		if (!elm) continue;

		if (auto NameAttribute = elm->GetAttribute(L"name", eresult))
		{
			FString attrName = NameAttribute->GetStringValue();
			if (attrName == L"size")
			{
				SizeElement = elm;
				break;
			}
		}
	}

	//This is bad if it happens
	if (!SizeElement)
	{
		GEngine->AddOnScreenDebugMessage(0, 1.0f, FColor::Red, L"FAILED TO FIND SIZE ELEMENT");
		return Result;
	}

	for (UEasyXMLElement*& elm : SizeElement->Children)
	{
		if (elm->Name == L"X")
			Result.X = elm->GetFloatValue();
		else if (elm->Name == L"Y")
			Result.Z = elm->GetFloatValue();
		else if (elm->Name == L"Z")
			Result.Y = elm->GetFloatValue();
	}

	return Result;
}

//Parse the shape of the part
EBrickShape UBlocksConversionLibrary::GetShape(UEasyXMLElement* properties)
{
	EEasyXMLParserFound result;
	EBrickShape Shape;
	for (UEasyXMLElement*& elm : properties->Children)
	{
		if (auto NameAttribute = elm->GetAttribute(L"name", result))
		{
			FString attrName = NameAttribute->GetStringValue();
			if (attrName == L"shape")
			{
				switch (elm->GetIntValue())
				{
					case 0:
						Shape = EBrickShape::Ball;
						break;

					case 1:
						Shape = EBrickShape::Block;
						break;

					case 2:
						Shape = EBrickShape::Cylinder;
						break;

					default:
						Shape = EBrickShape::Block;
						break;
				}
				//Shape = (EBrickShape)elm->GetIntValue();
				break;
			}
		}
	}

	return Shape;
}

FString UBlocksConversionLibrary::GetName(UEasyXMLElement* properties)
{
	EEasyXMLParserFound result;
	FString Name;
	for (UEasyXMLElement*& elm : properties->Children)
	{
		if (auto NameAttribute = elm->GetAttribute(L"name", result))
		{
			FString attrName = NameAttribute->GetStringValue();
			if (attrName == L"Name")
			{
				Name = elm->GetStringValue();
				break;
			}
		}
	}

	return Name;
}

FLinearColor UBlocksConversionLibrary::GetBrickColor(UEasyXMLElement* properties)
{
	EEasyXMLParserFound result;
	FLinearColor Color;
	for (UEasyXMLElement*& elm : properties->Children)
	{
		if (auto NameAttribute = elm->GetAttribute(L"name", result))
		{
			FString attrName = NameAttribute->GetStringValue();
			if (attrName == L"BrickColor")
			{
				Color = FLinearColor(BrickColorTable[elm->GetIntValue()]); // look up the linear color in a brickcolor lookup table
				break;
			}
		}
	}

	return Color;
}

float UBlocksConversionLibrary::GetTransparency(UEasyXMLElement* properties)
{
	EEasyXMLParserFound result;
	float Transparency = 0.0f;
	for (UEasyXMLElement*& elm : properties->Children)
	{
		if (auto NameAttribute = elm->GetAttribute(L"name", result))
		{
			FString attrName = NameAttribute->GetStringValue();
			if (attrName == L"Transparency")
			{
				Transparency = elm->GetFloatValue();
				break;
			}
		}
	}

	return Transparency;
}

UEasyXMLElement* UBlocksConversionLibrary::GetCoordinateFrame(UEasyXMLElement* properties)
{
	EEasyXMLParserFound result;
	for (UEasyXMLElement*& elm : properties->Children)
	{
		if (auto NameAttribute = elm->GetAttribute(L"name", result))
		{
			FString Name = NameAttribute->GetStringValue();
			if (Name == L"CFrame")
				return elm;
		}
	}
	return nullptr;
}

UEasyXMLElement* UBlocksConversionLibrary::GetProperties(UEasyXMLElement* Part)
{
	for (UEasyXMLElement*& elm : Part->Children)
	{
		if (!elm) continue;

		if (elm->Name == L"Properties")
			return elm;
	}
	return nullptr;
}

void UBlocksConversionLibrary::HandleCylinderMesh(UEasyXMLElement* CylinderMesh)
{
	EEasyXMLParserFound result;

	auto partParent = Cast<UEasyXMLElement>(CylinderMesh->Parent);
	if (!partParent) return;

	FString Referent = partParent->GetAttribute(L"referent", result)->GetStringValue(); //prob should null check, oh well

	ABrick* ParentBrick = nullptr;
	for (int i = 0; i < StaticBrickCache.Num(); ++i)
	{
		auto Brick = StaticBrickCache[i];
		if (Brick && Brick->Referent == Referent) // find the parent brick using the referent
		{
			ParentBrick = Brick;
			break;
		}
	}

	if (ParentBrick)
		ParentBrick->Shape = EBrickShape::CylinderMesh;
}

AActor* UBlocksConversionLibrary::HandleDecal(UEasyXMLElement* Part)
{
	return nullptr;
}

TArray<AActor*> UBlocksConversionLibrary::HandleModel(UEasyXMLElement* Part)
{
	TArray<AActor*> resultarray;
	if (Part->Children.Num() > 0)
	{
		auto parsedChildren = ParseItems(Part->Children);
#if WITH_EDITOR //with editor incase you want to use this kismet library at runtime on a shipping release, shipping doesn't need groupactors
		if (parsedChildren.Num() > 0)
		{
			auto GroupActor = GWorld->SpawnActor<AGroupActor>(AGroupActor::StaticClass(), parsedChildren[0]->GetActorLocation(), FRotator());
			if (GroupActor)
			{
				for (AActor*& child : parsedChildren)
					GroupActor->Add(*child); //who the fuck wrote this function and why does it use a actor reference???

				GroupActor->bLocked = true;
				resultarray.Add(GroupActor);
			}
		}
#endif
		resultarray.Append(parsedChildren);
	}
	return resultarray;
}

TArray<AActor*> UBlocksConversionLibrary::HandleSpawnLocation(UEasyXMLElement* Part)
{
	TArray<AActor*> resultarray;
	auto Brick = HandlePart(Part);
	if (Brick)
	{
		FActorSpawnParameters params;
		params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		auto trans = Brick->GetActorTransform();
		FVector BoxExtent;
		FVector Origin;

		Brick->GetActorBounds(true,Origin,BoxExtent,true);

		trans.SetLocation(trans.GetLocation() + FVector(0, 0, BoxExtent.Z));
		trans.SetScale3D(FVector(1, 1, 1));
		auto playerstart = GWorld->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), trans, params);
		//playerstart->GetActorBounds(true,Origin,BoxExtent,true);
		playerstart->SetActorLocation(playerstart->GetActorLocation() + FVector(0, 0, playerstart->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight()));
		
		resultarray.Add(Brick);
		resultarray.Add(playerstart);
	}
	return resultarray;
}

ABrick* UBlocksConversionLibrary::HandlePart(UEasyXMLElement* Part)
{
	EEasyXMLParserFound result;

	auto Properties = GetProperties(Part);
	if (!Properties) return nullptr;

	auto CoordinateFrame = GetCoordinateFrame(Properties);

	FRVectorRotator SpawnTransform = ConvertCoordinateFrameToTransform(CoordinateFrame);
	FVector Scale = GetSize(Part);
	//SpawnTransform.SetScale3D(GetSize(Part));

	FActorSpawnParameters spawnparams; 
	//spawnparams.Name = FName(*GetName(Properties)); //for some reason this does not work properly, prob because of duplicate names
	spawnparams.bNoFail = true;
	spawnparams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	auto spawned = GWorld->SpawnActor<ABrick>(StaticBrickClass.Get(), SpawnTransform.Loc, FRotator(0,45,0), spawnparams);
	spawned->SetActorScale3D(Scale);
	spawned->SetActorRelativeRotation(SpawnTransform.Rot);
	spawned->Color = GetBrickColor(Properties);
	spawned->Shape = GetShape(Properties);
	spawned->Transparency = GetTransparency(Properties);
	FString referent = Part->GetAttribute(L"referent", result)->GetStringValue();
	spawned->Referent = referent;

	StaticBrickCache.Add(spawned);
	UE_LOG(LogBlockLibrary, Warning, L"Spawned Part with referent: ", *referent);

	return spawned;
}

TArray<AActor*> UBlocksConversionLibrary::ParseItems(TArray<UEasyXMLElement*>& Items)
{
	TArray<AActor*> resultArray;

	EEasyXMLParserFound result;
	for (UEasyXMLElement*& elm : Items)
	{
		if (auto ClassAttribute = elm->GetAttribute(L"class", result))
		{
			 // comparisons could be optimized with fnames
			FString ClassName = ClassAttribute->GetStringValue();
			if (ClassName == L"Part")
			{
				auto part = HandlePart(elm);
				resultArray.Add(part);
				if (elm->Children.Num() > 0)
				{
					auto parsedChildren = ParseItems(elm->Children);
					resultArray.Append(parsedChildren);
				}
			}
			else if (ClassName == L"Model") // for models, its children will be handled in handlemodel
			{
				auto parsedChildren = HandleModel(elm);
				resultArray.Append(parsedChildren);
			}
			else if (ClassName == L"SpawnLocation")
			{
				auto actors = HandleSpawnLocation(elm);
				resultArray.Append(actors);
				if (elm->Children.Num() > 0)
				{
					auto parsedChildren = ParseItems(elm->Children);
					resultArray.Append(parsedChildren);
				}
			}
			else if (ClassName == L"CylinderMesh")
			{
				HandleCylinderMesh(elm);
				if (elm->Children.Num() > 0)
				{
					auto parsedChildren = ParseItems(elm->Children);
					resultArray.Append(parsedChildren);
				}
			}
		}
	}

	return resultArray;
}

UEasyXMLElement* UBlocksConversionLibrary::GetWorkspace(UEasyXMLElement* RootElement)
{
	EEasyXMLParserFound result;
	TArray<UEasyXMLElement*> elements = RootElement->ReadElements(L"roblox.Item", result); //get the root of everything

	for (UEasyXMLElement*& elm : elements)
	{
		if (auto ClassAttribute = elm->GetAttribute(L"class", result))
		{
			FString Name = ClassAttribute->GetStringValue();
			if (Name == L"Workspace")
				return elm;
		}
	}
	return nullptr;
}

void UBlocksConversionLibrary::ConvertRBXL(FString FilePath, TSubclassOf<ABrick> BrickClass)
{
	StaticBrickCache.Empty(); //never can be too safe, unless you can

	if (StaticBrickClass.Get())
		StaticBrickClass = BrickClass;
	else
	{
		StaticBrickClass = ABrick::StaticClass();
		GEngine->AddOnScreenDebugMessage(0, 1.0f, FColor::Yellow, L"NO BRICK CLASS SPECIFIED");
	}

	EEasyXMLParserErrorCode result;
	FString ErrorMessage;
	auto xml = UEasyXMLParseManager::LoadFromFile(FilePath, true, result, ErrorMessage);
	
	if (result == EEasyXMLParserErrorCode::Failed)
	{
		GEngine->AddOnScreenDebugMessage(0, 1.0f, FColor::Red, ErrorMessage);
		return;
	}

	auto Workspace = GetWorkspace(xml); // currently only load from workspace, perhaps in future add support from reading from lighting and adding an option to choose what to read from there
	if (!Workspace)
	{
		GEngine->AddOnScreenDebugMessage(0, 1.0f, FColor::Red, L"FAILED TO FIND WORKSPACE");
		return;
	}

	ParseItems(Workspace->Children);

	StaticBrickCache.Empty();
}