#pragma once

#include "CoreMinimal.h"
#include "AsyncTickPawn.h"
#include "VehiclePawn.generated.h"

class UStaticMeshComponent;
class UWheelComponent;

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

private:
    void InitializeVehicle();
    void ApplyAsyncSuspensionForces(float DeltaTime);

private:
    UPROPERTY(VisibleAnywhere, Category = "Vehicle")
    UStaticMeshComponent* BodyMesh = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Vehicle")
    UWheelComponent* FrontLeftWheel = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Vehicle")
    UWheelComponent* FrontRightWheel = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Vehicle")
    UWheelComponent* RearLeftWheel = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Vehicle")
    UWheelComponent* RearRightWheel = nullptr;

    UPROPERTY(EditAnywhere, Category = "Vehicle|Physics")
    float ChassisMassInKg = 1200.0f;

    UPROPERTY(EditAnywhere, Category = "Vehicle|Physics")
    float ExtraDownforce = 2500.0f;

    UPROPERTY(EditAnywhere, Category = "Vehicle|Physics")
    float ChassisLinearDamping = 1.8f;

    UPROPERTY(EditAnywhere, Category = "Vehicle|Physics")
    float ChassisAngularDamping = 6.0f;

    UPROPERTY(EditAnywhere, Category = "Vehicle|Physics")
    bool bEnableAsyncPhysicsForce = true;
};
