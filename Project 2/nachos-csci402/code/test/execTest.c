/*Exec Test
	This test takes in a test file, and essentially creates a new process.
	It is similar to the start process function. The current test file is 
	halt.c, which halts the machine.
*/

#include "syscall.h"

void testFuction(){
	Printf("Fort Test Fucntion\n", 19, 0,0);
}

int main(){
	int processLocation = 0;

	processLocation = Exec("../test/halt", 12);

	if(processLocation != -1)
		Printf("New process location: %d\n", 26, 1, processLocation);
}
