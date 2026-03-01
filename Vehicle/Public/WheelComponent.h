#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "WheelComponent.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UPhysicalMaterial;

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

    void CalculateSweep();
    void CalculatePhysics(float DeltaTime);
    void VisualUpdate();

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
    float WheelRadius = 30.f;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel")
    UStaticMesh* CollisionShape = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel")
    UStaticMesh* VisualWheelMesh = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
    bool bContactPointActive = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
    float CurrentLength = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
    FVector SuspensionForce = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
    FVector ContactLocation = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
    FVector ContactNormal = FVector::UpVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
    FVector TracedHubLocation = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
    float LastSpringForce = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
    float LastDamperForce = 0.f;

private:
    UPROPERTY()
    UStaticMeshComponent* SweepCollisionComponent = nullptr;

    UPROPERTY()
    UStaticMeshComponent* VisualWheelMeshComponent = nullptr;

    UPROPERTY()
    UStaticMeshComponent* Body = nullptr;

    UPROPERTY()
    FHitResult ShapeSweepClosestOutHit;

    bool bHadContactLastFrame = false;
    float LastLength = 0.f;
};
