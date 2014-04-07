#include "Pathfinder.h"
#include <iostream>

using namespace std;

const int SHIP_ID = 29;
const int SHIP_X = 7;
const int SHIP_Y = 2;

int main()
{
	CPathfinder pathfinder = CPathfinder();

	// path from ship to 2,11 (id=123)
	// !***** results in hexMap_Path1.jpg *****!
	// testing
	cout << "\nPath Success?: " << pathfinder.FindPath(SHIP_ID , 123) << "\n\n";

	// path from ship to 6,13 (id ==)
	// !***** results in hexMap_Path2.jpg *****!
	cout << "\nPath Success?: " << pathfinder.FindPath(SHIP_X,SHIP_Y , 2,2) << "\n\n";

	system("pause");
}