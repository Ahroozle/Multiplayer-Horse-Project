// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"

#include "VM/kernel.h"

#include "ANLFacadeLib.generated.h"

/*
	This is a wrapper structure for passing
	ANL Instruction Indices around in
	blueprints!

	NOTE: There aren't any variables to see
	by breaking this.
*/
USTRUCT(BlueprintType, meta = (DisplayName = "ANL Instruction Index"))
struct FANL_II
{
	GENERATED_USTRUCT_BODY();

	uint32 instructionIndex;
};

USTRUCT(BlueprintType, meta = (DisplayName = "ANL VM Output"))
struct FANL_VMOut
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(BlueprintReadOnly)
		float ResultFloat;

	UPROPERTY(BlueprintReadOnly)
		FLinearColor ResultColor;

};

UENUM(BlueprintType)
enum class EANL_Axis : uint8
{
	X,Y,Z,W,U,V
};

UENUM(BlueprintType)
enum class EANL_InterpType : uint8
{
	INTERP_NONE			UMETA(DisplayName = "No Interpolation"),
	INTERP_LINEAR		UMETA(DisplayName = "Linear Interpolation"),
	INTERP_HERMITE		UMETA(DisplayName = "Hermite Interpolation"),
	INTERP_QUINTIC		UMETA(DisplayName = "Quintic Interpolation")
};

// NOTE: iirc looking at the code greatestaxis/leastaxis are actually backwards in the enum
//		 so maybe I should switch them in this one?
UENUM(BlueprintType)
enum class EANL_DistType : uint8
{
	DISTANCE_EUCLID			UMETA(DisplayName = "Euclidean Distance"),
	DISTANCE_MANHATTAN		UMETA(DisplayName = "Manhattan Distance"),
	DISTANCE_LEASTAXIS		UMETA(DisplayName = "Least-Axis Distance"),
	DISTANCE_GREATESTAXIS	UMETA(DisplayName = "Greatest-Axis Distance")
};

UENUM(BlueprintType)
enum class EANL_BasisType : uint8
{
	BASIS_VALUE			UMETA(DisplayName = "Value Basis"),
	BASIS_GRADIENT		UMETA(DisplayName = "Gradient Basis"),
	BASIS_SIMPLEX		UMETA(DisplayName = "Simplex Basis")
};


UCLASS()
class MPHORSO_API UANLFacadeLib : public UObject
{
	GENERATED_BODY()
	
public:
	
	// One-And-Done functions; These don't affect the static kernel or vm



	// Build-up functions; these affect the static kernel and vm and are
	// here so that the excellent procedure-building functionality of ANL
	// is preserved.

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static void ANL_ResetNoiseKernel();

	// Gets the CInstructionIndex of constant Pi
	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_GetConstant_Pi();

	// Gets the CInstructionIndex of constant e
	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_GetConstant_e();

	// Gets the CInstructionIndex of constant 1
	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_GetConstant_One();

	// Gets the CInstructionIndex of constant 0
	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_GetConstant_Zero();

	// Gets the CInstructionIndex of constant 0.5
	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_GetConstant_Half();

	// Gets the CInstructionIndex of constant sqrt(2)
	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_GetConstant_RootTwo();


	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Constant(float val);

	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Seed(int val);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_ValueBasis(FANL_II interpIndex, FANL_II seed);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_GradientBasis(FANL_II interpIndex, FANL_II seed);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_SimplexBasis(FANL_II seed);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_CellularBasis(FANL_II f1, FANL_II f2, FANL_II f3, FANL_II f4,
										 FANL_II d1, FANL_II d2, FANL_II d3, FANL_II d4,
										 FANL_II dist, FANL_II seed);

	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Add(FANL_II s1Index, FANL_II s2Index);

	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Subtract(FANL_II s1, FANL_II s2);

	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Multiply(FANL_II s1Index, FANL_II s2Index);

	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Divide(FANL_II s1, FANL_II s2);

	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Maximum(FANL_II s1Index, FANL_II s2Index);

	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Minimum(FANL_II s1Index, FANL_II s2Index);

	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Abs(FANL_II sIndex);

	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Pow(FANL_II s1, FANL_II s2);

	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Bias(FANL_II s1, FANL_II s2);

	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Gain(FANL_II s1, FANL_II s2);


	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_ScaleDomain(FANL_II srcIndex, FANL_II scale);


	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Scale(EANL_Axis Axis, FANL_II src, FANL_II scale);


	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_TranslateDomain(FANL_II srcIndex, FANL_II trans);


	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Translate(EANL_Axis Axis, FANL_II src, FANL_II trans);


	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_RotateRomain(FANL_II src, FANL_II angle, FANL_II ax, FANL_II ay, FANL_II az);


	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_AddSequence(FANL_II baseIndex, int number, int stride);

	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_MultiplySequence(FANL_II baseIndex, int number, int stride);

	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_MaxSequence(FANL_II baseIndex, int number, int stride);

	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_MinSequence(FANL_II baseIndex, int number, int stride);


	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Blend(FANL_II low, FANL_II high, FANL_II control);

	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Select(FANL_II low, FANL_II high, FANL_II control, FANL_II threshold, FANL_II falloff);

	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Clamp(FANL_II src, FANL_II low, FANL_II high);


	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Cos(FANL_II src);

	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Sin(FANL_II src);

	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Tan(FANL_II src);

	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Acos(FANL_II src);

	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Asin(FANL_II src);

	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Atan(FANL_II src);


	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Tiers(FANL_II src, FANL_II numTiers);

	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_SmoothTiers(FANL_II src, FANL_II numTiers);


	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Axis(EANL_Axis Axis);


	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_dAxis(EANL_Axis Axis, FANL_II src, FANL_II spacing);


	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Sigmoid(FANL_II src);

	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_SigmoidDetailed(FANL_II src, FANL_II center, FANL_II ramp);


	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Radial();


	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_HexTile(FANL_II seed);

	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_HexBump();


	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Color(FLinearColor c);


	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_CombineRGBA(FANL_II r, FANL_II g, FANL_II b, FANL_II a);


	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_ScaleOffset(FANL_II src, float scale, float offset);


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_SimpleFractalLayer(EANL_BasisType basisType, FANL_II interpTypeIndex, float layerScale, float layerFreq, int seed,
											  bool rot = true, float angle = 0.5f, float ax = 0, float ay = 0, float az = 1);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_SimpleRidgedLayer(EANL_BasisType basisType, FANL_II interpTypeIndex, float layerScale, float layerFreq, int seed,
											 bool rot = true, float angle = 0.5f, float ax = 0, float ay = 0, float az = 1);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_SimpleBillowLayer(EANL_BasisType basisType, FANL_II interpTypeIndex, float layerScale, float layerFreq, int seed,
											 bool rot = true, float angle = 0.5f, float ax = 0, float ay = 0, float az = 1);


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Simple_fBm(EANL_BasisType basisType, EANL_InterpType interpType,
									  int octaves, float freq, int seed, bool rot = true);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_SimpleRidgedMultifractal(EANL_BasisType basisType, EANL_InterpType interpType,
													int octaves, float freq, int seed, bool rot = true);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_SimpleBillow(EANL_BasisType basisType, EANL_InterpType interpType,
										int octaves, float freq, int seed, bool rot = true);


	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_NextIndex();

	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_LastIndex();


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static void ANL_SetVar(FString name, float val);

	UFUNCTION(BlueprintPure, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_GetVar(FString name);


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_VMOut ANL_Evaluate(int Dimension, float x, float y, float z, float w, float u, float v);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_VMOut ANL_EvaluateAt(int Dimension, float x, float y, float z, float w, float u, float v, FANL_II index);


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static float ANL_EvaluateScalar(int Dimension, float x, float y, float z, float w, float u, float v, FANL_II idx);


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FLinearColor ANL_EvaluateColor(int Dimension, float x, float y, float z, float w, float u, float v, FANL_II idx);


	// Blueprint-friendly overloads for all of the possible CInstructionIndex operations.

	UFUNCTION(BlueprintPure, meta = (DisplayName = "instructionindex + instructionindex", CompactNodeTitle = "+", Keywords = "+ add plus", CommutativeAssociativeBinaryOperator = "true"), Category = "ANL Simplification Facade Function Library")
		static FANL_II Add_InsInds(FANL_II a, FANL_II b);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "instructionindex + int", CompactNodeTitle = "+", Keywords = "+ add plus", CommutativeAssociativeBinaryOperator = "true"), Category = "ANL Simplification Facade Function Library")
		static FANL_II Add_InsInd_Int(FANL_II a, int32 b);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "instructionindex - instructionindex", CompactNodeTitle = "-", Keywords = "- subtract minus"), Category = "ANL Simplification Facade Function Library")
		static FANL_II Sub_InsInds(FANL_II a, FANL_II b);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "instructionindex - int", CompactNodeTitle = "-", Keywords = "- subtract minus"), Category = "ANL Simplification Facade Function Library")
		static FANL_II Sub_InsInd_Int(FANL_II a, int32 b);


private:

	static anl::CKernel Kernel;
	static anl::CNoiseExecutor VM;

};
