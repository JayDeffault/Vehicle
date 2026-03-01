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

}

void AVehiclePawn::BeginPlay()
{
    Super::BeginPlay();

}

void AVehiclePawn::NativeAsyncTick(float DeltaTime)
{
    Super::NativeAsyncTick(DeltaTime);

}

void AVehiclePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}