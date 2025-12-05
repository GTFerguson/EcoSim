/**
 *  Title   : Calendar
 *	Author  : Gary Ferguson
 *	Date		: Jun 10, 2020 03:39:24 (BST)
 *	Purpose	: This class handles a calendar for in-game use.
 */

#include "../include/calendar.hpp"

using namespace std;

//================================================================================
//  Constants 
//================================================================================
// TODO a vector of structs representing each month might be interesting
const vector<string> Calendar::MONTH_STRINGS = 
  { "January",  "February", "March",      "April",    "May",      "June",
    "July",     "August",   "September",  "October",  "November", "December"  };

const vector<unsigned short> Calendar::DAYS_IN_MONTH = 
  { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

//================================================================================
//  Constructors
//================================================================================
Calendar::Calendar () {
  _time.minute  = 0;
  _time.hour    = 0;
  _date.day     = 1;
  _date.month   = 0;
  _date.year    = 100;
}

Calendar::Calendar (const Time &time, const Date &date) {
  _time = time;
  _date = date;
}

//================================================================================
//  Incrementing Methods 
//================================================================================
//  Postfix - returns copy of original value before modification
Calendar Calendar::operator++ (int) {
  Calendar old = *this;
  incrementByMinute ();
  return old;
}

void Calendar::incrementByMinute () {
  _time.minute++;

  if (_time.minute >= MINUTES_IN_HOUR) {
    incrementByHour ();
    _time.minute = 0;
  }
}

void Calendar::incrementByHour () {
  _time.hour++;

  if (_time.hour >= HOURS_IN_DAY) {
    incrementByDay ();
    _time.hour = 0;
  }
}

void Calendar::incrementByDay () {
  _date.day++;

  if (_date.day > DAYS_IN_MONTH.at(_date.month)) {
    incrementByMonth ();
    _date.day = 1;
  }
}

void Calendar::incrementByMonth () {
  _date.month++;

  if (_date.month >= MONTHS_IN_YEAR) {
    incrementByYear ();
    _date.month = 0;
  }
}

void Calendar::incrementByYear () {
  _date.year++;
}

//================================================================================
//  Getters
//================================================================================
unsigned short  Calendar::getMinute () const { return _time.minute; }
unsigned short  Calendar::getHour   () const { return _time.hour;   }
unsigned short  Calendar::getDay    () const { return _date.day;    }
unsigned short  Calendar::getMonth  () const { return _date.month;  }
unsigned        Calendar::getYear   () const { return _date.year;   }

//================================================================================
//  Formatted Date
//================================================================================
std::string Calendar::shortTime () const {
  std::ostringstream ss;
  ss << std::setw(2) << std::setfill('0') << _time.hour << ":"
     << std::setw(2) << std::setfill('0') << _time.minute;
  return ss.str();
}

std::string Calendar::monthToString (const unsigned short &month) const {
  return MONTH_STRINGS.at(month);
}

std::string Calendar::getDayAffix (const unsigned short &day) const {
  switch (day) {
    case 1: case 21: case 31:   return "st";
    case 2: case 22:            return "nd";
    case 3: case 23:            return "rd";
    default:                    return "th";
  }
}

std::string Calendar::shortDate () const {
  std::ostringstream ss;
  ss << _date.day << "-" << _date.month+1 << "-" << _date.year;
  return ss.str();
}

std::string Calendar::longDate () const {
  std::ostringstream ss;
  ss  << monthToString(_date.month)           << " "
      << _date.day << getDayAffix(_date.day)  << ", "
      << _date.year;
  return ss.str();
}

//================================================================================
//  To String
//================================================================================
std::string Calendar::toString () const {
  std::ostringstream ss;
  ss  << _time.hour << "," << _time.minute  << ","
      << _date.day  << "," << _date.month   << "," << _date.year;
  return ss.str();
}
