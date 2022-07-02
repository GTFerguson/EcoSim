/**
 *  Title   :
 *	Author  : Gary Ferguson
 *	Date		: Jun 10, 2020 05:28:09 (BST)
 *	Purpose	:
 */

#include <iostream>
#include "../../include/calendar.hpp"

using namespace std;

int main () {
  Calendar c;
  cout << endl << c.toString () << endl;
  c.incrementByMinute ();
  cout << endl << c.toString () << endl;
  c.incrementByYear ();
  cout << endl << c.toString () << endl;
	return 0;
}

