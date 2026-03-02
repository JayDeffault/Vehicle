#include "VehiclePawn.h"

#include "Components/StaticMeshComponent.h"
#include "WheelComponent.h"

AVehiclePawn::AVehiclePawn()
{
    PrimaryActorTick.bCanEverTick = true;

    BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
    SetRootComponent(BodyMesh);
    BodyMesh->SetSimulatePhysics(true);
    BodyMesh->SetEnableGravity(true);
    BodyMesh->SetLinearDamping(ChassisLinearDamping);
    BodyMesh->SetAngularDamping(ChassisAngularDamping);

    FrontLeftWheel = CreateDefaultSubobject<UWheelComponent>(TEXT("FrontLeftWheel"));
    FrontLeftWheel->SetupAttachment(BodyMesh);

    FrontRightWheel = CreateDefaultSubobject<UWheelComponent>(TEXT("FrontRightWheel"));
    FrontRightWheel->SetupAttachment(BodyMesh);

    RearLeftWheel = CreateDefaultSubobject<UWheelComponent>(TEXT("RearLeftWheel"));
    RearLeftWheel->SetupAttachment(BodyMesh);

    RearRightWheel = CreateDefaultSubobject<UWheelComponent>(TEXT("RearRightWheel"));
    RearRightWheel->SetupAttachment(BodyMesh);
}

void AVehiclePawn::BeginPlay()
{
    Super::BeginPlay();

    InitializeVehicle();
}

void AVehiclePawn::InitializeVehicle()
{
    if (BodyMesh == nullptr)
    {
        return;
    }

    BodyMesh->SetSimulatePhysics(true);
    BodyMesh->SetMassOverrideInKg(NAME_None, ChassisMassInKg, true);
    BodyMesh->SetEnableGravity(true);
    BodyMesh->SetLinearDamping(ChassisLinearDamping);
    BodyMesh->SetAngularDamping(ChassisAngularDamping);
    BodyMesh->WakeAllRigidBodies();
}

void AVehiclePawn::NativeAsyncTick(float DeltaTime)
{
    Super::NativeAsyncTick(DeltaTime);

    ApplyAsyncSuspensionForces(DeltaTime);
}

void AVehiclePawn::ApplyAsyncSuspensionForces(float DeltaTime)
{
    if (!bEnableAsyncPhysicsForce || DeltaTime <= SMALL_NUMBER || BodyMesh == nullptr || !BodyMesh->IsSimulatingPhysics())
    {
        return;
    }

    const FVector DownForce = -GetActorUpVector() * ExtraDownforce;
    BodyMesh->AddForce(DownForce);
}

void AVehiclePawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
