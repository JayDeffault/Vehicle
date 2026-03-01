#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Engine/EngineTypes.h"
#include "Runtime/Engine/Public/CollisionQueryParams.h"
#include "WheelComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VEHICLE_API UWheelComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UWheelComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY()
    UStaticMeshComponent* SweepCollisionComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wheel")
    UStaticMesh* CollisionShape = nullptr;

    UPROPERTY()
    UStaticMeshComponent* VisualWheelMeshComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wheel")
    UStaticMesh* VisualWheelMesh = nullptr;

    UPROPERTY()
    UStaticMeshComponent* Body = nullptr;

    void CalculateSweep();
    void CalculatePhysics(float DeltaTime);
    void VisualUpdate();

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
    float WheelRadius = 30.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension", meta = (ClampMin = -30, ClampMax = 30))
    float TopSpringOffset = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
    float SpringLength = 30.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
    float SpringStiffness = 7000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
    float SpringDamping = 900.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
    float MaxSuspensionForce = 40000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MinGroundNormalAlignment = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
    bool bEnableDebugMode = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
    float LastSpringForce = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
    float LastDamperForce = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
    FVector LastAppliedForce = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
    FVector LastEstimatedTorque = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float DebugForceDrawScale = 0.0025f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float DebugLogInterval = 0.15f;

    float DebugLogTimer = 0.f;

    UPROPERTY()
    FVector ReferenceFrameLocation = FVector::ZeroVector;

    UPROPERTY()
    FTransform ReferenceFrameTransform = FTransform::Identity;

    UPROPERTY()
    FHitResult ShapeSweepClosestOutHit;

    UPROPERTY()
    FVector ContactLocation = FVector::ZeroVector;

    UPROPERTY()
    FVector ContactNormal = FVector::UpVector;

    UPROPERTY()
    UPhysicalMaterial* ContactPhysicalMaterial = nullptr;

    UPROPERTY()
    FVector TracedHubLocation = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
    FVector SpringDirection = FVector::UpVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
    FVector SuspensionForce = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
    bool bContactPointActive = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
    bool bHadContactLastFrame = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
    float CurrentLength = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
    float LastLength = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
    float LastCompressionVelocity = 0.f;
};
