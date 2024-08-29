#include "HWTraceUtils.h"

bool HWTraceUtils::SweepCapsuleSingleByChannel(const UWorld* World, FHitResult& OutHit, const FVector& Start,
	const FVector& End, float CapsuleRadius, float CapsuleHalfHeight, const FQuat& Rot, ECollisionChannel TraceChannel,
	const FCollisionQueryParams& Params, const FCollisionResponseParams& ResponseParam, bool DrawDebug, float DrawTime,
	FColor TraceColor, FColor HitColor)
{
	const FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);
	bool bResult = World->SweepSingleByChannel(OutHit, Start, End, Rot, TraceChannel, CollisionShape, Params, ResponseParam);

#if ENABLE_DRAW_DEBUG
	if (DrawDebug)
	{
		DrawDebugCapsule(World, Start, CapsuleHalfHeight, CapsuleRadius, Rot, TraceColor, false, DrawTime);
		DrawDebugCapsule(World, End, CapsuleHalfHeight, CapsuleRadius, Rot, TraceColor, false, DrawTime);
		DrawDebugLine(World, Start, End, TraceColor, false, DrawTime);
		if (bResult)
		{
			DrawDebugCapsule(World, OutHit.Location, CapsuleHalfHeight, CapsuleRadius, Rot, HitColor, false, DrawTime);
			DrawDebugPoint(World, OutHit.ImpactPoint, 10.0f, HitColor, false, DrawTime);
		}
	}
#endif
	return bResult;
}

bool HWTraceUtils::SweepSphereSingleByChannel(const UWorld* World, FHitResult& OutHit, const FVector& Start,
	const FVector& End, float Radius, ECollisionChannel TraceChannel, const FCollisionQueryParams& Params,
	const FCollisionResponseParams& ResponseParam, bool DrawDebug, float DrawTime, FColor TraceColor,
	FColor HitColor)
{
	FVector DebugDownwardCapsuleLocation = (Start + End) * 0.5f;
	FVector TraceVector = End - Start;
	float DebugDownwardCapsuleHalfHeight = TraceVector.Size() * 0.5f;

	FQuat CapsuleRotation = FRotationMatrix::MakeFromZ(TraceVector).ToQuat();
	
	const FCollisionShape CollisionShape = FCollisionShape::MakeSphere(Radius);
	bool bResult = World->SweepSingleByChannel(OutHit, Start, End, CapsuleRotation, TraceChannel, CollisionShape, Params, ResponseParam);

#if ENABLE_DRAW_DEBUG
	if (DrawDebug)
	{
		DrawDebugCapsule(World, DebugDownwardCapsuleLocation, DebugDownwardCapsuleHalfHeight, Radius, CapsuleRotation, TraceColor, false, DrawTime);
		if (bResult)
		{
			DrawDebugSphere(World, OutHit.Location, Radius, 16, HitColor, false, DrawTime);
			DrawDebugPoint(World, OutHit.ImpactPoint, 10.0f, HitColor, false, DrawTime);
		}
	}
#endif
	return bResult;
}

bool HWTraceUtils::OverlapCapsuleAnyByProfile(const UWorld* World, const FVector& Pos, float CapsuleRadius,
	float CapsuleHalfHeight, const FQuat& Rot, FName ProfileName, const FCollisionQueryParams& Params, bool DrawDebug,
	float DrawTime, FColor HitColor)
{
	FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);

	bool bResult = World->OverlapAnyTestByProfile(Pos, Rot, ProfileName, CollisionShape, Params);

#if ENABLE_DRAW_DEBUG
	if (DrawDebug && bResult)
	{
		DrawDebugCapsule(World, Pos, CapsuleHalfHeight, CapsuleRadius, Rot, HitColor, false, DrawTime);
	}
#endif
	return bResult;
}

bool HWTraceUtils::OverlapCapsuleBlockingByProfile(const UWorld* World, const FVector& Pos, float CapsuleRadius,
	float CapsuleHalfHeight, const FQuat& Rot, FName ProfileName, const FCollisionQueryParams& Params, bool DrawDebug,
	float DrawTime, FColor HitColor)
{
	FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);

	bool bResult = World->OverlapBlockingTestByProfile(Pos, Rot, ProfileName, CollisionShape, Params);

#if ENABLE_DRAW_DEBUG
	if (DrawDebug && bResult)
	{
		DrawDebugCapsule(World, Pos, CapsuleHalfHeight, CapsuleRadius, Rot, HitColor, false, DrawTime);
	}
#endif
	return bResult;
}
