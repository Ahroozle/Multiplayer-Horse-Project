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


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Constant(float val);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
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

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Add(FANL_II s1Index, FANL_II s2Index);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Subtract(FANL_II s1, FANL_II s2);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Multiply(FANL_II s1Index, FANL_II s2Index);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Divide(FANL_II s1, FANL_II s2);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Maximum(FANL_II s1Index, FANL_II s2Index);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Minimum(FANL_II s1Index, FANL_II s2Index);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Abs(FANL_II sIndex);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Pow(FANL_II s1, FANL_II s2);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Bias(FANL_II s1, FANL_II s2);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Gain(FANL_II s1, FANL_II s2);


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_ScaleDomain(FANL_II srcIndex, FANL_II scale);


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_ScaleX(FANL_II src, FANL_II scale);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_ScaleY(FANL_II src, FANL_II scale);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_ScaleZ(FANL_II src, FANL_II scale);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_ScaleW(FANL_II src, FANL_II scale);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_ScaleU(FANL_II src, FANL_II scale);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_ScaleV(FANL_II src, FANL_II scale);


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_TranslateDomain(FANL_II srcIndex, FANL_II trans);


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_TranslateX(FANL_II src, FANL_II trans);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_TranslateY(FANL_II src, FANL_II trans);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_TranslateZ(FANL_II src, FANL_II trans);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_TranslateW(FANL_II src, FANL_II trans);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_TranslateU(FANL_II src, FANL_II trans);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_TranslateV(FANL_II src, FANL_II trans);


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_RotateRomain(FANL_II src, FANL_II angle, FANL_II ax, FANL_II ay, FANL_II az);


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_AddSequence(FANL_II baseIndex, int number, int stride);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_MultiplySequence(FANL_II baseIndex, int number, int stride);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_MaxSequence(FANL_II baseIndex, int number, int stride);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_MinSequence(FANL_II baseIndex, int number, int stride);


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Blend(FANL_II low, FANL_II high, FANL_II control);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Select(FANL_II low, FANL_II high, FANL_II control, FANL_II threshold, FANL_II falloff);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Clamp(FANL_II src, FANL_II low, FANL_II high);


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Cos(FANL_II src);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Sin(FANL_II src);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Tan(FANL_II src);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Acos(FANL_II src);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Asin(FANL_II src);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Atan(FANL_II src);


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Tiers(FANL_II src, FANL_II numTiers);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_SmoothTiers(FANL_II src, FANL_II numTiers);


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_X();

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Y();

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Z();

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_W();

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_U();

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_V();


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_dX(FANL_II src, FANL_II spacing);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_dY(FANL_II src, FANL_II spacing);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_dZ(FANL_II src, FANL_II spacing);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_dW(FANL_II src, FANL_II spacing);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_dU(FANL_II src, FANL_II spacing);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_dV(FANL_II src, FANL_II spacing);


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Sigmoid(FANL_II src);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_SigmoidDetailed(FANL_II src, FANL_II center, FANL_II ramp);


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Radial();


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_HexTile(FANL_II seed);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_HexBump();


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Color(FLinearColor c);


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_CombineRGBA(FANL_II r, FANL_II g, FANL_II b, FANL_II a);


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_ScaleOffset(FANL_II src, float scale, float offset);


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_SimpleFractalLayer(int basisType, FANL_II interpTypeIndex, float layerScale, float layerFreq, int seed,
											  bool rot = true, float angle = 0.5f, float ax = 0, float ay = 0, float az = 1);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_SimpleRidgedLayer(int basisType, FANL_II interpTypeIndex, float layerScale, float layerFreq, int seed,
											 bool rot = true, float angle = 0.5f, float ax = 0, float ay = 0, float az = 1);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_SimpleBillowLayer(int basisType, FANL_II interpTypeIndex, float layerScale, float layerFreq, int seed,
											 bool rot = true, float angle = 0.5f, float ax = 0, float ay = 0, float az = 1);


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_Simple_fBm(int basisType, int interpType, int octaves, float freq, int seed, bool rot = true);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_SimpleRidgedMultifractal(int basisType, int interpType, int octaves, float freq, int seed, bool rot = true);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_SimpleBillow(int basisType, int interpType, int octaves, float freq, int seed, bool rot = true);


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_NextIndex();

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_LastIndex();


	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static void ANL_SetVar(FString name, float val);

	UFUNCTION(BlueprintCallable, Category = "ANL Simplification Facade Function Library")
		static FANL_II ANL_GetVar(FString name);


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
