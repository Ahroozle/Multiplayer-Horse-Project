// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "NPCDateTime.generated.h"


UENUM(BlueprintType, meta = (Bitflags))
enum class ENPCWeekday : uint8
{
	Sunday,
	Monday,
	Tuesday,
	Wednesday,
	Thursday,
	Friday,
	Saturday
};

UENUM(BlueprintType)
enum class ENPCMonth : uint8
{
	January = 1,
	February,
	March,
	April,
	May,
	June,
	July,
	August,
	September,
	October,
	November,
	December
};

UENUM(BlueprintType)
enum class ENPCSeason : uint8
{
	Winter,
	Spring,
	Summer,
	Fall
};

namespace ENPCTimespan
{
	const int MinutesPerYear = 525600;
	const int MinutesPerWeek = 10080;
	const int MinutesPerDay = 1440;
	const int MinutesPerHour = 60;

	/*
		Solely for converting to and from accurate real-world times. NOT related to lore!

		.4 rl sec => 1 eqmin
	*/
	const float InGameMinutesPerRealSecond = 2.5f;
}

// Timestamp representing the inception of the equine calendar
namespace ENPCCalendarDates
{
	const uint64 BeginningOfTimeStamp = 783360000;//0Ui64;
}

USTRUCT(BlueprintType, meta = (DisplayName = "NPC Timespan", HasNativeMake = "MPHorso.NPCDateFuncLib.MakeNPCTimespan", HasNativeBreak = "MPHorso.NPCDateFuncLib.BreakNPCTimespan"))
struct FNPCTimespan
{
	GENERATED_USTRUCT_BODY();

	FNPCTimespan() : EqMinutes(0) {}
	FNPCTimespan(int64 InMinutes) : EqMinutes(InMinutes) {}
	FNPCTimespan(int Years, int Days, int Hours, int Minutes);

	UPROPERTY()
		int64 EqMinutes;

	int GetYears() { return EqMinutes / ENPCTimespan::MinutesPerYear; }
	int GetDays() { return (EqMinutes - (GetYears()*ENPCTimespan::MinutesPerYear)) / ENPCTimespan::MinutesPerDay; }
	int GetHours() { return (EqMinutes / ENPCTimespan::MinutesPerHour) % 24; }
	int GetMinutes() { return EqMinutes % 60; }
	int GetTotalDays() { return EqMinutes / ENPCTimespan::MinutesPerDay; }
	int GetTotalHours() { return EqMinutes / ENPCTimespan::MinutesPerHour; }
	int GetTotalMinutes() { return EqMinutes; }

	FNPCTimespan operator+(const FNPCTimespan& O) { return FNPCTimespan(EqMinutes + O.EqMinutes); }
	FNPCTimespan& operator+=(const FNPCTimespan& O) { EqMinutes += O.EqMinutes; return *this; }
	FNPCTimespan operator-() { return FNPCTimespan(-EqMinutes); }
	FNPCTimespan operator-(const FNPCTimespan& O) { return FNPCTimespan(EqMinutes - O.EqMinutes); }
	FNPCTimespan& operator-=(const FNPCTimespan& O) { EqMinutes -= O.EqMinutes; return*this; }
	FNPCTimespan operator*(float S) { return FNPCTimespan((int64)(EqMinutes * S)); }
	FNPCTimespan& operator*=(float S) { EqMinutes = (int64)(EqMinutes * S); return *this; }
	FNPCTimespan operator%(const FNPCTimespan& O) { return FNPCTimespan(EqMinutes%O.EqMinutes); }
	FNPCTimespan& operator%=(const FNPCTimespan& O) { EqMinutes %= O.EqMinutes; return *this; }
	bool operator==(const FNPCTimespan& O) { return EqMinutes == O.EqMinutes; }
	bool operator!=(const FNPCTimespan& O) { return EqMinutes != O.EqMinutes; }
	bool operator>(const FNPCTimespan& O) { return EqMinutes > O.EqMinutes; }
	bool operator>=(const FNPCTimespan& O) { return EqMinutes >= O.EqMinutes; }
	bool operator<(const FNPCTimespan& O) { return EqMinutes < O.EqMinutes; }
	bool operator<=(const FNPCTimespan& O) { return EqMinutes <= O.EqMinutes; }

	FString ToString();
	FString ToString(const TCHAR* Format);
};

USTRUCT(BlueprintType, meta = (DisplayName = "NPC DateTime", HasNativeMake = "MPHorso.NPCDateFuncLib.MakeNPCDateTime", HasNativeBreak = "MPHorso.NPCDateFuncLib.BreakNPCDateTime"))
struct FNPCDateTime
{
	GENERATED_USTRUCT_BODY();

	FNPCDateTime() : EqMinutes(0) {}
	FNPCDateTime(uint64 InMinutes) : EqMinutes(InMinutes) {}
	FNPCDateTime(int Year, int Month, int Day, int Hour, int Minute);


	// NPC DateTime's version of 'ticks', the smallest unit is a minute here.
	UPROPERTY()
		uint64 EqMinutes;


	void GetDate(int& OutYear, ENPCMonth& OutMonth, int& OutDay, int& OutHour, int& OutMinute);
	void GetVerboseDate(int& OutYear, ENPCSeason& OutSeason, ENPCMonth& OutMonth, ENPCWeekday& OutDoW, int& OutDay, int& OutHour, int& OutMinute);

	static int DaysInMonth(int Month);
	static const int DaysToMonth[];
	static ENPCSeason SeasonOf(int Month);

	static bool Validate(int Year, int Month, int Day, int Hour, int Minute);


	FNPCDateTime operator+(const FNPCTimespan& O) { return FNPCDateTime(EqMinutes + O.EqMinutes); }
	FNPCDateTime& operator+=(const FNPCTimespan& O) { EqMinutes += O.EqMinutes; return *this; }
	FNPCTimespan operator-(const FNPCDateTime& O) { return FNPCTimespan((int64)EqMinutes - (int64)O.EqMinutes); }
	FNPCDateTime operator-(const FNPCTimespan& O) { return FNPCDateTime(EqMinutes - O.EqMinutes); }
	FNPCDateTime& operator-=(const FNPCTimespan& O) { EqMinutes -= O.EqMinutes; return *this; }
	bool operator==(const FNPCDateTime& O) { return EqMinutes == O.EqMinutes; }
	bool operator!=(const FNPCDateTime& O) { return EqMinutes != O.EqMinutes; }
	bool operator>(const FNPCDateTime& O) { return EqMinutes > O.EqMinutes; }
	bool operator>=(const FNPCDateTime& O) { return EqMinutes >= O.EqMinutes; }
	bool operator<(const FNPCDateTime& O) { return EqMinutes < O.EqMinutes; }
	bool operator<=(const FNPCDateTime& O) { return EqMinutes <= O.EqMinutes; }

	FString ToString();
	FString ToString(const TCHAR* Format);
};

/**
 * 
 */
UCLASS(meta = (DisplayName = "NPC Date Function Library"))
class MPHORSO_API UNPCDateFuncLib : public UObject
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Make NPC DateTime", NativeMakeFunc))
		static FNPCDateTime MakeNPCDateTime(int Year = 1, ENPCMonth Month = ENPCMonth::January, int Day = 1, int Hour = 0, int Minute = 0);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Break NPC DateTime", NativeBreakFunc))
		static void BreakNPCDateTime(FNPCDateTime InDateTime, int& Year, ENPCSeason& Season, ENPCMonth& Month, ENPCWeekday& DayOfWeek, int& Day, int& Hour, int& Minute);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Make NPC Timespan", NativeMakeFunc))
		static FNPCTimespan MakeNPCTimespan(int Years = 0, int Days = 0, int Hours = 0, int Minutes = 0);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Break NPC Timespan", NativeBreakFunc))
		static void BreakNPCTimespan(FNPCTimespan InTimespan, int& Years, int& Days, int& Hours, int& Minutes);
	
	UFUNCTION(BlueprintPure)
		static bool IsValidEquineCalendarDate(const FNPCDateTime& NPCDateTime) { return NPCDateTime.EqMinutes >= ENPCCalendarDates::BeginningOfTimeStamp; }

	UFUNCTION(BlueprintPure)
		static FDateTime GetHumanDate(const FNPCDateTime& NPCDateTime);

	UFUNCTION(BlueprintPure)
		static FTimespan GetHumanTimespan(const FNPCTimespan& NPCTimespan) { return FTimespan(0, NPCTimespan.EqMinutes, 0); }

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get In-Game Minutes From Real Life Seconds"))
		static float GetEqMinFromRLSecs(float TimespanSeconds) { return TimespanSeconds * ENPCTimespan::InGameMinutesPerRealSecond; }
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Real Life Seconds From In-Game Minutes"))
		static float GetRLSecsFromEqMin(float EqTimespanMinutes) { return EqTimespanMinutes / ENPCTimespan::InGameMinutesPerRealSecond; }


	UFUNCTION(BlueprintPure)
		static FNPCTimespan FromYears(int Years) { return FNPCTimespan(Years * ENPCTimespan::MinutesPerYear); }
	UFUNCTION(BlueprintPure)
		static FNPCTimespan FromWeeks(int Weeks) { return FNPCTimespan(Weeks * 7 * ENPCTimespan::MinutesPerDay); }
	UFUNCTION(BlueprintPure)
		static FNPCTimespan FromDays(int Days) { return FNPCTimespan(Days * ENPCTimespan::MinutesPerDay); }
	UFUNCTION(BlueprintPure)
		static FNPCTimespan FromHours(int Hours) { return FNPCTimespan(Hours * ENPCTimespan::MinutesPerHour); }
	UFUNCTION(BlueprintPure)
		static FNPCTimespan FromMinutes(int Minutes) { return FNPCTimespan(Minutes); }

	// lifted and rewritten from KismetMathLibrary to save time

	/* Addition (A + B) */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "NPCDateTime + NPCTimespan", CompactNodeTitle = "+", Keywords = "+ add plus"))
		static FNPCDateTime Add_DateTimeTimespan(FNPCDateTime A, FNPCTimespan B);

	/* Subtraction (A - B) */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "NPCDateTime - NPCTimespan", CompactNodeTitle = "-", Keywords = "- subtract minus"))
		static FNPCDateTime Subtract_DateTimeTimespan(FNPCDateTime A, FNPCTimespan B);

	/* Subtraction (A - B) */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "NPCDateTime - NPCDateTime", CompactNodeTitle = "-", Keywords = "- subtract minus"))
		static FNPCTimespan Subtract_DateTimeDateTime(FNPCDateTime A, FNPCDateTime B);

	/* Returns true if the values are equal (A == B) */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Equal (NPCDateTime)", CompactNodeTitle = "==", Keywords = "== equal"))
		static bool EqualEqual_DateTimeDateTime(FNPCDateTime A, FNPCDateTime B);

	/* Returns true if the values are not equal (A != B) */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "NotEqual (NPCDateTime)", CompactNodeTitle = "!=", Keywords = "!= not equal"))
		static bool NotEqual_DateTimeDateTime(FNPCDateTime A, FNPCDateTime B);

	/* Returns true if A is greater than B (A > B) */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "NPCDateTime > NPCDateTime", CompactNodeTitle = ">", Keywords = "> greater"))
		static bool Greater_DateTimeDateTime(FNPCDateTime A, FNPCDateTime B);

	/* Returns true if A is greater than or equal to B (A >= B) */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "NPCDateTime >= NPCDateTime", CompactNodeTitle = ">=", Keywords = ">= greater"))
		static bool GreaterEqual_DateTimeDateTime(FNPCDateTime A, FNPCDateTime B);

	/* Returns true if A is less than B (A < B) */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "NPCDateTime < NPCDateTime", CompactNodeTitle = "<", Keywords = "< less"))
		static bool Less_DateTimeDateTime(FNPCDateTime A, FNPCDateTime B);

	/* Returns true if A is less than or equal to B (A <= B) */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "NPCDateTime <= NPCDateTime", CompactNodeTitle = "<=", Keywords = "<= less"))
		static bool LessEqual_DateTimeDateTime(FNPCDateTime A, FNPCDateTime B);


	/* Addition (A + B) */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "NPCTimespan + NPCTimespan", CompactNodeTitle = "+", Keywords = "+ add plus"))
		static FNPCTimespan Add_TimespanTimespan(FNPCTimespan A, FNPCTimespan B);

	/* Subtraction (A - B) */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "NPCTimespan - NPCTimespan", CompactNodeTitle = "-", Keywords = "- subtract minus"))
		static FNPCTimespan Subtract_TimespanTimespan(FNPCTimespan A, FNPCTimespan B);

	/* Scalar multiplication (A * s) */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "NPCTimespan * float", CompactNodeTitle = "*", Keywords = "* multiply"))
		static FNPCTimespan Multiply_TimespanFloat(FNPCTimespan A, float Scalar);

	/* Returns true if the values are equal (A == B) */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Equal (NPCTimespan)", CompactNodeTitle = "==", Keywords = "== equal"))
		static bool EqualEqual_TimespanTimespan(FNPCTimespan A, FNPCTimespan B);

	/* Returns true if the values are not equal (A != B) */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "NotEqual (NPCTimespan)", CompactNodeTitle = "!=", Keywords = "!= not equal"))
		static bool NotEqual_TimespanTimespan(FNPCTimespan A, FNPCTimespan B);

	/* Returns true if A is greater than B (A > B) */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "NPCTimespan > NPCTimespan", CompactNodeTitle = ">", Keywords = "> greater"))
		static bool Greater_TimespanTimespan(FNPCTimespan A, FNPCTimespan B);

	/* Returns true if A is greater than or equal to B (A >= B) */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "NPCTimespan >= NPCTimespan", CompactNodeTitle = ">=", Keywords = ">= greater"))
		static bool GreaterEqual_TimespanTimespan(FNPCTimespan A, FNPCTimespan B);

	/* Returns true if A is less than B (A < B) */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "NPCTimespan < NPCTimespan", CompactNodeTitle = "<", Keywords = "< less"))
		static bool Less_TimespanTimespan(FNPCTimespan A, FNPCTimespan B);

	/* Returns true if A is less than or equal to B (A <= B) */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "NPCTimespan <= NPCTimespan", CompactNodeTitle = "<=", Keywords = "<= less"))
		static bool LessEqual_TimespanTimespan(FNPCTimespan A, FNPCTimespan B);


	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (NPCDateTime)", CompactNodeTitle = "->", BlueprintAutocast))
		static FString Conv_NPCDateTimeToString(FNPCDateTime InDateTime) { return InDateTime.ToString(); }
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (NPCTimespan)", CompactNodeTitle = "->", BlueprintAutocast))
		static FString Conv_NPCTimespanToString(FNPCTimespan InTimespan) { return InTimespan.ToString(); }

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (NPCDateTime) With Format"))
		static FString FormatNPCDateTime(FNPCDateTime InDateTime, FString Format) { return (Format.IsEmpty() ? InDateTime.ToString() : InDateTime.ToString(*Format)); }
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (NPCTimespan) With Format"))
		static FString FormatNPCTimespan(FNPCTimespan InTimespan, FString Format) { return (Format.IsEmpty() ? InTimespan.ToString() : InTimespan.ToString(*Format)); }


	UFUNCTION()
		static FString GetWeekdayString(ENPCWeekday Weekday, bool Shorten = false);

	UFUNCTION()
		static FString GetMonthString(ENPCMonth Month, bool Shorten = false);

	// Gets the ordinal of a number (i.e. 1->"st",2->"nd", etc.)
	UFUNCTION()
		static FString GetOrdinalOf(int Num);

	/*
		Silly little function based on Thori's QST impl in Lunabot.
		(NOTE: Expects UTC!)
		Link to reference in source.
	*/
	UFUNCTION(BlueprintPure)
		static FDateTime GetQST(FDateTime DateTime)
		{
			// Derived from https://github.com/Thorinair/Princess-Luna/blob/master/luna.js

			// Expects UTC
			// now / 8760 + 93*365*24*60*60*1000 + 11*60*60*1000
			// seems to be adding 93 years and 11 hours of ms to the value
			// according to Thori, 8760 is the time dilation divisor
			// I know they're magic horses but do they have to use magic *numbers* too? :^)

			return FDateTime::FromUnixTimestamp(DateTime.ToUnixTimestamp() / 8760 + 2932887600);
		}

};
