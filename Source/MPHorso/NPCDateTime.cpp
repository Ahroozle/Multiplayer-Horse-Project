// Fill out your copyright notice in the Description page of Project Settings.

#include "MPHorso.h"
#include "NPCDateTime.h"

#include "StaticFuncLib.h"


const int FNPCDateTime::DaysToMonth[] = { 0,30,61,91,122,152,183,213,243,274,304,335,365 };

FNPCTimespan::FNPCTimespan(int Years, int Days, int Hours, int Minutes)
{
	EqMinutes = (Years * ENPCTimespan::MinutesPerYear) +
				(Days * ENPCTimespan::MinutesPerDay) +
				(Hours * ENPCTimespan::MinutesPerHour) +
				Minutes;
}

FString FNPCTimespan::ToString()
{
	FString Format = "%n";

	if (GetYears() != 0)
		Format += "%y,";

	if (GetDays() != 0)
		Format += "%d.";

	return ToString(*(Format + "%h:%m"));
}

FString FNPCTimespan::ToString(const TCHAR* Format)
{
	FString Result;

	while (*Format != TCHAR('\0'))
	{
		if ((*Format == TCHAR('%')) && (*++Format != TCHAR('\0')))
		{
			switch (*Format)
			{
			case TCHAR('n'): if (EqMinutes < 0) Result += TCHAR('-'); break;
			case TCHAR('N'): Result += (EqMinutes < 0) ? TCHAR('-') : TCHAR('+'); break;
			case TCHAR('y'): Result += FString::Printf(TEXT("%i"), FMath::Abs(GetYears())); break;
			case TCHAR('d'): Result += FString::Printf(TEXT("%i"), FMath::Abs(GetDays())); break;
			case TCHAR('h'): Result += FString::Printf(TEXT("%02i"), FMath::Abs(GetHours())); break;
			case TCHAR('m'): Result += FString::Printf(TEXT("%02i"), FMath::Abs(GetMinutes())); break;
			case TCHAR('D'): Result += FString::Printf(TEXT("%i"), FMath::Abs(GetTotalDays())); break;
			case TCHAR('H'): Result += FString::Printf(TEXT("%i"), FMath::Abs(GetTotalHours())); break;
			case TCHAR('M'): Result += FString::Printf(TEXT("%i"), FMath::Abs(GetTotalMinutes())); break;
			default:		 Result += *Format;
			}
		}
		else
			Result += *Format;

		++Format;
	}

	return Result;
}


int FNPCDateTime::DaysInMonth(int Month)
{
	// {30, 31, 30, 31, 30, 31, 30, 30, 31, 30, 31, 30}

	if (Month < 7)
		return 30 + (--Month % 2);

	return 30 + ((Month - 8) % 2);
}

ENPCSeason FNPCDateTime::SeasonOf(int Month)
{
	// {0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 0, 0}

	return (ENPCSeason)FMath::FloorToInt((++Month % 12) / 3.0f);
}

FNPCDateTime::FNPCDateTime(int Year, int Month, int Day, int Hour, int Minute)
{
	check(Validate(Year, Month, Day, Hour, Minute));

	--Year;
	--Month;

	int TotalDays = 0;
	TotalDays += Year * 365;
	TotalDays += DaysToMonth[Month];
	TotalDays += Day - 1;

	EqMinutes = (TotalDays * ENPCTimespan::MinutesPerDay) +
				(Hour * ENPCTimespan::MinutesPerHour) +
				Minute +
				ENPCCalendarDates::BeginningOfTimeStamp;
}

void FNPCDateTime::GetDate(int& OutYear, ENPCMonth& OutMonth, int& OutDay, int& OutHour, int& OutMinute)
{
	// All NPC DateTimes are technically timespans counting the minutes from Sunday, January 1st, 0001 00:00 of the Equine Calendar

	if (!UNPCDateFuncLib::IsValidEquineCalendarDate(*this))
	{
		UStaticFuncLib::Print("FNPCDateTime::GetDate: This NPCDateTime is invalid for Equine Calendar ops.");
		return;
	}

	uint64 StoredEqMin = EqMinutes - ENPCCalendarDates::BeginningOfTimeStamp;

	OutYear = (StoredEqMin / ENPCTimespan::MinutesPerYear);
	if (OutYear * ENPCTimespan::MinutesPerYear <= StoredEqMin)
		StoredEqMin -= OutYear * ENPCTimespan::MinutesPerYear;
	++OutYear;

	int FoundMonth = 13;
	OutDay = (StoredEqMin / ENPCTimespan::MinutesPerDay);
	while (DaysToMonth[--FoundMonth] > OutDay);
	OutMonth = (ENPCMonth)(FoundMonth + 1);
	if (DaysToMonth[FoundMonth] * ENPCTimespan::MinutesPerDay <= StoredEqMin)
		StoredEqMin -= DaysToMonth[FoundMonth] * ENPCTimespan::MinutesPerDay;

	OutDay -= DaysToMonth[FoundMonth];
	if (OutDay * ENPCTimespan::MinutesPerDay <= StoredEqMin)
		StoredEqMin -= OutDay * ENPCTimespan::MinutesPerDay;
	++OutDay;

	OutHour = (StoredEqMin / ENPCTimespan::MinutesPerHour);
	if (OutHour * ENPCTimespan::MinutesPerHour <= StoredEqMin)
		StoredEqMin -= OutHour * ENPCTimespan::MinutesPerHour;

	OutMinute = StoredEqMin;
}

void FNPCDateTime::GetVerboseDate(int& OutYear, ENPCSeason& OutSeason, ENPCMonth& OutMonth, ENPCWeekday& OutDoW, int& OutDay, int& OutHour, int& OutMinute)
{
	GetDate(OutYear, OutMonth, OutDay, OutHour, OutMinute);
	OutSeason = SeasonOf((int)OutMonth);
	OutDoW = (ENPCWeekday)(((EqMinutes - ENPCCalendarDates::BeginningOfTimeStamp) / ENPCTimespan::MinutesPerDay) % 7);
}

bool FNPCDateTime::Validate(int Year, int Month, int Day, int Hour, int Minute)
{
	return (Year > 0) &&
			(Month > 0 && Month < 13) &&
			(Day > 0 && Day <= DaysInMonth(Month)) &&
			(Hour >= 0 && Hour < 24) &&
			(Minute >= 0 && Minute < 60);
}

FString FNPCDateTime::ToString()
{
	return ToString(TEXT("%Y.%M.%d-%H.%m"));
}

FString FNPCDateTime::ToString(const TCHAR* Format)
{
	FString Result;

	if (Format != nullptr)
	{
		int Year, Day, Hour, Minute;
		ENPCSeason Season;
		ENPCMonth Month;
		ENPCWeekday DayOfWeek;
		GetVerboseDate(Year, Season, Month, DayOfWeek, Day, Hour, Minute);

		const TCHAR EndChar('\0');
		const TCHAR FieldChar('%');

		TSet<TCHAR> EndFieldChars({ '/', '.', '-', ' ', '\0' });

		while (*Format != EndChar)
		{
			if (*Format == FieldChar && *(Format + 1) != EndChar)
			{
				const TCHAR* FieldEnd = Format;
				FString FormatChunk;
				while (!EndFieldChars.Contains(*(++FieldEnd)))
					FormatChunk += *FieldEnd;

				if (!FormatChunk.IsEmpty())
				{
					if (FormatChunk.Len() == 1)
					{
						switch (FormatChunk[0])
						{
						case TCHAR('a'): Result += Hour < 12 ? TEXT("am") : TEXT("pm"); break;
						case TCHAR('A'): Result += Hour < 12 ? TEXT("AM") : TEXT("PM"); break;

						case TCHAR('d'):
						case TCHAR('D'): Result += FString::Printf(TEXT("%02i"), Day); break;

						case TCHAR('M'): Result += FString::Printf(TEXT("%02i"), (int)Month); break;

						case TCHAR('y'):
						case TCHAR('Y'): Result += FString::Printf(TEXT("%i"), Year); break;

						case TCHAR('h'): Result += FString::Printf(TEXT("%02i"), (Hour < 1 ? 12 : (Hour > 12 ? Hour - 12 : Hour))); break;
						case TCHAR('H'): Result += FString::Printf(TEXT("%02i"), Hour); break;

						case TCHAR('m'): Result += FString::Printf(TEXT("%02i"), Minute); break;

						case TCHAR('w'): Result += UNPCDateFuncLib::GetWeekdayString(DayOfWeek, true); break;
						case TCHAR('W'): Result += UNPCDateFuncLib::GetWeekdayString(DayOfWeek); break;

						default:		 Result += *Format;
						}
					}
					else
					{
						if (FormatChunk == "Do") Result += FString::Printf(TEXT("%i%s"), Day, *UNPCDateFuncLib::GetOrdinalOf(Day));
						else if (FormatChunk == "dd" /*|| FormatChunk == "DD"*/) Result += FString::Printf(TEXT("%02i"), Day);
						else if (FPlatformString::Strcmp(*FormatChunk, TEXT("MM")) == 0) Result += FString::Printf(TEXT("%02i"), (int)Month);
						else if (FormatChunk == "yy" /*|| FormatChunk == "YY"*/) Result += FString::Printf(TEXT("%02i"), Year % 100);
						else if (FPlatformString::Strcmp(*FormatChunk, TEXT("HH")) == 0) Result += FString::Printf(TEXT("%02i"), Hour);
						else if (FormatChunk == "hh") Result += FString::Printf(TEXT("%02i"), (Hour < 1 ? 12 : (Hour > 12 ? Hour - 12 : Hour)));
						else if (FormatChunk == "mm") Result += FString::Printf(TEXT("%02i"), Minute);
						else if (FPlatformString::Strcmp(*FormatChunk, TEXT("ddd")) == 0) Result += UNPCDateFuncLib::GetWeekdayString(DayOfWeek, true);
						else if (FormatChunk == "DDD") Result += FString::Printf(TEXT("%03i"), (EqMinutes / ENPCTimespan::MinutesPerDay) % 365);
						else if (FormatChunk == "MMM") Result += UNPCDateFuncLib::GetMonthString(Month, true);
						else if (FPlatformString::Strcmp(*FormatChunk, TEXT("dddd")) == 0) Result += UNPCDateFuncLib::GetWeekdayString(DayOfWeek);
						else if (FormatChunk == "DDDD") Result += FString::Printf(TEXT("%03i"), (EqMinutes / ENPCTimespan::MinutesPerDay) % 365);
						else if (FormatChunk == "MMMM") Result += UNPCDateFuncLib::GetMonthString(Month);
						else if (FormatChunk == "YYYY") Result += FString::Printf(TEXT("%04i"), Year);
						else Result += *Format;
					}

					Format = FieldEnd - 1;
				}
				else
					Result += *Format;
			}
			else
				Result += *Format;

			++Format;
		}
	}

	return Result;
}


FNPCDateTime UNPCDateFuncLib::MakeNPCDateTime(int Year, ENPCMonth Month, int Day, int Hour, int Minute)
{
	if (!FNPCDateTime::Validate(Year, (int)Month, Day, Hour, Minute))
	{
		FFrame::KismetExecutionMessage(*FString::Printf(TEXT("NPCDateTime in bad format (year %d, month %d, day %d, hour %d, minute %d). E.g. year, month and day can't be zero."), Year, (int)Month, Day, Hour, Minute), ELogVerbosity::Warning, FName("InvalidDateWarning"));

		return FNPCDateTime();
	}

	return FNPCDateTime(Year, (int)Month, Day, Hour, Minute);
}

void UNPCDateFuncLib::BreakNPCDateTime(FNPCDateTime InDateTime, int& Year, ENPCSeason& Season, ENPCMonth& Month, ENPCWeekday& DayOfWeek, int& Day, int& Hour, int& Minute)
{
	InDateTime.GetVerboseDate(Year, Season, Month, DayOfWeek, Day, Hour, Minute);
}

FNPCTimespan UNPCDateFuncLib::MakeNPCTimespan(int Years, int Days, int Hours, int Minutes)
{
	return FNPCTimespan(Years, Days, Hours, Minutes);
}

void UNPCDateFuncLib::BreakNPCTimespan(FNPCTimespan InTimespan, int& Years, int& Days, int& Hours, int& Minutes)
{
	Years = InTimespan.GetYears();
	Days = InTimespan.GetDays();
	Hours = InTimespan.GetHours();
	Minutes = InTimespan.GetMinutes();
}

FDateTime UNPCDateFuncLib::GetHumanDate(const FNPCDateTime& NPCDateTime)
{
	// The very beginning
	FDateTime StartHumanDate(2620, (int)EMonthOfYear::May, 18, 8, 37, 21, 8);
	FTimespan ToCurrent(0, NPCDateTime.EqMinutes, 0);

	return StartHumanDate + ToCurrent;
}

FNPCDateTime UNPCDateFuncLib::Add_DateTimeTimespan(FNPCDateTime A, FNPCTimespan B) { return A + B; }
FNPCDateTime UNPCDateFuncLib::Subtract_DateTimeTimespan(FNPCDateTime A, FNPCTimespan B) { return A - B; }
FNPCTimespan UNPCDateFuncLib::Subtract_DateTimeDateTime(FNPCDateTime A, FNPCDateTime B) { return A - B; }
bool UNPCDateFuncLib::EqualEqual_DateTimeDateTime(FNPCDateTime A, FNPCDateTime B) { return A == B; }
bool UNPCDateFuncLib::NotEqual_DateTimeDateTime(FNPCDateTime A, FNPCDateTime B) { return A != B; }
bool UNPCDateFuncLib::Greater_DateTimeDateTime(FNPCDateTime A, FNPCDateTime B) { return A > B; }
bool UNPCDateFuncLib::GreaterEqual_DateTimeDateTime(FNPCDateTime A, FNPCDateTime B) { return A >= B; }
bool UNPCDateFuncLib::Less_DateTimeDateTime(FNPCDateTime A, FNPCDateTime B) { return A < B; }
bool UNPCDateFuncLib::LessEqual_DateTimeDateTime(FNPCDateTime A, FNPCDateTime B) { return A <= B; }

FNPCTimespan UNPCDateFuncLib::Add_TimespanTimespan(FNPCTimespan A, FNPCTimespan B) { return A + B; }
FNPCTimespan UNPCDateFuncLib::Subtract_TimespanTimespan(FNPCTimespan A, FNPCTimespan B) { return A - B; }
FNPCTimespan UNPCDateFuncLib::Multiply_TimespanFloat(FNPCTimespan A, float Scalar) { return A*Scalar; }
bool UNPCDateFuncLib::EqualEqual_TimespanTimespan(FNPCTimespan A, FNPCTimespan B) { return A == B; }
bool UNPCDateFuncLib::NotEqual_TimespanTimespan(FNPCTimespan A, FNPCTimespan B) { return A != B; }
bool UNPCDateFuncLib::Greater_TimespanTimespan(FNPCTimespan A, FNPCTimespan B) { return A > B; }
bool UNPCDateFuncLib::GreaterEqual_TimespanTimespan(FNPCTimespan A, FNPCTimespan B) { return A >= B; }
bool UNPCDateFuncLib::Less_TimespanTimespan(FNPCTimespan A, FNPCTimespan B) { return A < B; }
bool UNPCDateFuncLib::LessEqual_TimespanTimespan(FNPCTimespan A, FNPCTimespan B) {return A <= B; }

FString UNPCDateFuncLib::GetWeekdayString(ENPCWeekday Weekday, bool Shorten)
{
	UEnum* Gotten = FindObject<UEnum>(ANY_PACKAGE, TEXT("ENPCWeekday"), true);
	if (!Gotten)
		return "Errorsday";
	
	FString Retrieved = Gotten->GetNameByValue((int)Weekday).ToString();
	Retrieved.ReplaceInline(TEXT("ENPCWeekday::"), TEXT(""));
	if (Shorten)
		Retrieved = Retrieved.Left(3);

	return Retrieved;
}

FString UNPCDateFuncLib::GetMonthString(ENPCMonth Month, bool Shorten)
{
	UEnum* Gotten = FindObject<UEnum>(ANY_PACKAGE, TEXT("ENPCMonth"), true);
	if (!Gotten)
		return "Errorcember";

	FString Retrieved = Gotten->GetNameByValue((int)Month).ToString();
	Retrieved.ReplaceInline(TEXT("ENPCMonth::"), TEXT(""));
	if (Shorten)
		Retrieved = Retrieved.Left(3);

	return Retrieved;
}

FString UNPCDateFuncLib::GetOrdinalOf(int Num)
{
	int Tens = Num % 100;

	// everything from 4 to 20 has 'th' as its suffix.
	if (Tens > 3 && Tens < 21)
		return "th";
	else
	{
		// Then every singular digit up to 3 has a unique prefix
		switch (Tens % 10)
		{
		case 1: return "st";
		case 2: return "nd";
		case 3: return "rd";
		default: return "th";
		}
	}
}
