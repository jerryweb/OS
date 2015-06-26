/*Exit Test
	This test takes in a test file, and essentially creates a new process.
	It cerates a new thread and forks it before the creation of a new process.

*/

#include "syscall.h"

void Threadfunc(){
	Printf("Fort Test Fucntion for thread A\n", 32, 0,0);
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
