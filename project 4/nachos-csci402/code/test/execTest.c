/*Exec Test
	This test takes in a test file, and essentially creates a new process.
	It is similar to the start process function. The current test file is 
	testf, which halts the machine. The next exec function should through 
	an unable to load file exception because the file is not in the test 
	directory.
*/

#include "syscall.h"

int main(){
	/*Tests the creation of the new process and main thread*/
	int processLocation = Exec("../test/testfiles", 17);

	if(processLocation != -1)
		Printf("New process location: %d\n", 26, 1, processLocation);

	/*Tests input with a nonexistant file */
	processLocation = Exec("../test/FooBar", 14);
	/*Tests input with a nonexistant file with the incorrect number of
	characters in the file name.
	processLocation = Exec("../test/Fooar", 14);*/
}
