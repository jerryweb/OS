
/* matmultExec
    exec 2 matmults
*/

#include "syscall.h"

int main()
{
	Exec("../test/matmult", 15);
    Exec("../test/matmult", 15);
}
