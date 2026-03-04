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

        const float CurrentDistanceSq = FMath::Square(Hit.Distance);

        if (CurrentDistanceSq < ClosestDistanceSq)
        {
            ClosestDistanceSq = CurrentDistanceSq;
            OutBestHit = Hit;
            bFoundBlockingHit = true;
        }
    }

    return bFoundBlockingHit;
}

void UWheelComponent::ComputeSuspensionForce(float DeltaTime)
{
    bHasPendingSuspensionForce = false;
    PendingSuspensionForce = FVector::ZeroVector;

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
        if (bDrawDebug)
        {
            DrawDebugLine(GetWorld(), SweepStart, SweepEnd, FColor::Red, false, -1.0f, 0, 1.5f);
        }

        PreviousSpringLength = SuspensionLength;
        bHadGroundContactLastFrame = false;
        return;
    }

    const FVector SuspensionAxis = GetUpVector();
    const float NewSpringLength = FMath::Clamp(BestHit.Distance, 0.0f, SuspensionLength);
    const float SpringDelta = FMath::Clamp(SuspensionLength - NewSpringLength, 0.0f, SuspensionLength);
    const float CompressionRatio = FMath::Clamp(SpringDelta / SuspensionLength, 0.0f, 1.0f);

    float SpringForce = 0.0f;
    if (SpringDelta > 1.0f)
    {
        SpringForce = FMath::Lerp(SpringStiffness * SpringMaxOutputRatio, SpringStiffness, CompressionRatio);
    }
    else
    {
        SpringForce = SpringStiffness * SpringDelta;
    }

    float SpringDamping = DamperStiffness * ((NewSpringLength - PreviousSpringLength) / DeltaTime);
    SpringDamping = FMath::Clamp(SpringDamping, -SpringForce, SpringForce);

    if (!bHadGroundContactLastFrame)
    {
        SpringDamping = 0.0f;
    }

    const float SuspensionForceMagnitude = FMath::Max(SpringForce - SpringDamping, 0.0f);

    PendingSuspensionForce = SuspensionAxis * SuspensionForceMagnitude;
    PendingSuspensionForceLocation = BestHit.ImpactPoint;
    bHasPendingSuspensionForce = SuspensionForceMagnitude > KINDA_SMALL_NUMBER;
    PreviousSpringLength = NewSpringLength;
    bHadGroundContactLastFrame = true;

    if (bDrawDebug)
    {
        DrawDebugLine(GetWorld(), SweepStart, BestHit.ImpactPoint, FColor::Green, false, -1.0f, 0, 2.0f);
        DrawDebugLine(GetWorld(), SweepStart, SweepStart + PendingSuspensionForce * 0.001f, FColor::Blue, false, -1.0f, 0, 1.0f);
        DrawDebugSphere(GetWorld(), BestHit.ImpactPoint, 8.0f, 12, FColor::Green, false, -1.0f, 0, 1.5f);
    }
}

bool UWheelComponent::HasSuspensionForce() const
{
    return bHasPendingSuspensionForce;
}

FVector UWheelComponent::GetSuspensionForce() const
{
    return PendingSuspensionForce;
}

FVector UWheelComponent::GetSuspensionForceLocation() const
{
    return PendingSuspensionForceLocation;
}

void UWheelComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
