/*Airport Simulation
	This will run 2 instances of the airport simulation using system calls.
	Both airports will run simulaniously, and the print statments will follow
	consecutively in sections. For example, all of the passenger threads for
	both processes will	operate until a context switch, then the liaisons 
	will operate, etc..
*/
#include "syscall.h"


int main(){

	int airportA = Exec("../test/airportSim", 18);
	/*int airportB = Exec("../test/airportSim", 18);*/

}

