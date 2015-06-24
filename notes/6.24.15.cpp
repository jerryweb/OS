Demand Page virtual memory
MMU
	0. Recieve & validate virtual address
	1. look in TLB //for VPN
		-if found, compute physical adddress
	2. if not in TLB, look in IPT(pp to vp)
	;
		-if found,
			- update TLB
			- compute P.A. - Done
	3. Look in translation tables to find 
		-location of needed VP 
			* 3 locations: executable, swap, not on disk
	4. Find an unused memory page 					// **** the page we select may be from a different process (in reference to the steps below)
		a. if memory full, select a page to evict - some policy
			- if page is dirty, write to swap file									//now translation table must be updated
			- must update translation tables - to contain swap file location 		//need location and bool of where it is 
					//make sure you use the correct page table, not just currentThread->pagetable
					//Must look in IPT for the correct process that owns the page table
	5. 'load' in needed virtual page to the physical page from step 4
	6. Update Inverted Page table
	7. Update TLB
	8. Update translation tables for currect process 	//valid bit set to true, set physical page
	9. Restart the userprogram


CPU Scheduling
	