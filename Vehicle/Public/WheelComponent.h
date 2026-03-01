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

    UStaticMeshComponent* SweepCollisionComponent;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wheel")
    UStaticMesh* CollisionShape;

    UStaticMeshComponent* VisualWheelMeshComponent;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wheel")
    UStaticMesh* VisualWheelMesh;

    UStaticMeshComponent* Body;

    void CalculateSweep();
    void CalculatePhysics(float DeltaTime);
    void VisualUpdate();

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
    float WheelRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension", meta=(ClampMin = -30, ClampMax = 30))
    float TopSpringOffset = FMath::Clamp(TopSpringOffset, -30, 30);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
    float SpringLength = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
    float SpringStiffness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
    float SpringDamping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
    bool bEnableDebugMode;

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