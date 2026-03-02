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

private:
    void InitializeWheelComponents();
    bool PerformSuspensionSweep(FHitResult& OutBestHit, FVector& OutStart, FVector& OutEnd) const;

private:
    UPROPERTY(EditAnywhere, Category = "Suspension")
    float SuspensionLength = 45.0f;

    UPROPERTY(EditAnywhere, Category = "Suspension")
    float SpringStiffness = 42000.0f;

    UPROPERTY(EditAnywhere, Category = "Suspension")
    float DamperStiffness = 14000.0f;

    UPROPERTY(EditAnywhere, Category = "Suspension")
    float WheelRadius = 18.0f;

    UPROPERTY(EditAnywhere, Category = "Suspension")
    float MaxSuspensionForce = 80000.0f;


    UPROPERTY(EditAnywhere, Category = "Wheel")
    UStaticMeshComponent* SweepCollisionComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wheel", meta = (AllowPrivateAccess = "true"))
    UStaticMesh* CollisionShape = nullptr;

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDrawDebug = false;

    UPROPERTY()
    UPrimitiveComponent* Body = nullptr;

};
