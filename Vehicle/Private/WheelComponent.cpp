// Fill out your copyright notice in the Description page of Project Settings.

#include "WheelComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include <limits>

UWheelComponent::UWheelComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UWheelComponent::BeginPlay()
{
    Super::BeginPlay();

    if (CollisionShape)
    {
        SweepCollisionComponent = NewObject<UStaticMeshComponent>(this, TEXT("SweepCollisionTemp"));
        SweepCollisionComponent->SetupAttachment(this);
        SweepCollisionComponent->SetStaticMesh(CollisionShape);
        SweepCollisionComponent->SetRelativeScale3D(FVector(1.f));
        SweepCollisionComponent->SetVisibility(false);
        SweepCollisionComponent->SetHiddenInGame(true);
        SweepCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        SweepCollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
        SweepCollisionComponent->SetCollisionObjectType(ECC_Visibility);
        SweepCollisionComponent->RegisterComponent();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Wheel[%s] has no CollisionShape. Sweep disabled."), *GetName());
    }

    if (VisualWheelMesh)
    {
        VisualWheelMeshComponent = NewObject<UStaticMeshComponent>(this, TEXT("V_WheelMesh_"));
        VisualWheelMeshComponent->SetupAttachment(this);
        VisualWheelMeshComponent->SetStaticMesh(VisualWheelMesh);
        VisualWheelMeshComponent->SetRelativeScale3D(FVector(1.f));

        const float SideY = GetRelativeLocation().Y;
        FVector MeshScale = VisualWheelMeshComponent->GetRelativeScale3D();
        MeshScale.Y = (SideY > 0.f) ? -FMath::Abs(MeshScale.Y) : FMath::Abs(MeshScale.Y);
        VisualWheelMeshComponent->SetRelativeScale3D(MeshScale);

        VisualWheelMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        VisualWheelMeshComponent->RegisterComponent();
    }

    Body = Cast<UStaticMeshComponent>(GetOwner() ? GetOwner()->GetRootComponent() : nullptr);

    CurrentLength = SpringLength;
    LastLength = SpringLength;
}

void UWheelComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UWheelComponent::CalculateSweep()
{
    bContactPointActive = false;

    if (!SweepCollisionComponent)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    const FVector SuspensionAxis = FVector::UpVector;
    const FVector SweepStart = ReferenceFrameLocation + (SuspensionAxis * TopSpringOffset);
    const FVector SweepEnd = SweepStart - (SuspensionAxis * (SpringLength + WheelRadius));

    FComponentQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());

    TArray<FHitResult> Hits;
    const bool bAnyHit = World->ComponentSweepMulti(
        Hits,
        SweepCollisionComponent,
        SweepStart,
        SweepEnd,
        SweepCollisionComponent->GetComponentQuat(),
        QueryParams
    );

    if (!bAnyHit)
    {
        if (bEnableDebugMode)
        {
            DrawDebugLine(World, SweepStart, SweepEnd, FColor::Yellow, false, 0.f, 0, 0.8f);
        }
        return;
    }

    bool bFoundBlockingHit = false;
    FHitResult ClosestHit;
    float ClosestDistance = std::numeric_limits<float>::max();

    for (const FHitResult& Hit : Hits)
    {
        if (!Hit.bBlockingHit)
        {
            continue;
        }

        if (Hit.Distance < ClosestDistance)
        {
            ClosestDistance = Hit.Distance;
            ClosestHit = Hit;
            bFoundBlockingHit = true;
        }
    }

    if (!bFoundBlockingHit)
    {
        return;
    }

    bContactPointActive = true;
    ShapeSweepClosestOutHit = ClosestHit;
    ContactLocation = ClosestHit.ImpactPoint;
    ContactNormal = ClosestHit.ImpactNormal.GetSafeNormal(FVector::UpVector);
    ContactPhysicalMaterial = ClosestHit.PhysMaterial.Get();
    TracedHubLocation = ClosestHit.Location;

    if (bEnableDebugMode)
    {
        DrawDebugLine(World, SweepStart, SweepEnd, FColor::Yellow, false, 0.f, 0, 0.8f);
        DrawDebugLine(World, ContactLocation, TracedHubLocation, FColor::Red, false, 0.f, 0, 1.2f);
        DrawDebugPoint(World, ContactLocation, 9.f, FColor::Red, false, 0.f, 0);
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
        bHadContactLastFrame = false;
        CurrentLength = SpringLength;
        LastLength = SpringLength;
        LastCompressionVelocity = 0.f;
        return;
    }

    const float GroundAlignment = FVector::DotProduct(ContactNormal, FVector::UpVector);
    if (GroundAlignment < MinGroundNormalAlignment)
    {
        bContactPointActive = false;
        bHadContactLastFrame = false;
        CurrentLength = SpringLength;
        LastLength = SpringLength;
        LastCompressionVelocity = 0.f;
        return;
    }

    CurrentLength = FMath::Clamp(ShapeSweepClosestOutHit.Distance - WheelRadius, 0.f, SpringLength);

    if (!bHadContactLastFrame)
    {
        LastLength = CurrentLength;
        bHadContactLastFrame = true;
    }

    const float SafeDeltaTime = FMath::Max(DeltaTime, KINDA_SMALL_NUMBER);
    const float Compression = SpringLength - CurrentLength;
    LastCompressionVelocity = (LastLength - CurrentLength) / SafeDeltaTime;

    LastSpringForce = SpringStiffness * Compression;
    LastDamperForce = SpringDamping * LastCompressionVelocity;

    const float TotalSuspensionForce = FMath::Clamp(LastSpringForce + LastDamperForce, 0.f, MaxSuspensionForce);

    SuspensionForce = SpringDirection * TotalSuspensionForce;
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

        DebugLogTimer += DeltaTime;
        if (DebugLogTimer >= DebugLogInterval)
        {
            DebugLogTimer = 0.f;
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("Wheel[%s] Len=%.2f Comp=%.2f Vel=%.2f Spring=%.1f Damp=%.1f Force=(%.1f,%.1f,%.1f)"),
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
