Exit/Fork/Exec
	must count the total threads in a process
				"   		"	  awake

		-Acquire/Release/Wait/Signal/Broadcast need to be changed to handle this
		//in Acquire and Wait:  
			Decrement awake count 			//add this line first
			currentThread->Sleep()

		For the other 3:

			if(WaitQueue != Empty){
				Increment awake count
				Scheduler-> ReadyToRun(	)
			}


//Add these arrays around the the 
	int x[1000];
	ScreenLineLock;
	int y[1000];

	//The definition of the process table goes into the system.h and system.cc


Physical Memory Management
	Most Basic - Uniprogramming
		1 process - 1 thread
		Loaded into Physical Memory at a time 
		-No sharing 
		-No security between program users //you are the only program

	Address
	  0 ------------
	   |Kernel O.S. | 
	  x	------------ Fence Reg 
	   |User Program|
	   |			|

	Next Simplest Way Fixed Partitions
		//divide up the memory into a set of multiple partitions of various sizes
		//small user prog loaded into a small partition and a large user prog loaded into a large partition
		//can't run a prog if there is not a partiion atleast large enough to hold that program 
		Load a userprogram into a partition at least the size of A.S. (address space)
		-------------
		|  program	|		* No sharing between partitions 
		|			|		//advantage
		|			|		+ Multiprogramming 
		-------------		+ Partitions are the security
		|  program	|		//disadvantage
		-------------		- Cannot run a userprogram larger than the largest partition
		|  program  |		- memory fragmentation -internal //when a user program is given a partition larger than the actual size of the program, so the program cannot use all of it
		|			|
		|			|
		|			|
		-------------


	Method #3 Dynamic Partitions - Base & Bounds 
		A partition is created at process startup 
		Exactly the size of the programs A.S.
		//advantage 
		+ Can run a userprogram anywhere there is enough memory 
		+ Can run 1 program up to the size of physical memory
		//disadvantage 
		- Need security mechanism between user programs
		------------- 
		|-----------| <- start
		|| p1	   ||size		
	end	|-----------| <-base		
		|			|		 bounds
		-------------		
		|  p2		|		
		-------------		
		|  p3  		|		
		|			|
		|			|
		|			|
		-------------
		 * 1 set of Base&Bounds register // This solves the securtiy issue
		 	//They hold the start and the base and the size, NOT the end (because this is virtual memory); can find the bound by size plus start
	

	Issue #1: No way to share data between user programs
		- Dynamic partitions - overlap? NO
		//cannot overlap because the top of address space is code and the bottom of the program address space is stack. 
		//do not want to overlap code with stack and vice-versa

	Method #4 - Segmentation
		-Break up the address space

						V.A.S. //Virtual Address Space
			------------- 0
			|Code		|
			-------------
			|Data		|
			-------------
			|Heap		|
			|-----------|
			|			|
			|Stack		|
			------------- N -1		


		Allocate Memory by Segmnet
			+ Segments are smaller than the entire A.S. 
			* Maybe higher use of memory

		Segments are Allocated continously - individually 
			- max number of Segments
		
						V.A.S
			------------- 0
			|-----------|
			||P1 Code  ||
			|-----------|
			|-----------|
			||P1 Data  ||
			||P2 Data  ||
			|-----------|
			|-----------|
			|| P1 S&H  ||
			|-----------|
			|			|
			|			|
			|			|
			|-----------|
			||P2 code  ||
			|-----------|
			------------- N -1

		Create a segment table 
			Base & Bounds for each segment in A.S.
		/*New*/ + Memory sharing between processes can occur
		/*New*/ + Virtual memory possible - can run a user program larger than the physical memory

			   *- External Memory fragmentation			//Fragmentation will always exist and cannot be completely fixed
				- Memory allocation is not very fast


	Paging
		Allocate memory in fixed size pieces
			all the samesize (page)
			"Locgically"divide the A.S. into pages //virtual pages
		
		*Page table maps all pages
		
		Virtual pages are the same size as physical pages
		//advantages
		+ Any virtual page can be loaded into any physical page\data
		//disadvantages
		- Internal memory fragmentation

	Paging Has A Problem
	- page table is an array (contiguous in memory) //the page table is not part of the user address space; 
													//it is also not part of virtual address space
													//contiguous becasue it is managed by the OS
		indexed by virtual page number
		managed by OS - no translation of page table

	Page Tables can be Large - 32bit OS
		2^32 bytes available = 4 GB
		How big is a page?
			assume 1 MB is size
		Max A.S.: 4096 entries

	VP:0_____________
		|			|
		|-----------|
		|			|
		|			|
		|			|  //Programs always have a page table and each page table requries space between the stack and heap (this inevitably wastes memory)
		|			|
		|			|
 VP:4096|-----------|
		|start		|
		|___________|

		int main(){
			while(true);
		}

	in address constructor: bzero zeros out all of memory //delete this once we start using fork and exec



	Method #5 - Multilevel Paging/Segmentation
		2 - level mapping
		* The last level is always paging usually at least 4 levels //including physical memory
		*Saves all the memory wasted in between 
		New Problem: 4 level translation produces 2.5 times slowdown
		//see the image (needs more data)

	Solution - Speedup address translation
		translation lookaside buffer (TLB)
		- cache of page table entries
		- indexed by virtual page numbers - contains physical page number
			can get 90% hit ratio by TLB depending on how well the programmer implemented his code
