// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "SimplexNoiseLibrary.generated.h"

// Credit where credit is due!
// most of this is ripped direct from https://github.com/devdad/SimplexNoise
// if you get around to reading this, pay them some love somehow, they deserve it.
//
// TODO : Remember to mention in credits.


UCLASS()
class MPHORSO_API USimplexNoiseLibrary : public UObject
{
	GENERATED_BODY()
	
private:

	static unsigned char perm[];
	static float  grad(int hash, float x);
	static float  grad(int hash, float x, float y);
	static float  grad(int hash, float x, float y, float z);
	static float  grad(int hash, float x, float y, float z, float t);

public:

	UFUNCTION(BlueprintCallable, Category = "SimplexNoise")
		static void setNoiseSeed(const int32& newSeed);

	UFUNCTION(BlueprintCallable, Category = "SimplexNoise")
		static float SimplexNoise1D(float x);

	UFUNCTION(BlueprintCallable, Category = "SimplexNoise")
		static float SimplexNoise2D(float x, float y);

	UFUNCTION(BlueprintCallable, Category = "SimplexNoise")
		static float SimplexNoise3D(float x, float y, float z);

	UFUNCTION(BlueprintCallable, Category = "SimplexNoise")
		static float SimplexNoise4D(float x, float y, float z, float w);

	// Scaled by float value
	UFUNCTION(BlueprintCallable, Category = "SimplexNoise")
		static float SimplexNoiseScaled1D(float x, float s);

	UFUNCTION(BlueprintCallable, Category = "SimplexNoise")
		static float SimplexNoiseScaled2D(float x, float y, float s);

	UFUNCTION(BlueprintCallable, Category = "SimplexNoise")
		static float SimplexNoiseScaled3D(float x, float y, float z, float s);

	UFUNCTION(BlueprintCallable, Category = "SimplexNoise")
		static float SimplexNoiseScaled4D(float x, float y, float z, float w, float s);

	// Return value in Range between two float numbers
	// Return Value is scaled by difference between rangeMin & rangeMax value

	UFUNCTION(BlueprintCallable, Category = "SimplexNoise")
		static float SimplexNoiseInRange1D(float x, float rangeMin, float rangeMax);

	UFUNCTION(BlueprintCallable, Category = "SimplexNoise")
		static float SimplexNoiseInRange2D(float x, float y, float rangeMin, float rangeMax);

	UFUNCTION(BlueprintCallable, Category = "SimplexNoise")
		static float SimplexNoiseInRange3D(float x, float y, float z, float rangeMin, float rangeMax);

	UFUNCTION(BlueprintCallable, Category = "SimplexNoise")
		static float SimplexNoiseInRange4D(float x, float y, float z, float w, float rangeMin, float rangeMax);



	UFUNCTION(BlueprintCallable, Category = "SimplexNoise")
		static float OctaveSimplex1D(float x, float octaves, float persistence);

	UFUNCTION(BlueprintCallable, Category = "SimplexNoise")
		static float OctaveSimplex2D(float x, float y, float octaves, float persistence);

	UFUNCTION(BlueprintCallable, Category = "SimplexNoise")
		static float OctaveSimplex3D(float x, float y, float z, float octaves, float persistence);

	UFUNCTION(BlueprintCallable, Category = "SimplexNoise")
		static float OctaveSimplex4D(float x, float y, float z, float w, float octaves, float persistence);

};
