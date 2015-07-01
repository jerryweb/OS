Demand Page virtual memory
MMU
	0. Recieve & validate virtual address
	1. look in TLB (for VPN)
	;
		-if found, compute physical adddress
	2. if not in TLB, look in IPT(pp to vp)
	;
		-if found,
			- update TLB
			- compute P.A. - Done
	3. Look in translation tables to find 
		-location of needed VP 
			* 3 locations: executable, swap, not on disk
	4. Find an unused physical memory page 					// **** the page we select may be from a different process (in reference to the steps below)
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
	Task: Which thread gets the CPU next?
	  Policy: Rules			Traffic Laws
	  Mechanism: Implementation

	5 Possible Goals
		Fairness: Everyone gets about the same amount of CPU time
		Efficiency: Keeps as many computer resources as busy as Possible
		Response Time: Favor (or not) interactive jobs
		Turnaround Time: Favor (or not) background jobs
		Throughput: Complete as many jobs as possible in some time frame

	Issue: Do jobs run to completion, or not?
		- Non-preemptive Scheduling //if you allow completion of the job
		- preemtive Scheduling		//if you do not allow completion of the job
			*how long is a time slice?
				- if too long, interactive use can suffer
				- if too short, unecessary context switches
	/**** Policies ****/
		Non-preemptive: job that waited the longest gets the CPU next
		+ fair, no starvation
		+ simple to implement
		- short jobs can get 'stuck' behind long jobs  
	
		Round Robin
		*preemtive first come, first serve
			Jobs run until their time slice expires OR they perform a slow operation
		+ fair
		+ short jobs do not get stuck behind long jobs
		- unnecessary if all jobs are 'long' & about the same run time

		Shortest Job First
		  Maximizes throughput
		  Non-preemptive
		  Run the job requiring the least CPU time
		- Not fair - starvation
		- Not implementable

		Shortest Remaining Time to Complete First
		  preemtive
		  How much of a jobs time slice has it been using 
		  	- Scheduler keeps a "sliding window"  of the % utilization of a time slice
		  Job with the smallest utilization will get the CPU next
		+ implementable
		- lots of overhead
		- not fair 

		Priority-Based Scheduling
			priortiy: Some jobs are "favored" over others
			policy: Higher priortiy jobs run before lower priortiy jobs
			Issue: How many priorites?		// Ready Queue for each priority
				3-5 						// Each ready queue can have a different scheduling policy
				- not fair 					// Different time slice
			Issue: change over time?
				//static vs dynamic