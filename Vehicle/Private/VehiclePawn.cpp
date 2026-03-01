// Fill out your copyright notice in the Description page of Project Settings.


#include "VehiclePawn.h"
#include "AsyncTickFunctions.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "PhysicsEngine/BodySetup.h"
#include "Engine/StaticMesh.h"


AVehiclePawn::AVehiclePawn()
{
    PrimaryActorTick.bCanEverTick = true;

    CarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CarMesh"));
    CarMesh->SetSimulatePhysics(true);
    CarMesh->SetNotifyRigidBodyCollision(true);
    RootComponent = CarMesh;
}

void AVehiclePawn::BeginPlay()
{
    Super::BeginPlay();

    GetComponents<UWheelComponent>(WheelComponents);

}

void AVehiclePawn::NativeAsyncTick(float DeltaTime)
{
    Super::NativeAsyncTick(DeltaTime);

    for (UWheelComponent* WheelComp : WheelComponents)
    {
        if (!WheelComp) continue;

        WheelComp->ReferenceFrameTransform = GetTransform();
        WheelComp->ReferenceFrameLocation = WheelComp->GetComponentLocation();

        WheelComp->CalculatePhysics(DeltaTime);

        const FVector ForceApplicationPoint = WheelComp->bContactPointActive
            ? WheelComp->ContactLocation
            : WheelComp->GetComponentLocation();

        UAsyncTickFunctions::ATP_AddForceAtPosition(CarMesh, WheelComp->SuspensionForce, ForceApplicationPoint);
    }
}

void AVehiclePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    for (UWheelComponent* WheelComp : WheelComponents)
    {
        if (!WheelComp) continue;

        WheelComp->VisualUpdate();
    }
}