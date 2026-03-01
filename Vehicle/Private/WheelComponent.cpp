// Fill out your copyright notice in the Description page of Project Settings.

#include "WheelComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "CollisionShape.h"
#include "Engine/World.h"

UWheelComponent::UWheelComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UWheelComponent::BeginPlay()
{
    Super::BeginPlay();

    if (VisualWheelMesh)
    {
        VisualWheelMeshComponent = NewObject<UStaticMeshComponent>(this, TEXT("V_WheelMesh_"));
        VisualWheelMeshComponent->SetupAttachment(this);
        VisualWheelMeshComponent->SetStaticMesh(VisualWheelMesh);
        VisualWheelMeshComponent->SetUsingAbsoluteScale(false);
        VisualWheelMeshComponent->SetRelativeScale3D(FVector(1.f));

        const float SideY = GetRelativeLocation().Y;
        FVector MeshScale = VisualWheelMeshComponent->GetRelativeScale3D();
        MeshScale.Y = (SideY > 0.f) ? -FMath::Abs(MeshScale.Y) : FMath::Abs(MeshScale.Y);
        VisualWheelMeshComponent->SetRelativeScale3D(MeshScale);

        VisualWheelMeshComponent->RegisterComponent();
        VisualWheelMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        VisualWheelMeshComponent->SetUsingAbsoluteLocation(false);
        VisualWheelMeshComponent->SetUsingAbsoluteRotation(false);
        VisualWheelMeshComponent->SetUsingAbsoluteScale(false);
    }

    Body = Cast<UStaticMeshComponent>(GetOwner() ? GetOwner()->GetRootComponent() : nullptr);
}

void UWheelComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UWheelComponent::CalculateSweep()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        bContactPointActive = false;
        return;
    }

    const FVector SuspensionAxis = FVector::UpVector;
    const FVector SweepStart = ReferenceFrameLocation + (SuspensionAxis * TopSpringOffset);
    const FVector SweepEnd = SweepStart - (SuspensionAxis * (SpringLength + WheelRadius));

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(WheelSweep), false, GetOwner());
    QueryParams.bReturnPhysicalMaterial = true;

    FHitResult Hit;
    const FCollisionShape SweepShape = FCollisionShape::MakeSphere(WheelRadius);

    bContactPointActive = World->SweepSingleByChannel(
        Hit,
        SweepStart,
        SweepEnd,
        FQuat::Identity,
        ECC_Visibility,
        SweepShape,
        QueryParams
    );

    if (bContactPointActive)
    {
        ShapeSweepClosestOutHit = Hit;
        ContactLocation = Hit.ImpactPoint;
        ContactNormal = Hit.ImpactNormal.GetSafeNormal();
        ContactPhysicalMaterial = Hit.PhysMaterial.Get();
        TracedHubLocation = Hit.Location;
    }

    if (bEnableDebugMode)
    {
        DrawDebugLine(World, SweepStart, SweepEnd, FColor::Yellow, false, 0.f, 0, 0.f);
        if (bContactPointActive)
        {
            DrawDebugLine(World, ContactLocation, TracedHubLocation, FColor::Red, false, 0.f, 0, 1.2f);
            DrawDebugSphere(World, TracedHubLocation, WheelRadius, 12, FColor::Green, false, 0.f, 0, 0.6f);
        }
    }
}

void UWheelComponent::CalculatePhysics(float DeltaTime)
{
    CalculateSweep();

    SuspensionForce = FVector::ZeroVector;
    LastAppliedForce = FVector::ZeroVector;
    LastEstimatedTorque = FVector::ZeroVector;
    SpringDirection = FVector::UpVector;

    if (!bContactPointActive)
    {
        LastLength = SpringLength;
        CurrentLength = SpringLength;
        LastCompressionVelocity = 0.f;
        return;
    }

    const float GroundAlignment = FVector::DotProduct(ContactNormal, FVector::UpVector);
    if (GroundAlignment < MinGroundNormalAlignment)
    {
        bContactPointActive = false;
        LastLength = SpringLength;
        CurrentLength = SpringLength;
        LastCompressionVelocity = 0.f;
        return;
    }

    CurrentLength = FMath::Clamp(ShapeSweepClosestOutHit.Distance - WheelRadius, 0.f, SpringLength);

    if (LastLength <= 0.f || LastLength > SpringLength)
    {
        LastLength = CurrentLength;
    }

    const float Compression = SpringLength - CurrentLength;
    const float SafeDeltaTime = FMath::Max(DeltaTime, KINDA_SMALL_NUMBER);
    LastCompressionVelocity = (LastLength - CurrentLength) / SafeDeltaTime;

    LastSpringForce = SpringStiffness * Compression;
    LastDamperForce = SpringDamping * LastCompressionVelocity;

    float TotalSuspForce = LastSpringForce + LastDamperForce;
    TotalSuspForce = FMath::Clamp(TotalSuspForce, 0.f, MaxSuspensionForce);

    SuspensionForce = SpringDirection * TotalSuspForce;
    LastAppliedForce = SuspensionForce;

    const FVector ForcePoint = TracedHubLocation;
    if (Body)
    {
        const FVector LeverArm = ForcePoint - Body->GetCenterOfMass();
        LastEstimatedTorque = FVector::CrossProduct(LeverArm, SuspensionForce);
    }

    if (bEnableDebugMode)
    {
        const FVector ForceEnd = ForcePoint + (SuspensionForce * DebugForceDrawScale);
        DrawDebugLine(GetWorld(), ForcePoint, ForceEnd, FColor::Cyan, false, 0.f, 0, 2.f);
        DrawDebugPoint(GetWorld(), ForcePoint, 8.f, FColor::Cyan, false, 0.f, 0);

        DebugLogTimer += DeltaTime;
        if (DebugLogTimer >= DebugLogInterval)
        {
            DebugLogTimer = 0.f;
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("Wheel[%s] Len=%.2f Comp=%.2f Vel=%.2f Spring=%.1f Damping=%.1f Force=(%.1f,%.1f,%.1f)"),
                *GetName(),
                CurrentLength,
                Compression,
                LastCompressionVelocity,
                LastSpringForce,
                LastDamperForce,
                SuspensionForce.X,
                SuspensionForce.Y,
                SuspensionForce.Z
            );
        }
    }

    LastLength = CurrentLength;
}

void UWheelComponent::VisualUpdate()
{
    if (!VisualWheelMeshComponent)
    {
        return;
    }

    const float VisualLength = bContactPointActive ? CurrentLength : SpringLength;
    VisualWheelMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -(VisualLength + WheelRadius)));
}
