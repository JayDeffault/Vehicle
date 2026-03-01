// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AsyncTickPawn.h"
#include "GameFramework/Pawn.h"
#include "WheelComponent.h"
#include "VehiclePawn.generated.h"

UCLASS()
class VEHICLE_API AVehiclePawn : public AAsyncTickPawn
{
    GENERATED_BODY()

public:
    AVehiclePawn();

protected:
    virtual void BeginPlay() override;

    virtual void NativeAsyncTick(float DeltaTime) override;

public:
    virtual void Tick(float DeltaTime) override;

};
