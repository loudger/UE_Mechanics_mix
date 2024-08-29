#include "HWDebugUtils.h"


void HWDebugUtils::DebugFVector(FVector Vector, FString DebugMessage, int32 DebugKey, FColor DebugColor, float DebugTime)
{
	GEngine->AddOnScreenDebugMessage(DebugKey, DebugTime, DebugColor, FString::Printf(TEXT("%s - X=%f Y=%f Z=%f"), *DebugMessage, Vector.X, Vector.Y, Vector.Z));
}
