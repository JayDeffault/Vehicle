#include "WheelComponent.h"

#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

UWheelComponent::UWheelComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UWheelComponent::BeginPlay()
{
    Super::BeginPlay();

    Body = Cast<UStaticMeshComponent>(GetOwner() ? GetOwner()->GetRootComponent() : nullptr);

    if (CollisionShape)
    {
        SweepCollisionComponent = NewObject<UStaticMeshComponent>(this, TEXT("SweepCollisionTemp"));
        SweepCollisionComponent->SetupAttachment(this);
        SweepCollisionComponent->SetStaticMesh(CollisionShape);
        SweepCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        SweepCollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
        SweepCollisionComponent->SetHiddenInGame(true);
        SweepCollisionComponent->SetVisibility(false);
        SweepCollisionComponent->RegisterComponent();
    }

    if (VisualWheelMesh)
    {
        VisualWheelMeshComponent = NewObject<UStaticMeshComponent>(this, TEXT("VisualWheelMesh"));
        VisualWheelMeshComponent->SetupAttachment(this);
        VisualWheelMeshComponent->SetStaticMesh(VisualWheelMesh);

        FVector MeshScale = VisualWheelMeshComponent->GetRelativeScale3D();
        MeshScale.Y = (GetRelativeLocation().Y > 0.f) ? -FMath::Abs(MeshScale.Y) : FMath::Abs(MeshScale.Y);
        VisualWheelMeshComponent->SetRelativeScale3D(MeshScale);

        VisualWheelMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        VisualWheelMeshComponent->RegisterComponent();
    }

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

    const FVector SweepStart = GetComponentLocation();
    const FVector SweepEnd = SweepStart - (FVector::UpVector * (SpringLength + WheelRadius));

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

    int32 BestHitIdx = INDEX_NONE;
    float BestDistance = TNumericLimits<float>::Max();

    for (int32 Idx = 0; Idx < Hits.Num(); ++Idx)
    {
        const FHitResult& Hit = Hits[Idx];
        if (!Hit.bBlockingHit)
        {
            continue;
        }

        if (Hit.Distance < BestDistance)
        {
            BestDistance = Hit.Distance;
            BestHitIdx = Idx;
        }
    }

    if (BestHitIdx == INDEX_NONE)
    {
        return;
    }

    const FHitResult& BestHit = Hits[BestHitIdx];

    ShapeSweepClosestOutHit = BestHit;
    bContactPointActive = true;
    ContactLocation = BestHit.ImpactPoint;
    ContactNormal = BestHit.ImpactNormal.GetSafeNormal(FVector::UpVector);
    TracedHubLocation = BestHit.Location;

    if (bEnableDebugMode)
    {
        DrawDebugLine(World, SweepStart, SweepEnd, FColor::Yellow, false, 0.f, 0, 0.8f);
        DrawDebugLine(World, ContactLocation, TracedHubLocation, FColor::Red, false, 0.f, 0, 1.2f);
        DrawDebugPoint(World, ContactLocation, 8.f, FColor::Red, false, 0.f, 0);
    }
}

void UWheelComponent::CalculatePhysics(float DeltaTime)
{
    CalculateSweep();

    SuspensionForce = FVector::ZeroVector;
    LastSpringForce = 0.f;
    LastDamperForce = 0.f;

    if (!bContactPointActive)
    {
        bHadContactLastFrame = false;
        CurrentLength = SpringLength;
        LastLength = SpringLength;
        return;
    }

    if (FVector::DotProduct(ContactNormal, FVector::UpVector) < MinGroundNormalAlignment)
    {
        bContactPointActive = false;
        bHadContactLastFrame = false;
        CurrentLength = SpringLength;
        LastLength = SpringLength;
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
    const float CompressionVelocity = (LastLength - CurrentLength) / SafeDeltaTime;

    LastSpringForce = SpringStiffness * Compression;
    LastDamperForce = SpringDamping * CompressionVelocity;

    const float TotalForce = FMath::Clamp(LastSpringForce + LastDamperForce, 0.f, MaxSuspensionForce);
    SuspensionForce = FVector::UpVector * TotalForce;

    if (bEnableDebugMode)
    {
        DrawDebugLine(GetWorld(), TracedHubLocation, TracedHubLocation + (SuspensionForce * 0.0025f), FColor::Cyan, false, 0.f, 0, 2.f);
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
