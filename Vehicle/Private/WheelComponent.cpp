// Fill out your copyright notice in the Description page of Project Settings.


#include "WheelComponent.h"
#include "AsyncTickFunctions.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/KismetMathLibrary.h"

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
    if (!SweepCollisionComponent)
    {
        bContactPointActive = false;
        return;
    }

    const FVector ShapeSweepStart = ReferenceFrameLocation;
    const FVector SuspensionAxis = ReferenceFrameTransform.GetUnitAxis(EAxis::Z).GetSafeNormal();
    const FVector ShapeSweepEnd = ShapeSweepStart - (SuspensionAxis * (SpringLength + WheelRadius));

    const FQuat ShapeRotation = GetComponentQuat();
    FComponentQueryParams Params;
    Params.AddIgnoredActor(GetOwner());
    TArray<FHitResult> ShapeSweepOutHits;

    FCollisionQueryParams CollisionQueryParams;
    CollisionQueryParams.AddIgnoredActor(GetOwner());

    bContactPointActive = GetOwner()->GetWorld()->ComponentSweepMulti(
        ShapeSweepOutHits,
        SweepCollisionComponent,
        ShapeSweepStart,
        ShapeSweepEnd,
        ShapeRotation,
        Params
    );

    if (bContactPointActive)
    {
        ShapeSweepClosestOutHit = ShapeSweepOutHits[0];

        for (const FHitResult& Hit : ShapeSweepOutHits)
        {
            if (Hit.Distance < ShapeSweepClosestOutHit.Distance)
            {
                ShapeSweepClosestOutHit = Hit;
            }
        }

        ContactLocation = ShapeSweepClosestOutHit.ImpactPoint;
        ContactNormal = ShapeSweepClosestOutHit.ImpactNormal;
        ContactPhysicalMaterial = ShapeSweepClosestOutHit.PhysMaterial.Get();
        TracedHubLocation = ShapeSweepClosestOutHit.Location;
    }
    else
    {
        if (bEnableDebugMode)
        {
            UE_LOG(LogTemp, Warning, TEXT("No hit detected during trace."));
        }
    }

    if (bEnableDebugMode)
    {
        DrawDebugLine(GetWorld(), ShapeSweepStart, ShapeSweepEnd, FColor::Yellow, false, 0, 0, 0);

        if (bContactPointActive)
        {
            DrawDebugLine(GetWorld(), ContactLocation, ShapeSweepStart, FColor::Red, false, 0, 0, 0);
            DrawDebugPoint(GetWorld(), ContactLocation, 12, FColor::Red, false, 0, 0);
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

    const float TotalSuspForce = FMath::Max(LastSpringForce + LastDamperForce, 0.f);

    SpringDirection = ReferenceFrameTransform.GetUnitAxis(EAxis::Z).GetSafeNormal();
    SuspensionForce = SpringDirection * TotalSuspForce;
    LastAppliedForce = SuspensionForce;

    LastEstimatedTorque = FVector::ZeroVector;
    if (Body)
    {
        const FVector ForcePoint = GetComponentLocation();
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
