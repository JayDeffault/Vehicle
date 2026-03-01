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
    UStaticMesh* CollisionShape;

    UPROPERTY()
    UStaticMeshComponent* VisualWheelMeshComponent = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wheel")
    UStaticMesh* VisualWheelMesh;

    UPROPERTY()
    UStaticMeshComponent* Body = nullptr;

    void CalculateSweep();
    void CalculatePhysics(float DeltaTime);
    void VisualUpdate();

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
    float WheelRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension", meta=(ClampMin = -30, ClampMax = 30))
    float TopSpringOffset = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
    float SpringLength = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
    float SpringStiffness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
    float SpringDamping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
    float MaxSuspensionForce = 120000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
    bool bEnableDebugMode;

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
    USceneComponent* ParentComponentRef;
    UPROPERTY()
    FVector ReferenceFrameLocation;
    UPROPERTY()
    FHitResult ShapeSweepClosestOutHit;

    FTransform ReferenceFrameTransform;

    bool bContactPointActive;
    bool bHadContactLastFrame = false;
    float CurrentLength = 0;
    float LastLength = 0;
    FVector ContactLocation;
    FVector ContactNormal;
    UPhysicalMaterial* ContactPhysicalMaterial;
    FVector TracedHubLocation;
    FVector SpringDirection;
    FVector SuspensionForce;
};
