// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "HorsoEyedropperBase.h"


FCursorReply UHorsoEyedropperBase::NativeOnCursorQuery(const FGeometry& InGeometry, const FPointerEvent& InCursorEvent)
{
	return FCursorReply::Cursor(RetrieveCursorType());
}
