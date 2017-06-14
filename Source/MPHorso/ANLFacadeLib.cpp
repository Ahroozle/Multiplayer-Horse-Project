// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "ANLFacadeLib.h"


using namespace anl;

/*
					IT'S THE

	/---------------------------------------\
	|                                       |
	|       qUEstIONaBlE CAsTING CLUb       |
	|                                       |
	\---------------------------------------/
					   | |
					   | |
					   | |
			 \/\\/\/\///\\//\\/\//\/\
*/

// ARE YOU READY FOR SOME ~*~**~***INTENSE CHEATARONI***~**~*~
CInstructionIndex ToCIns(FANL_II& FromBP) { return *((CInstructionIndex*)(&FromBP)); } // this can only end in tears

// remember kids don't try this at home
// try it at a friend's home instead
FANL_II ToBPIns(CInstructionIndex insind) { return { *((uint32*)&insind) }; }

FANL_VMOut ToBPOut(SVMOutput vmo) { return { (float)(vmo.outfloat_), *((FLinearColor*)(&vmo.outrgba_)) }; }


// One-And-Dones



// Build-ups

CKernel UANLFacadeLib::Kernel;
CNoiseExecutor UANLFacadeLib::VM(Kernel);

void UANLFacadeLib::ANL_ResetNoiseKernel() { Kernel.getKernel()->clear(); }

FANL_II UANLFacadeLib::ANL_GetConstant_Pi()			{ return ToBPIns( Kernel.pi() ); }
FANL_II UANLFacadeLib::ANL_GetConstant_e()			{ return ToBPIns( Kernel.e() ); }
FANL_II UANLFacadeLib::ANL_GetConstant_One()		{ return ToBPIns( Kernel.one() ); }
FANL_II UANLFacadeLib::ANL_GetConstant_Zero()		{ return ToBPIns( Kernel.zero() ); }
FANL_II UANLFacadeLib::ANL_GetConstant_Half()		{ return ToBPIns( Kernel.point5() ); }
FANL_II UANLFacadeLib::ANL_GetConstant_RootTwo()	{ return ToBPIns( Kernel.sqrt2() ); }


FANL_II UANLFacadeLib::ANL_Constant(float val)		{ return ToBPIns( Kernel.constant(val)		); }
FANL_II UANLFacadeLib::ANL_Seed(int val)			{ return ToBPIns( Kernel.seed((uint32)val)	); }

FANL_II UANLFacadeLib::ANL_ValueBasis(FANL_II interpIndex, FANL_II seed)
{
	return ToBPIns(Kernel.valueBasis(ToCIns(interpIndex), ToCIns(seed)));
}

FANL_II UANLFacadeLib::ANL_GradientBasis(FANL_II interpIndex, FANL_II seed)
{
	return ToBPIns(Kernel.gradientBasis(ToCIns(interpIndex), ToCIns(seed)));
}

FANL_II UANLFacadeLib::ANL_SimplexBasis(FANL_II seed) { return ToBPIns(Kernel.simplexBasis(ToCIns(seed))); }

FANL_II UANLFacadeLib::ANL_CellularBasis(FANL_II f1, FANL_II f2, FANL_II f3, FANL_II f4,
										 FANL_II d1, FANL_II d2, FANL_II d3, FANL_II d4,
										 FANL_II dist, FANL_II seed)
{
	return ToBPIns(Kernel.cellularBasis(ToCIns(f1), ToCIns(f2), ToCIns(f3), ToCIns(f4), 
										ToCIns(d1), ToCIns(d2), ToCIns(d3), ToCIns(d4), 
										ToCIns(dist), ToCIns(seed)));
}

FANL_II UANLFacadeLib::ANL_Add(FANL_II s1Index, FANL_II s2Index)		{ return ToBPIns(Kernel.add(ToCIns(s1Index), ToCIns(s2Index))); }
FANL_II UANLFacadeLib::ANL_Subtract(FANL_II s1, FANL_II s2)				{ return ToBPIns(Kernel.subtract(ToCIns(s1), ToCIns(s2))); }
FANL_II UANLFacadeLib::ANL_Multiply(FANL_II s1Index, FANL_II s2Index)	{ return ToBPIns(Kernel.multiply(ToCIns(s1Index), ToCIns(s2Index))); }
FANL_II UANLFacadeLib::ANL_Divide(FANL_II s1, FANL_II s2)				{ return ToBPIns(Kernel.divide(ToCIns(s1), ToCIns(s2))); }
FANL_II UANLFacadeLib::ANL_Maximum(FANL_II s1Index, FANL_II s2Index)	{ return ToBPIns(Kernel.maximum(ToCIns(s1Index), ToCIns(s2Index))); }
FANL_II UANLFacadeLib::ANL_Minimum(FANL_II s1Index, FANL_II s2Index)	{ return ToBPIns(Kernel.minimum(ToCIns(s1Index), ToCIns(s2Index))); }
FANL_II UANLFacadeLib::ANL_Abs(FANL_II sIndex)							{ return ToBPIns(Kernel.abs(ToCIns(sIndex))); }
FANL_II UANLFacadeLib::ANL_Pow(FANL_II s1, FANL_II s2)					{ return ToBPIns(Kernel.pow(ToCIns(s1), ToCIns(s2))); }
FANL_II UANLFacadeLib::ANL_Bias(FANL_II s1, FANL_II s2)					{ return ToBPIns(Kernel.bias(ToCIns(s1), ToCIns(s2))); }
FANL_II UANLFacadeLib::ANL_Gain(FANL_II s1, FANL_II s2)					{ return ToBPIns(Kernel.gain(ToCIns(s1), ToCIns(s2))); }


FANL_II UANLFacadeLib::ANL_ScaleDomain(FANL_II srcIndex, FANL_II scale)
{
	return ToBPIns(Kernel.scaleDomain(ToCIns(srcIndex), ToCIns(scale)));
}


FANL_II UANLFacadeLib::ANL_ScaleX(FANL_II src, FANL_II scale)	{ return ToBPIns(Kernel.scaleX(ToCIns(src), ToCIns(scale))); }
FANL_II UANLFacadeLib::ANL_ScaleY(FANL_II src, FANL_II scale)	{ return ToBPIns(Kernel.scaleY(ToCIns(src), ToCIns(scale))); }
FANL_II UANLFacadeLib::ANL_ScaleZ(FANL_II src, FANL_II scale)	{ return ToBPIns(Kernel.scaleZ(ToCIns(src), ToCIns(scale))); }
FANL_II UANLFacadeLib::ANL_ScaleW(FANL_II src, FANL_II scale)	{ return ToBPIns(Kernel.scaleW(ToCIns(src), ToCIns(scale))); }
FANL_II UANLFacadeLib::ANL_ScaleU(FANL_II src, FANL_II scale)	{ return ToBPIns(Kernel.scaleU(ToCIns(src), ToCIns(scale))); }
FANL_II UANLFacadeLib::ANL_ScaleV(FANL_II src, FANL_II scale)	{ return ToBPIns(Kernel.scaleV(ToCIns(src), ToCIns(scale))); }


FANL_II UANLFacadeLib::ANL_TranslateDomain(FANL_II srcIndex, FANL_II trans)
{
	return ToBPIns(Kernel.translateDomain(ToCIns(srcIndex), ToCIns(trans)));
}


FANL_II UANLFacadeLib::ANL_TranslateX(FANL_II src, FANL_II trans)	{ return ToBPIns(Kernel.translateX(ToCIns(src), ToCIns(trans))); }
FANL_II UANLFacadeLib::ANL_TranslateY(FANL_II src, FANL_II trans)	{ return ToBPIns(Kernel.translateY(ToCIns(src), ToCIns(trans))); }
FANL_II UANLFacadeLib::ANL_TranslateZ(FANL_II src, FANL_II trans)	{ return ToBPIns(Kernel.translateZ(ToCIns(src), ToCIns(trans))); }
FANL_II UANLFacadeLib::ANL_TranslateW(FANL_II src, FANL_II trans)	{ return ToBPIns(Kernel.translateW(ToCIns(src), ToCIns(trans))); }
FANL_II UANLFacadeLib::ANL_TranslateU(FANL_II src, FANL_II trans)	{ return ToBPIns(Kernel.translateU(ToCIns(src), ToCIns(trans))); }
FANL_II UANLFacadeLib::ANL_TranslateV(FANL_II src, FANL_II trans)	{ return ToBPIns(Kernel.translateV(ToCIns(src), ToCIns(trans))); }


FANL_II UANLFacadeLib::ANL_RotateRomain(FANL_II src, FANL_II angle, FANL_II ax, FANL_II ay, FANL_II az)
{
	return ToBPIns(Kernel.rotateDomain(ToCIns(src), ToCIns(angle), ToCIns(ax), ToCIns(ay), ToCIns(az)));
}


FANL_II UANLFacadeLib::ANL_AddSequence(FANL_II baseIndex, int number, int stride)
{
	return ToBPIns(Kernel.addSequence(ToCIns(baseIndex), (uint32)number, (uint32)stride));
}

FANL_II UANLFacadeLib::ANL_MultiplySequence(FANL_II baseIndex, int number, int stride)
{
	return ToBPIns(Kernel.multiplySequence(ToCIns(baseIndex), (uint32)number, (uint32)stride));
}

FANL_II UANLFacadeLib::ANL_MaxSequence(FANL_II baseIndex, int number, int stride)
{
	return ToBPIns(Kernel.maxSequence(ToCIns(baseIndex), (uint32)number, (uint32)stride));
}

FANL_II UANLFacadeLib::ANL_MinSequence(FANL_II baseIndex, int number, int stride)
{
	return ToBPIns(Kernel.minSequence(ToCIns(baseIndex), (uint32)number, (uint32)stride));
}


FANL_II UANLFacadeLib::ANL_Blend(FANL_II low, FANL_II high, FANL_II control)
{
	return ToBPIns(Kernel.blend(ToCIns(low), ToCIns(high), ToCIns(control)));
}

FANL_II UANLFacadeLib::ANL_Select(FANL_II low, FANL_II high, FANL_II control, FANL_II threshold, FANL_II falloff)
{
	return ToBPIns(Kernel.select(ToCIns(low), ToCIns(high), ToCIns(control), ToCIns(threshold), ToCIns(falloff)));
}

FANL_II UANLFacadeLib::ANL_Clamp(FANL_II src, FANL_II low, FANL_II high)
{
	return ToBPIns(Kernel.blend(ToCIns(src), ToCIns(low), ToCIns(high)));
}


FANL_II UANLFacadeLib::ANL_Cos(FANL_II src)		{ return ToBPIns(Kernel.cos(ToCIns(src))); }
FANL_II UANLFacadeLib::ANL_Sin(FANL_II src)		{ return ToBPIns(Kernel.sin(ToCIns(src))); }
FANL_II UANLFacadeLib::ANL_Tan(FANL_II src)		{ return ToBPIns(Kernel.tan(ToCIns(src))); }
FANL_II UANLFacadeLib::ANL_Acos(FANL_II src)	{ return ToBPIns(Kernel.acos(ToCIns(src))); }
FANL_II UANLFacadeLib::ANL_Asin(FANL_II src)	{ return ToBPIns(Kernel.asin(ToCIns(src))); }
FANL_II UANLFacadeLib::ANL_Atan(FANL_II src)	{ return ToBPIns(Kernel.atan(ToCIns(src))); }


FANL_II UANLFacadeLib::ANL_Tiers(FANL_II src, FANL_II numTiers)			{ return ToBPIns(Kernel.tiers(ToCIns(src), ToCIns(numTiers))); }
FANL_II UANLFacadeLib::ANL_SmoothTiers(FANL_II src, FANL_II numTiers)	{ return ToBPIns(Kernel.smoothTiers(ToCIns(src), ToCIns(numTiers))); }


FANL_II UANLFacadeLib::ANL_X()	{ return ToBPIns(Kernel.x()); }
FANL_II UANLFacadeLib::ANL_Y()	{ return ToBPIns(Kernel.y()); }
FANL_II UANLFacadeLib::ANL_Z()	{ return ToBPIns(Kernel.z()); }
FANL_II UANLFacadeLib::ANL_W()	{ return ToBPIns(Kernel.w()); }
FANL_II UANLFacadeLib::ANL_U()	{ return ToBPIns(Kernel.u()); }
FANL_II UANLFacadeLib::ANL_V()	{ return ToBPIns(Kernel.v()); }


FANL_II UANLFacadeLib::ANL_dX(FANL_II src, FANL_II spacing)	{ return ToBPIns(Kernel.dx(ToCIns(src), ToCIns(spacing))); }
FANL_II UANLFacadeLib::ANL_dY(FANL_II src, FANL_II spacing)	{ return ToBPIns(Kernel.dy(ToCIns(src), ToCIns(spacing))); }
FANL_II UANLFacadeLib::ANL_dZ(FANL_II src, FANL_II spacing)	{ return ToBPIns(Kernel.dz(ToCIns(src), ToCIns(spacing))); }
FANL_II UANLFacadeLib::ANL_dW(FANL_II src, FANL_II spacing)	{ return ToBPIns(Kernel.dw(ToCIns(src), ToCIns(spacing))); }
FANL_II UANLFacadeLib::ANL_dU(FANL_II src, FANL_II spacing)	{ return ToBPIns(Kernel.du(ToCIns(src), ToCIns(spacing))); }
FANL_II UANLFacadeLib::ANL_dV(FANL_II src, FANL_II spacing)	{ return ToBPIns(Kernel.dv(ToCIns(src), ToCIns(spacing))); }


FANL_II UANLFacadeLib::ANL_Sigmoid(FANL_II src)		{ return ToBPIns(Kernel.sigmoid(ToCIns(src))); }

FANL_II UANLFacadeLib::ANL_SigmoidDetailed(FANL_II src, FANL_II center, FANL_II ramp)
{
	return ToBPIns(Kernel.sigmoid(ToCIns(src), ToCIns(center), ToCIns(ramp)));
}


FANL_II UANLFacadeLib::ANL_Radial()		{ return ToBPIns(Kernel.radial()); }


FANL_II UANLFacadeLib::ANL_HexTile(FANL_II seed)	{ return ToBPIns(Kernel.hexTile(ToCIns(seed))); }
FANL_II UANLFacadeLib::ANL_HexBump()				{ return ToBPIns(Kernel.hexBump()); }


FANL_II UANLFacadeLib::ANL_Color(FLinearColor c) { return ToBPIns(Kernel.color(*((SRGBA*)(&c)))); }


FANL_II UANLFacadeLib::ANL_CombineRGBA(FANL_II r, FANL_II g, FANL_II b, FANL_II a)
{
	return ToBPIns(Kernel.combineRGBA(ToCIns(r), ToCIns(g), ToCIns(b), ToCIns(a)));
}


FANL_II UANLFacadeLib::ANL_ScaleOffset(FANL_II src, float scale, float offset)
{
	return ToBPIns(Kernel.scaleOffset(ToCIns(src), scale, offset));
}


FANL_II UANLFacadeLib::ANL_SimpleFractalLayer(int basisType, FANL_II interpTypeIndex, float layerScale, float layerFreq, int seed,
											  bool rot, float angle, float ax, float ay, float az)
{
	return ToBPIns(Kernel.simpleFractalLayer((uint32)basisType, ToCIns(interpTypeIndex), layerScale, layerFreq, (uint32)seed,
											 rot, angle, ax, ay, az));
}

FANL_II UANLFacadeLib::ANL_SimpleRidgedLayer(int basisType, FANL_II interpTypeIndex, float layerScale, float layerFreq, int seed,
											 bool rot, float angle, float ax, float ay, float az)
{
	return ToBPIns(Kernel.simpleRidgedLayer((uint32)basisType, ToCIns(interpTypeIndex), layerScale, layerFreq, (uint32)seed,
											rot, angle, ax, ay, az));
}

FANL_II UANLFacadeLib::ANL_SimpleBillowLayer(int basisType, FANL_II interpTypeIndex, float layerScale, float layerFreq, int seed,
											 bool rot, float angle, float ax, float ay, float az)
{
	return ToBPIns(Kernel.simpleBillowLayer((uint32)basisType, ToCIns(interpTypeIndex), layerScale, layerFreq, (uint32)seed,
											rot, angle, ax, ay, az));
}


FANL_II UANLFacadeLib::ANL_Simple_fBm(int basisType, int interpType, int octaves, float freq, int seed, bool rot)
{
	return ToBPIns(Kernel.simplefBm((uint32)basisType, (uint32)interpType, (uint32)octaves, freq, (uint32)seed, rot));
}

FANL_II UANLFacadeLib::ANL_SimpleRidgedMultifractal(int basisType, int interpType, int octaves, float freq, int seed, bool rot)
{
	return ToBPIns(Kernel.simpleRidgedMultifractal((uint32)basisType, (uint32)interpType, (uint32)octaves, freq, (uint32)seed, rot));
}

FANL_II UANLFacadeLib::ANL_SimpleBillow(int basisType, int interpType, int octaves, float freq, int seed, bool rot)
{
	return ToBPIns(Kernel.simpleBillow((uint32)basisType, (uint32)interpType, (uint32)octaves, freq, (uint32)seed, rot));
}


FANL_II UANLFacadeLib::ANL_NextIndex()	{ return ToBPIns(Kernel.nextIndex()); }
FANL_II UANLFacadeLib::ANL_LastIndex()	{ return ToBPIns(Kernel.lastIndex()); }


void UANLFacadeLib::ANL_SetVar(FString name, float val) { Kernel.setVar(TCHAR_TO_ANSI(*name), val); }
FANL_II UANLFacadeLib::ANL_GetVar(FString name) { return ToBPIns(Kernel.getVar(TCHAR_TO_ANSI(*name))); }


FANL_VMOut UANLFacadeLib::ANL_Evaluate(int Dimension, float x, float y, float z, float w, float u, float v)
{
	CCoordinate coord;
	switch (Dimension)
	{
	case 1:
	case 2:
		coord.set(x, y);
		break;

	case 3:
		coord.set(x, y, z);
		break;
		
	case 4:
		coord.set(x, y, z, w);
		break;

	default:
		coord.set(x, y, z, w, u, v);
		break;
	}

	return ToBPOut(VM.evaluate(coord));
}

FANL_VMOut UANLFacadeLib::ANL_EvaluateAt(int Dimension, float x, float y, float z, float w, float u, float v, FANL_II index)
{
	CCoordinate coord;
	switch (Dimension)
	{
	case 1:
	case 2:
		coord.set(x, y);
		break;

	case 3:
		coord.set(x, y, z);
		break;

	case 4:
		coord.set(x, y, z, w);
		break;

	default:
		coord.set(x, y, z, w, u, v);
		break;
	}

	return ToBPOut(VM.evaluateAt(coord, ToCIns(index)));
}


float UANLFacadeLib::ANL_EvaluateScalar(int Dimension, float x, float y, float z, float w, float u, float v, FANL_II idx)
{
	double ret;
	switch (Dimension)
	{
	case 1:
	case 2:
		ret = VM.evaluateScalar(x, y, ToCIns(idx));
		break;

	case 3:
		ret = VM.evaluateScalar(x, y, z, ToCIns(idx));
		break;

	case 4:
		ret = VM.evaluateScalar(x, y, z, w, ToCIns(idx));
		break;

	default:
		ret = VM.evaluateScalar(x, y, z, w, u, v, ToCIns(idx));
		break;
	}

	return (float)ret;
}


FLinearColor UANLFacadeLib::ANL_EvaluateColor(int Dimension, float x, float y, float z, float w, float u, float v, FANL_II idx)
{
	SRGBA ret;
	switch (Dimension)
	{
	case 1:
	case 2:
		ret = VM.evaluateColor(x, y, ToCIns(idx));
		break;

	case 3:
		ret = VM.evaluateColor(x, y, z, ToCIns(idx));
		break;

	case 4:
		ret = VM.evaluateColor(x, y, z, w, ToCIns(idx));
		break;

	default:
		ret = VM.evaluateColor(x, y, z, w, u, v, ToCIns(idx));
		break;
	}

	return *((FLinearColor*)(&ret));
}


// BP-friendly CInstructionIndex ops

FANL_II UANLFacadeLib::Add_InsInds(FANL_II a, FANL_II b) { return { (a.instructionIndex + b.instructionIndex) }; }
FANL_II UANLFacadeLib::Add_InsInd_Int(FANL_II a, int32 b) { return { (a.instructionIndex + b) }; }
FANL_II UANLFacadeLib::Sub_InsInds(FANL_II a, FANL_II b) { return { (a.instructionIndex - b.instructionIndex) }; }
FANL_II UANLFacadeLib::Sub_InsInd_Int(FANL_II a, int32 b) { return { (a.instructionIndex - b) }; }

