#ifndef CALENDAR_H
#define CALENDAR_H

/**
 *  Title   : Calendar
 *	Author  : Gary Ferguson
 *	Date		: Jun 10, 2020 03:35:51 (BST)
 *	Purpose	: This class handles a calendar for in-game use.
 */

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

struct Date {
  unsigned short day, month;
  unsigned year;
};

struct Time {
  unsigned short minute, hour;
};

class Calendar {
  private:
    const static unsigned short MINUTES_IN_HOUR = 60;
    const static unsigned short HOURS_IN_DAY    = 24;
    const static unsigned short MONTHS_IN_YEAR  = 12;

    const static std::vector<std::string>     MONTH_STRINGS;
    const static std::vector<unsigned short>  DAYS_IN_MONTH;

    Date _date;
    Time _time;

  public:
    //============================================================================
    //  Constructors
    //============================================================================
    Calendar ();
    Calendar (const Time &time, const Date &date);

    //============================================================================
    //  Incrementing
    //============================================================================
    Calendar operator++ (int);
    void incrementByMinute  ();
    void incrementByHour    ();
    void incrementByDay     ();
    void incrementByMonth   ();
    void incrementByYear    ();

    //============================================================================
    //  Getters
    //============================================================================
    unsigned short  getMinute () const;
    unsigned short  getHour   () const;
    unsigned short  getDay    () const;
    unsigned short  getMonth  () const;
    unsigned        getYear   () const;
    
    //============================================================================
    //  Formatted Date
    //============================================================================
    std::string shortTime     () const;
    std::string getDayAffix   (const unsigned short &day) const;
    std::string monthToString (const unsigned short &month) const;
    std::string shortDate     () const;
    std::string longDate      () const;

    //============================================================================
    //  To String 
    //============================================================================
    std::string toString () const;
};

#endif
