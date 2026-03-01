#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "WheelComponent.generated.h"

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
    float SpringStiffness = 65000.0f;

    UPROPERTY(EditAnywhere, Category = "Suspension")
    float DamperStiffness = 4500.0f;

    UPROPERTY(EditAnywhere, Category = "Suspension")
    float WheelRadius = 18.0f;

    UPROPERTY(EditAnywhere, Category = "Suspension")
    TEnumAsByte<ECollisionChannel> SweepChannel = ECC_WorldStatic;

    UPROPERTY(EditAnywhere, Category = "Setup")
    FName SweepCollisionComponentName = TEXT("SweepCollisionComponent");

    UPROPERTY(EditAnywhere, Category = "Setup")
    FName VisualWheelComponentName = TEXT("VisualWheelMeshComponent");

    UPROPERTY(EditAnywhere, Category = "Setup")
    UStaticMeshComponent* SweepCollisionComponent = nullptr;

    UPROPERTY(EditAnywhere, Category = "Setup")
    UStaticMeshComponent* VisualWheelMeshComponent = nullptr;

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDrawDebug = false;

    UPROPERTY()
    UPrimitiveComponent* Body = nullptr;

    FVector VisualMeshInitialRelativeLocation = FVector::ZeroVector;
};
