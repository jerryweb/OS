The more levels, the smaller the amount of wasted memory
most OSes have 4 levels on translation

TLB 
	translation lookaside buffer 		//not a data cache 
		- cache of page table data - index by initial page #
		- 64 entries
		- runs close to the speed of CPU
		- most recently used pages		//the last 64 pages used by the user program 
	
	What to do on a context swwitch?
		1. On eviction - save TLB state			//We dont do this 
			on CPU re-entry, reload TLB state

		2. Invalidate TLB 
	
	Virtual Memory Worst Case Scenario
		A Virtual Address needed by CPU is on a Virtual page not in the TLB
		Then we go to the translation tables - one(or more) of the translation
		tables is on disk(not in memory)
			- we must load into memory - multiple milliseconds //or microseconds
		We find... the Virtual page we needed was already in memory 

		Rule: We NEVER want to have to load a translation table from disk when
			the needed Virtual page is in memory


	Need another data structure
		We need a single table that maps physical memory
		- maps physical pages to Virtual pages 		// indexed by physcial page number since there is only
													// one table for the OS
			* called an Inverted Page Table (IPT)
			 - always in memory and only 1 for the OS
			 - has 1 entry for each physcial memory page
			 - managed by the OS
			Hash table 
			 - key - value							// cant just be the vpn becasue everyone has a vpn 0
			 VPN & process id 						// This combination is what we use
			 										// wastes a little memory but gives O(1) lookup time 
		// We need at min these 5 pieces of info
			ppn				1 byte
			VPN 			4 bytes
			process ID 		4
			valid bit		1
			dirty bit		1
						= 14 bytes

MMU (Memory Management Unit)
/*step*/0. receive & validate Virtual Address
	
	1. if valid, check TLB
		- IF found, compute physcial Address - Done
	
	2. If not in TLB, check IPT (inverted page table)
		- IF found, compute physcial Address
		- update TLB - Done
		//steps 0 through 2 are done at CPU speed 
		// This is called a page fault if you can't find it in the IPT
	3. If not in IPT, use translation tables
		a. check tables for the needed Virtual page number
		b. load the needed page into an availabel memory page (cant runout of memory)
		c. update IPT & TLB
		d. update translation tables
		e. Return to user mode and rerun the actual instruction 
		// if you have a = b, there could be 3 memory calls which require you to do 
		// these 3 times for the instuction and the a and b variables


Demand Page Virtual memory   	// essentially says that he wont load any physical page into virtual memory until its needed
* Dont preload any user memory pages
	- wait for a pagefault 

	What if physical memory is full or a pagefault?
	  we evict a page of memory - which one?
	  * What if page to evict is dirty?
	  	- must save this page
	  	- need a special file (reserved space on the disk) controlled by the OS
	  	// called a swap file; 1 for the OS

	  	int a = 5;
	  	int main(){
	  		cout << a;
	  		a = 10;
	  		// If we evict the page with a set to 5, the next time we run the prog, it will print 10
	  		// Cannot write to the exicutable 
	  		cout << a;
	  	}

	Page Replacement Policy
	 Global Decisions
	 	- if a page is dirty/not dirty?
	 	- Do we limit processes to some # of pages?
	 	 - local vs Global Replacement			// global means evict any page in memory
												// local means evict a page in your own address space
		 	Random: pick some random page number
		 	- Not based on usage 
		 	+ simple

		 	FIFO: Replace the page that has been in memory the longest
		 	- not based on usage
		 	* Beladys anomaly
		// In project 3, we will have to implement Random and FIFO

		 	Optimal (or Min): Replace the page not needed for the longest time 
		 	- not imeplementable 		- refernce string (set of all the vpn the program accessed while it was running)

			Aging Algorithms 
				Age - how long since a physical page has been used
				Replace the physical page not "accesed" for the LONGEST time

				// all of these bits are hardware
				clock 1-bit
				second chance 2-bits
				4-bits
				8-bits
				16/32 - Least Recently Used (LRU)

	Swap file
	 A cache of dirty, evicted memory pages
	 a paged structure
	 we write to an unused "page" if not in swap
	 translation tables will have swap location