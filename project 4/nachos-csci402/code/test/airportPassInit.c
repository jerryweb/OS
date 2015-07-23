/* Networked airport simulation.
 *  Initializes the 21 passengers.
 */

#include "syscall.h"
#include "airportStructs.h"

#define NULL (void*)0

int main()
{
    int i;
    Printf("Initializing Passengers\n", 24, 0, 0);
    for (i = 0; i < 21; i++)
    {
    	Exec("../test/airportPass", 19);
	}
}
