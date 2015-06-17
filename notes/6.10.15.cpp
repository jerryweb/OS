To debug multi-threaded code, you have to use print statements with the name of the thread and the id of the thread
If you have a thread stopping too early, the problem usually is around a Wait(	) 
condition

put a print statemnet before and after

			A 										B
cout << "Just before x->Wait\n";		cout << "Just before x->Signal\n";
x->Wait(	);							x->Signal(	);
cout << "Just after x->Wait\n";			cout 


/varltmp/spodl/csci350
for submitting:
	zip: code dir
			*group file with the names of people in the group
			*write up
		 
		 threads dir

	BEFORE you zip do a gmake clean command in threads and then another gmake. Then zip
	then email professor the zip file 


Project 2 (3 parts)
	parts 1 & 2 - OS code
		- system calls
		- multiprogramming 

	part 3 - convert airport simulation into nachos user program


Memory Management
	Nachos - uniprogrammed 

	Header - (size of the code)
			\
			 ----------> (start of init vars)
	-------------
	|Header		|
	-------------		(start _____________)
	|code      	|		(size of unninit vars)
	-------------
	|Init      	|				|
	|Vars     	|				|
	|			|				|
	|			|				|
	-------------				|
	Executable file             |
								V
							-------------		
							|code      	|
							------------		
							|Init      	|
							-------------		
							|Vars     	|		
							|			|
						--	|			|
							-------------
							Virtual address space -> from compiler
//page 
//numbers
	To Execute - A.S.	//this is divided up into memory and loaded into physical memory 
0	-------------	0 	// N address spaces 	
	|code      	|
1	------------		
	|I.D.      	|
2	-------------		
	|U.D. 		|		
3	-------------
	|			|		//Nachos does not have a heap
	|		^	|
	|stack	|	|
	-------------	N -1 // virtual pages


	int k[10000000];

	int i = 5;			//initialized value, whcih is written to the executable
	int j;				//uninitialized value, which is not written to the executable

	int main(){			
		cout << i;		//This should print 5
		cout << j;
	}

	The smallest amount of memory i can allocate to a user program is a page

	How to load executable into memory?
		memory is divided into pages
		memory manager allocates memory 1 page at a time

		To get the executable contents into memory, we copy 1 page worth of contents to memory at a time
			- to an unused page
		*The OS must keep track of used/unused pages & process owner
		*The Virtual address space can be mapped to any physical address space
		*pages copied to physical memory are called physical pages!


	Page Table
		- map of virtual to physical pages
		- converts references from virtual pages to physical pages 
		- Array
		- managed by O.S.
		- 1 entry for each virtual page
		- indexed by virtual page number

	a = b + c; 			//This has 4 page table accesses
		+  instruction
		read b
		read c
		write a

	//Lets say c is at address 1000 and page size is 100
		- to get v.p.: 1000/100 = 10, so at virtual page 10
		// now go to index page 10 in page table and then find the physical page number
		// DO NOT SHARE PAGES, even if the entire page is not used 

	Nachos has a page table
		- array of TranslationEntry //this is a class
		- "nachos (in userprog) will use a process' page table to do address translation" 
		Your job is to copy data from the executable into physical memory correctly & populate the page table properly


	Solution: when a virtual page needs to be loaded into memory
		//nachos virtual memory is always divided into 8 pages
	(@ startup) you will (1.)find unused memory page & (2.) mark it as used, & (3.) copy one page worth of content from the Executable
	file (4.) into the just allocated physical memory page. This is done till all the virtual pages are allocated.

	Nachos Page table 
		Is part of AddressSpace class
		- in userprog
		- addressspace.h/.cc 
		Loading of address space must occur before user program runs
		- in addrspace constructor


	 Finding an Unused Memory Page
		Use BitMap class 
		Has 2 methods:
			Find - returns an int
				 - an unused physical page #
			Clear(___) //to free a  "  "
		
		You will create 1 BitMap object 				// The contstructor takes an int of how many pages you have
			for all nachos - system.h/.cc 				//extern statements are located in system.h
							 extern BitMap memory[]		//real declaration is in the .cc file 
			a.cc 										//when including system.h in this file, it's scope only cover this specific file 
			Size of BitMap - same as number of physical pages 
			- in mahcine dir/machine.h 	NumPhysPages 512 //This won't be big enough for part 3 of project 2

			memMap = new BitMap(NumPhysPages);			//need to pass NumPhysPages as the arg for the constructor
			need a lock


	 Watch Out in AddrSpace Constructor
		Nachos does not copy from executable to memory 1 page at a time 

		//This works for single threads 
		for(int i =0; i < numPsages; i++){				//i is the virtual page number
			//populate the page table
			 executable->ReadAt(_,_,/*nofftt.code.inFileAddress + i*PageSize*/);
		}
		//DO NOT WORRY ABOUT PAGE BOUNDARIES WITH DATA AND CODE OVERLAPPING
		//You will need to comment out the following two sections of code after the for loop. This is for uniprogramming
		[copy code segment]	- executable->ReadAt(_/*where to read to*/,_/*how many bytes to read, nachos page is 128 bytes*/,_/*where to read from*/); //take one of these statements and put it in the for loop 
		[copy init segment]


		int ppn = memMap->Find();
		mainMemroy[ppn * PageSize]		//This is used to access the physical memory data


	 Stack Cannot Be Shared
		Need 1 stack per thread 
		Stack is 8 pages - only 1 page exists now: User StackSize 1024
			StackReg
		
		2 Choices:
			1. change page table to a start with X 		//(50) this adds 50*8  = 400 pages to the page table
				sets of 8 stack pages 					//divRoundUp
				- each time a new thread is to be created (Fork syscal);, set StackReg for that thread
			
			2. leave page table alone in address space constructor
				On each system call, make a new page table 8 pages bigger than current, copy all data from old page table to new,
				allocate 8 pages of memory for new stack

				newPageTable:							//This is done at the end to create a new page table
				delete pageTable;
				pageTable = new pageTable;

*Start looking at address space code tomorrow, and look into working on Fork and Exec. FIRST thing to start with is to change the 
constructor to run one thread at a time. There are 5 system calls in syscall.h. At the end of the weekend, the simple user programs work 