#pragma once

#include "CoreMinimal.h"
#include "animDataStruct.generated.h"

USTRUCT()
struct FValue {
	GENERATED_BODY()
public:

	TArray<FVector> PosKeys;
	TArray<FRotator> RotKeys;
	TArray<FVector> ScaleKeys;
};
USTRUCT()
struct FCamTrack {
	GENERATED_BODY()
public:
	FName Key;
	FValue Value;
};
USTRUCT()
struct FAnimDataTrack {
	GENERATED_BODY()
	TArray<FCamTrack> Tracks;
};

