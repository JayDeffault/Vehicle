#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
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

private:
    UPROPERTY()
    UStaticMeshComponent* SweepCollisionComponent = nullptr;

    UPROPERTY()
    UStaticMeshComponent* VisualWheelMeshComponent = nullptr;

    UPROPERTY()
    UStaticMeshComponent* Body = nullptr;
};
