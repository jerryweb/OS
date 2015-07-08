Part 3
 	Monitor variables 
 		* need syscalls

 		Suggest 
 		 all MVs are int arrays

 		a = CreateMV("____",/*length*/,/*array size*/);
 		 SetMV(a, /*index pos*/, /*array size*/);
 		 GetMV(______,______);
 		 DestroyMV


Swap File Stuff
Problem: Context switch on PRF /*page fault exception*/ with Random policy
	Issue: Lock will not help!		// can get a context switch where a thread 
									// only gets to run one instruction before switched

	Live Lock // all threads contend for the IPT lock
	need something that will not put the threads to sleep
	Solution: 'use' field in TranslationEntry
		- on a PFE, an eviction -IPTMiss
		- IPTLock->Acquire();
			- select a page and set the USE bit to true //meaning that this page is in use for
													//being evicted
			* if USE == true already, then select a different page
			// The 'use' flag is now atomic
		- IPTLock->Release
			.
			.
			.
		update TLB
		set use bit to false

TLB
	- on eviction, must check if it is my page because it may already be in the TLB
		* propagate the dirty bit to the IPT 
		* invalidate that TLB entry 

Data - Knowledge
	Every IPT page load
	- output in print statement: vpn, ppn, byteoffset, which file //swap, executable, neither
	Every eviction /*dirty or not dirty*/
	- output the same statements: vpn, ppn, byteoffset, file

	matmult: 37 pages 
	sort: 54 pages 

	start by incrementing numPhysPages as big as possible before it breaks and put all the output
	into a file.... run with the -d a 
	Most of the time, there is a problem with the next-to-last address because the last address will
	try and access an instruction in the data sector 

	3 important things: //this is what the tlb says 
	vpn, ppn, value //if this is a code page, the value should NEVER change 
	//take that last value and search for the corresponding vpn above in the output
	//find the last time you loaded that phyical page that was loaded into memory
	//trace the vpn and ppn to the very first time it was loaded 
	va 0x2fe3
		 |||x1	//conversion 
		 ||x16
		 |x256
		 x4096

RPCs
	Stub Generator // helps read, parse, etc. messages 
	- web services 
		* Axis
		 wsdl2java - feed this input files and it will give four classes

	Description Language 
	- request type 
	- request version?
	- formate/type/order of data 
	//this is refering to big vs little Endian systems 

	One more issue //how does the client stub know where the server stub is?
	1. how does the client stub 'know' server stub location?
		a. one method is hard coding the location 	//but there are as many client stubs 
			not flexible							// for every app
		b. Client stub gets network info @ request time => Dynamic Binding 
		 * new component: Binder
			- Track location of server stub
			- Handle client stub requests

			* Server stubs register with the binder 
			* Server stubs can also deregister with the binder //planned termination fo services
			* Binders can poll the server stubs /*similar to a ping*/ to make sure that they 
			are still in use; if no response is given after several 'pings' then the binder
			considereds it dead 
			* Load balancing
			* Authorization //server stub has to send a password with the request

