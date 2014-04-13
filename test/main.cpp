#include <iostream>

#include <swift/base/Timestamp.h>
#include <swift/base/MurmurHash3.h>
#include <swift/base/Date.h>

int main (int argc, char* argv[])
{
	swift::Timestamp tm = swift::Timestamp::Now ();
	
	std::cout << tm.ToFormattedString () << std::endl;
	std::cout << tm.SecondsSinceEpoch () << std::endl;
	swift::Date d (2014, 4, 10);
	std::cout << d.ToString () << " " << d.GetJulianDayNumber () << " Weekday: " << d.WeekDay () << std::endl;
	return 0; 
}