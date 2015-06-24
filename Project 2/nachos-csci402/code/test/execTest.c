/*Exec Test
	This test takes in a test file, and essentially creates a new process.
	It is similar to the start process function. The current test file is 
	testf, which halts the machine. The next exec function should through 
	an unable to load file exception because the file is not in the test 
	directory.
*/

#include "syscall.h"

void testFuction(){
	Printf("Fort Test Fucntion\n", 19, 0,0);
}

int main(){
	int processLocation = 0;

	processLocation = Exec("../test/testfiles", 17);

	if(processLocation != -1)
		Printf("New process location: %d\n", 26, 1, processLocation);

	processLocation = Exec("../test/FooBar", 14);

}
