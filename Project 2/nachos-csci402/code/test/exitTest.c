/*Exit Test
	This test takes in a test file, and essentially creates a new process.
	It is similar to the start process function. The current test file is 
	testf, which halts the machine. The next exec function should through 
	an unable to load file exception because the file is not in the test 
	directory.
*/

#include "syscall.h"

void Threadfunc(){
	Printf("Fort Test Fucntion for thread\n", 32, 0,0);
	Exit(0);
}

int main(){
	int processLocation = 0;

	
	Fork(Threadfunc, "Thread A", 8);
	/*Tests the creation of the new process and main thread*/
	processLocation = Exec("../test/testfiles", 17);

	if(processLocation != -1)
		Printf("New process location: %d\n", 26, 1, processLocation);

}
