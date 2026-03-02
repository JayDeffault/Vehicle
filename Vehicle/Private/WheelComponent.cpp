#include "WheelComponent.h"

#include "DrawDebugHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

UWheelComponent::UWheelComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UWheelComponent::BeginPlay()
{
    Super::BeginPlay();

    InitializeWheelComponents();
}

void UWheelComponent::InitializeWheelComponents()
{
    Body = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());

    TInlineComponentArray<UStaticMeshComponent*> MeshComponents(GetOwner());
    GetOwner()->GetComponents(MeshComponents);

    if (SweepCollisionComponent == nullptr)
    {
        for (UStaticMeshComponent* MeshComponent : MeshComponents)
        {
            if (MeshComponent != nullptr && MeshComponent->GetFName() == SweepCollisionComponentName)
            {
                SweepCollisionComponent = MeshComponent;
                break;
            }
        }
    }

    if (VisualWheelMeshComponent == nullptr)
    {
        for (UStaticMeshComponent* MeshComponent : MeshComponents)
        {
            if (MeshComponent != nullptr && MeshComponent->GetFName() == VisualWheelComponentName)
            {
                VisualWheelMeshComponent = MeshComponent;
                break;
            }
        }
    }

    if (VisualWheelMeshComponent != nullptr)
    {
        VisualMeshInitialRelativeLocation = VisualWheelMeshComponent->GetRelativeLocation();
    }
}

bool UWheelComponent::PerformSuspensionSweep(FHitResult& OutBestHit, FVector& OutStart, FVector& OutEnd) const
{
    if (GetWorld() == nullptr || SweepCollisionComponent == nullptr)
    {
        return false;
    }

    const FVector SuspensionDirection = -GetUpVector();
    OutStart = GetComponentLocation();
    OutEnd = OutStart + SuspensionDirection * SuspensionLength;

    FComponentQueryParams QueryParams(SCENE_QUERY_STAT(WheelSuspensionSweep), GetOwner());
    TArray<FHitResult> SweepHits;

    const bool bHasAnyHit = GetWorld()->ComponentSweepMulti(
        SweepHits,
        SweepCollisionComponent,
        OutStart,
        OutEnd,
        SweepCollisionComponent->GetComponentQuat(),
        QueryParams);

    if (!bHasAnyHit)
    {
        return false;
    }

    float ClosestDistanceSq = TNumericLimits<float>::Max();
    bool bFoundBlockingHit = false;

    for (const FHitResult& Hit : SweepHits)
    {
        if (!Hit.bBlockingHit || Hit.GetActor() == GetOwner())
        {
            continue;
        }

        const float CurrentDistanceSq = FVector::DistSquared(OutStart, Hit.ImpactPoint);

        if (CurrentDistanceSq < ClosestDistanceSq)
        {
            ClosestDistanceSq = CurrentDistanceSq;
            OutBestHit = Hit;
            bFoundBlockingHit = true;
        }
    }

    return bFoundBlockingHit;
}

void UWheelComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (Body == nullptr || SweepCollisionComponent == nullptr)
    {
        return;
    }

    FHitResult BestHit;
    FVector SweepStart = FVector::ZeroVector;
    FVector SweepEnd = FVector::ZeroVector;

    const bool bHasGroundHit = PerformSuspensionSweep(BestHit, SweepStart, SweepEnd);
    const FVector SuspensionAxis = GetUpVector();

    if (bHasGroundHit)
    {
        const float HitDistance = FVector::Distance(SweepStart, BestHit.ImpactPoint);
        const float CompressionDistance = FMath::Clamp(SuspensionLength - HitDistance, 0.0f, SuspensionLength);
        const float CompressionRatio = CompressionDistance / SuspensionLength;

        const float SpringForce = CompressionRatio * SpringStiffness;
        const float VelocityAlongSuspension = FVector::DotProduct(Body->GetPhysicsLinearVelocityAtPoint(SweepStart), SuspensionAxis);
        const float DampingForce = -VelocityAlongSuspension * DamperStiffness;
        const float TotalSuspensionForce = FMath::Max(SpringForce + DampingForce, 0.0f);

        Body->AddForceAtLocation(SuspensionAxis * TotalSuspensionForce, SweepStart);

        if (VisualWheelMeshComponent != nullptr)
        {
            const float WheelOffset = -(HitDistance - WheelRadius);
            VisualWheelMeshComponent->SetRelativeLocation(
                VisualMeshInitialRelativeLocation + FVector(0.0f, 0.0f, WheelOffset));
        }

        if (bDrawDebug)
        {
            DrawDebugLine(GetWorld(), SweepStart, BestHit.ImpactPoint, FColor::Green, false, -1.0f, 0, 2.0f);
            DrawDebugSphere(GetWorld(), BestHit.ImpactPoint, 8.0f, 12, FColor::Green, false, -1.0f, 0, 1.5f);
        }
    }
    else
    {
        if (VisualWheelMeshComponent != nullptr)
        {
            VisualWheelMeshComponent->SetRelativeLocation(
                VisualMeshInitialRelativeLocation + FVector(0.0f, 0.0f, -SuspensionLength));
        }

        if (bDrawDebug)
        {
            DrawDebugLine(GetWorld(), SweepStart, SweepEnd, FColor::Red, false, -1.0f, 0, 1.5f);
        }
    }
}
