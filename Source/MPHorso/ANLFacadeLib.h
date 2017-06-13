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
USTRUCT(BlueprintType)
struct FANL_InstructionIndex
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
		void ResetNoiseKernel();


	// Blueprint-friendly overloads for all of the possible CInstructionIndex operations.

	UFUNCTION(BlueprintPure, meta = (DisplayName = "instructionindex + instructionindex", CompactNodeTitle = "+", Keywords = "+ add plus", CommutativeAssociativeBinaryOperator = "true"), Category = "ANL Simplification Facade Function Library")
		static FANL_InstructionIndex Add_InsInds(FANL_InstructionIndex a, FANL_InstructionIndex b);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "instructionindex + int", CompactNodeTitle = "+", Keywords = "+ add plus", CommutativeAssociativeBinaryOperator = "true"), Category = "ANL Simplification Facade Function Library")
		static FANL_InstructionIndex Add_InsInd_Int(FANL_InstructionIndex a, int32 b);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "instructionindex - instructionindex", CompactNodeTitle = "-", Keywords = "- subtract minus"), Category = "ANL Simplification Facade Function Library")
		static FANL_InstructionIndex Sub_InsInds(FANL_InstructionIndex a, FANL_InstructionIndex b);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "instructionindex - int", CompactNodeTitle = "-", Keywords = "- subtract minus"), Category = "ANL Simplification Facade Function Library")
		static FANL_InstructionIndex Sub_InsInd_Int(FANL_InstructionIndex a, int32 b);


private:

	static anl::CKernel Kernel;
	static anl::CNoiseExecutor VM;

};
