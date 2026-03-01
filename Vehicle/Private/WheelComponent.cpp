// Fill out your copyright notice in the Description page of Project Settings.


#include "WheelComponent.h"
#include "AsyncTickFunctions.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "CollisionShape.h"

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
        SweepCollisionComponent->SetUsingAbsoluteScale(false);
        SweepCollisionComponent->SetRelativeScale3D(FVector(1.f));
        SweepCollisionComponent->RegisterComponent();

        SweepCollisionComponent->SetVisibility(false);
        SweepCollisionComponent->SetHiddenInGame(true);

        SweepCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        SweepCollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
        SweepCollisionComponent->SetCollisionObjectType(ECC_Visibility);

        SweepCollisionComponent->SetUsingAbsoluteLocation(false);
        SweepCollisionComponent->SetUsingAbsoluteRotation(false);
        SweepCollisionComponent->SetUsingAbsoluteScale(false);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No CollisionShape set, using default shape."));
    }

    if (VisualWheelMesh)
    {
        VisualWheelMeshComponent = NewObject<UStaticMeshComponent>(this, TEXT("V_WheelMesh_"));
        VisualWheelMeshComponent->SetupAttachment(this);
        VisualWheelMeshComponent->SetStaticMesh(VisualWheelMesh);
        VisualWheelMeshComponent->SetUsingAbsoluteScale(false);
        VisualWheelMeshComponent->SetRelativeScale3D(FVector(1.f));

        {
            const float SideY = GetRelativeLocation().Y;
            FVector S = VisualWheelMeshComponent->GetRelativeScale3D();

            if (SideY > 0.f)
                S.Y = -FMath::Abs(S.Y);
            else
                S.Y = FMath::Abs(S.Y);

            VisualWheelMeshComponent->SetRelativeScale3D(S);
        }

        VisualWheelMeshComponent->RegisterComponent();

        VisualWheelMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);


        VisualWheelMeshComponent->SetUsingAbsoluteLocation(false);
        VisualWheelMeshComponent->SetUsingAbsoluteRotation(false);
        VisualWheelMeshComponent->SetUsingAbsoluteScale(false);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No VisualWheelMesh set, using default mesh."));
    }

    Body = Cast<UStaticMeshComponent>(GetOwner()->GetRootComponent());
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
    const FVector ShapeSweepStart = ReferenceFrameLocation + (SuspensionAxis * TopSpringOffset);
    const FVector ShapeSweepEnd = ShapeSweepStart - (SuspensionAxis * (SpringLength + WheelRadius));

    FCollisionQueryParams CollisionQueryParams(SCENE_QUERY_STAT(WheelSweep), false, GetOwner());
    CollisionQueryParams.bReturnPhysicalMaterial = true;

    FHitResult Hit;
    const FCollisionShape SweepShape = FCollisionShape::MakeSphere(WheelRadius);

    bContactPointActive = World->SweepSingleByChannel(
        Hit,
        ShapeSweepStart,
        ShapeSweepEnd,
        FQuat::Identity,
        ECC_Visibility,
        SweepShape,
        CollisionQueryParams
    );

    if (bContactPointActive)
    {
        ShapeSweepClosestOutHit = Hit;
        ContactLocation = Hit.ImpactPoint;
        ContactNormal = Hit.ImpactNormal;
        ContactPhysicalMaterial = Hit.PhysMaterial.Get();
        TracedHubLocation = Hit.Location;
    }
    else if (bEnableDebugMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("No hit detected during trace."));
    }

    if (bEnableDebugMode)
    {
        DrawDebugLine(World, ShapeSweepStart, ShapeSweepEnd, FColor::Yellow, false, 0.f, 0, 0.f);

        if (bContactPointActive)
        {
            DrawDebugLine(World, ContactLocation, ShapeSweepStart, FColor::Red, false, 0.f, 0, 0.f);
            DrawDebugPoint(World, ContactLocation, 12.f, FColor::Red, false, 0.f, 0);
            DrawDebugSphere(World, TracedHubLocation, WheelRadius, 12, FColor::Green, false, 0.f, 0, 0.8f);
        }
    }
}

void UWheelComponent::CalculatePhysics(float DeltaTime)
{
    CalculateSweep();

    SuspensionForce = FVector::ZeroVector;

    if (!bContactPointActive)
    {
        bHadContactLastFrame = false;
        LastLength = CurrentLength;
        return;
    }

    CurrentLength = FMath::Clamp(ShapeSweepClosestOutHit.Distance - WheelRadius, 0.f, SpringLength);

    if (!bHadContactLastFrame)
    {
        LastLength = CurrentLength;
        bHadContactLastFrame = true;
    }

    const float SafeDeltaTime = FMath::Max(DeltaTime, KINDA_SMALL_NUMBER);
    LastSpringForce = SpringStiffness * (SpringLength - CurrentLength);
    LastDamperForce = SpringDamping * (LastLength - CurrentLength) / SafeDeltaTime;
    LastLength = CurrentLength;

    const float TotalSuspForce = FMath::Clamp(LastSpringForce + LastDamperForce, 0.f, MaxSuspensionForce);

    const float GroundAlignment = FVector::DotProduct(ContactNormal.GetSafeNormal(), FVector::UpVector);
    if (GroundAlignment < MinGroundNormalAlignment)
    {
        bContactPointActive = false;
        bHadContactLastFrame = false;
        SuspensionForce = FVector::ZeroVector;
        return;
    }

    SpringDirection = FVector::UpVector;
    SuspensionForce = SpringDirection * TotalSuspForce;
    LastAppliedForce = SuspensionForce;

    LastEstimatedTorque = FVector::ZeroVector;
    if (Body)
    {
        const FVector ForcePoint = bContactPointActive ? ContactLocation : GetComponentLocation();
        const FVector LeverArm = ForcePoint - Body->GetCenterOfMass();
        LastEstimatedTorque = FVector::CrossProduct(LeverArm, SuspensionForce);

        if (bEnableDebugMode)
        {
            const FVector ForceEnd = ForcePoint + (SuspensionForce * DebugForceDrawScale);
            DrawDebugLine(GetWorld(), ForcePoint, ForceEnd, FColor::Cyan, false, 0.f, 0, 2.f);
            DrawDebugPoint(GetWorld(), ForcePoint, 10.f, FColor::Cyan, false, 0.f, 0);

            DebugLogTimer += DeltaTime;
            if (DebugLogTimer >= DebugLogInterval)
            {
                DebugLogTimer = 0.f;
                UE_LOG(
                    LogTemp,
                    Warning,
                    TEXT("Wheel[%s] Len=%.2f Spring=%.2f Damping=%.2f Force=(%.1f,%.1f,%.1f) Torque=(%.1f,%.1f,%.1f)"),
                    *GetName(),
                    CurrentLength,
                    LastSpringForce,
                    LastDamperForce,
                    SuspensionForce.X,
                    SuspensionForce.Y,
                    SuspensionForce.Z,
                    LastEstimatedTorque.X,
                    LastEstimatedTorque.Y,
                    LastEstimatedTorque.Z
                );
            }
        }
    }
}

void UWheelComponent::VisualUpdate()
{
    if (!VisualWheelMeshComponent) return;

    const float VisualLength = bContactPointActive
        ? CurrentLength
        : SpringLength;

    VisualWheelMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -(VisualLength+WheelRadius)));
}
