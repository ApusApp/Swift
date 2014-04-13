#include <iostream>

#include <swift/base/Timestamp.h>
#include <swift/base/MurmurHash3.h>
#include <swift/base/Date.h>
#include <swift/base/LinkedHashMap.h>

using namespace std;

int main (int argc, char* argv[])
{
	swift::Timestamp tm = swift::Timestamp::Now ();
	
	std::cout << tm.ToFormattedString () << std::endl;
	std::cout << tm.SecondsSinceEpoch () << std::endl;
	swift::Date d (2014, 4, 10);
	std::cout << d.ToString () << " " << d.GetJulianDayNumber () << " Weekday: " << d.WeekDay () << std::endl;

	swift::LinkedHashMap<int, int> lhm;
	int a = 100;
	int b = 100;
	lhm.Set (100, 100, swift::LinkedHashMap<int, int>::MM_FIRST);
	lhm.Set (101, 100, swift::LinkedHashMap<int, int>::MM_FIRST);
	lhm.Set (102, 100, swift::LinkedHashMap<int, int>::MM_FIRST);
	lhm.Set (103, 100, swift::LinkedHashMap<int, int>::MM_FIRST);
	std::cout << *lhm.Get (100, swift::LinkedHashMap<int, int>::MM_FIRST) << std::endl;

	for (auto it = lhm.Begin (); it != lhm.End (); ++it) {
		std::cout << it.Key () << " : " << it.Value () << std::endl;
	}

	return 0; 
}