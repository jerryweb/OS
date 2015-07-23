/* Networked airport simulation.
 *  Initializes the 9 check in staff and manager.
 */

#include "syscall.h"
#include "airportStructs.h"

#define NULL (void*)0

int main()
{
    int i;
    Printf("Initializing CheckinStaff\n", 26, 0, 0);
    for (i = 0; i < 9; i++)
    {
    	Exec("../test/airportCheckin", 22);
	}
    Printf("Initializing Manager\n", 27, 0, 0);
    for (i = 0; i < 5; i++)
    {
    	Exec("../test/airportManager", 22);
	}
}
