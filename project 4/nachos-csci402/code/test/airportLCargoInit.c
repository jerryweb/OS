/* Networked airport simulation.
 *  Initializes the 5 liaisons and 5 cargo handlers.
 */

#include "syscall.h"
#include "airportStructs.h"

#define NULL (void*)0

int main()
{
    int i;
    Printf("Initializing Liaisons\n", 22, 0, 0);
    for (i = 0; i < 5; i++)
    {
    	Exec("../test/airportLiaison", 22);
	}
    Printf("Initializing CargoHandlers\n", 27, 0, 0);
    for (i = 0; i < 5; i++)
    {
    	Exec("../test/airportCargo", 20);
	}
}
