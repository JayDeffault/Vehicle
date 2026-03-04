#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "WheelComponent.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UPrimitiveComponent;

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

    void ComputeSuspensionForce(float DeltaTime);
    bool HasSuspensionForce() const;
    FVector GetSuspensionForce() const;
    FVector GetSuspensionForceLocation() const;

private:
    void InitializeWheelComponents();
    bool PerformSuspensionSweep(FHitResult& OutBestHit, FVector& OutStart, FVector& OutEnd) const;

private:
    UPROPERTY(EditAnywhere, Category = "Suspension")
    float SuspensionLength = 45.0f;

    UPROPERTY(EditAnywhere, Category = "Suspension")
    float SpringStiffness = 120000.0f;

    UPROPERTY(EditAnywhere, Category = "Suspension")
    float DamperStiffness = 4500.0f;

    UPROPERTY(EditAnywhere, Category = "Suspension")
    float SpringMaxOutputRatio = 0.65f;

    UPROPERTY(EditAnywhere, Category = "Suspension")
    float WheelRadius = 18.0f;

    UPROPERTY(EditAnywhere, Category = "Wheel")
    UStaticMeshComponent* SweepCollisionComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wheel", meta = (AllowPrivateAccess = "true"))
    UStaticMesh* CollisionShape = nullptr;

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDrawDebug = false;

    UPROPERTY()
    UPrimitiveComponent* Body = nullptr;

    FVector PendingSuspensionForce = FVector::ZeroVector;
    FVector PendingSuspensionForceLocation = FVector::ZeroVector;
    bool bHasPendingSuspensionForce = false;

    float PreviousSpringLength = 45.0f;
    bool bHadGroundContactLastFrame = false;
};
