#include "WheelComponent.h"

#include "DrawDebugHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"

UWheelComponent::UWheelComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UWheelComponent::BeginPlay()
{
    Super::BeginPlay();

    InitializeWheelComponents();
    PreviousSpringLength = SuspensionLength;
}

void UWheelComponent::InitializeWheelComponents()
{
    Body = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());

    if (SweepCollisionComponent == nullptr)
    {
        SweepCollisionComponent = NewObject<UStaticMeshComponent>(GetOwner(), TEXT("SweepCollisionComponent"));

        if (SweepCollisionComponent != nullptr)
        {
            GetOwner()->AddInstanceComponent(SweepCollisionComponent);
            SweepCollisionComponent->SetupAttachment(this);
            SweepCollisionComponent->RegisterComponent();
        }
    }

    if (SweepCollisionComponent != nullptr)
    {
        SweepCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        SweepCollisionComponent->SetGenerateOverlapEvents(false);

        if (CollisionShape != nullptr)
        {
            SweepCollisionComponent->SetStaticMesh(CollisionShape);
        }
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
    OutEnd = OutStart + SuspensionDirection * (SuspensionLength + WheelRadius);

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

    if (Body == nullptr || SweepCollisionComponent == nullptr || !Body->IsSimulatingPhysics() || DeltaTime <= SMALL_NUMBER)
    {
        return;
    }

    FHitResult BestHit;
    FVector SweepStart = FVector::ZeroVector;
    FVector SweepEnd = FVector::ZeroVector;

    const bool bHasGroundHit = PerformSuspensionSweep(BestHit, SweepStart, SweepEnd);

    if (!bHasGroundHit)
    {
        PreviousSpringLength = SuspensionLength;

        if (bDrawDebug)
        {
            DrawDebugLine(GetWorld(), SweepStart, SweepEnd, FColor::Red, false, -1.0f, 0, 1.5f);
        }

        return;
    }

    const FVector SuspensionAxis = GetUpVector();
    const float HitDistance = FVector::Distance(SweepStart, BestHit.ImpactPoint);
    const float CurrentSpringLength = FMath::Clamp(HitDistance - WheelRadius, 0.0f, SuspensionLength);
    const float CompressionDistance = SuspensionLength - CurrentSpringLength;

    const float SpringForce = CompressionDistance * SpringStiffness;
    const float SpringVelocity = (PreviousSpringLength - CurrentSpringLength) / DeltaTime;
    const float DampingForce = SpringVelocity * DamperStiffness;

    const float GroundAlignment = FMath::Clamp(FVector::DotProduct(BestHit.ImpactNormal, SuspensionAxis), 0.0f, 1.0f);
    const float TotalSuspensionForce = FMath::Clamp((SpringForce + DampingForce) * GroundAlignment, 0.0f, MaxSuspensionForce);

    Body->AddForceAtLocation(SuspensionAxis * TotalSuspensionForce, SweepStart);
    PreviousSpringLength = CurrentSpringLength;

    if (bDrawDebug)
    {
        DrawDebugLine(GetWorld(), SweepStart, BestHit.ImpactPoint, FColor::Green, false, -1.0f, 0, 2.0f);
        DrawDebugSphere(GetWorld(), BestHit.ImpactPoint, 8.0f, 12, FColor::Green, false, -1.0f, 0, 1.5f);
    }
}
