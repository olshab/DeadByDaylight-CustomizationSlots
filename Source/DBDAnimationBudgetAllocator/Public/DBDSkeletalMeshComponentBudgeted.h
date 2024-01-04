#pragma once

#include "CoreMinimal.h"
#include "SkeletalMeshComponentBudgeted.h"
#include "DBDSkeletalMeshComponentBudgeted.generated.h"

class USkeletalMeshComponent;

UCLASS(EditInlineNew, meta=(BlueprintSpawnableComponent))
class DBDANIMATIONBUDGETALLOCATOR_API UDBDSkeletalMeshComponentBudgeted : public USkeletalMeshComponentBudgeted
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly)
	bool _autoRegisterToAnimBudgeter;

	UPROPERTY(EditDefaultsOnly)
	bool _autoComputeSignificance;

public:
	UFUNCTION(BlueprintCallable)
	void SetAnimationDependency(USkeletalMeshComponent* meshDependency);

public:
	UDBDSkeletalMeshComponentBudgeted();
};

FORCEINLINE uint32 GetTypeHash(const UDBDSkeletalMeshComponentBudgeted) { return 0; }
