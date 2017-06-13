// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "ANLFacadeLib.h"

using namespace anl;

// ARE YOU READY FOR SOME ~*~**~***INTENSE CHEATARONI***~**~*~
CInstructionIndex* InstructionCast(FANL_InstructionIndex& FromBP) { return (CInstructionIndex*)(&FromBP); } // this can only end in tears

CKernel UANLFacadeLib::Kernel;
CNoiseExecutor UANLFacadeLib::VM(Kernel);

void UANLFacadeLib::ResetNoiseKernel()
{
	Kernel.getKernel()->clear();
}

FANL_InstructionIndex UANLFacadeLib::Add_InsInds(FANL_InstructionIndex a, FANL_InstructionIndex b) { return { (a.instructionIndex + b.instructionIndex) }; }
FANL_InstructionIndex UANLFacadeLib::Add_InsInd_Int(FANL_InstructionIndex a, int32 b) { return { (a.instructionIndex + b) }; }
FANL_InstructionIndex UANLFacadeLib::Sub_InsInds(FANL_InstructionIndex a, FANL_InstructionIndex b) { return { (a.instructionIndex - b.instructionIndex) }; }
FANL_InstructionIndex UANLFacadeLib::Sub_InsInd_Int(FANL_InstructionIndex a, int32 b) { return { (a.instructionIndex - b) }; }

