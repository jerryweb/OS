New Solution - Use a lock
	Lock milklock;
		algorithm
		main()
			milklock.Acquire();    //atomic
			if(nomilk){			//critical secition 
				buy milk;
			}					//critical secition 
			milklock.release(); //atomic 




Lock Implementation 
Rule: Aquire & Release – atomic
	Use interrupt disabling 
	Pattern
	  Disable interrupts 
	  	carryout tasks
	  restore interrupts




Nachos Interrupt Class
	Already exists 
	Interrupt object

Disable: IntStatus old = interrupt->SetLevel(IntOff); 

Restore: interrupt->SetLevel(old);

//myself represents the thread in the CPU	

No Busy Waiting 
	Approach: Sleep/wakeup
	Sleep: A thread "state"	where it cannot gain access to the CPU 
		* on its own
		* or by the scheduler
	
	Wakeup: the event a thread was waiting for has occured 

	sleep Implementation:
		for a thread to gain CPU access
			* it must be in the ready queue
			* in the ready State

	Goal #1: Removing a thread from the ready queue
			currentThread->Sleep(); 	//CurrentThread is a pointer to the thread in the CPU

	Goal #2: Use a seperate queue
			* wait queue 				
			* $pecific to each lock
			Add a waitqueue to lock class
			Be sure to add the currentThread to the waitqueue BEFORE currentThread->Sleep()

	Wakeup Implementation 
	1. Wakeup 1 sleepy thread - if one exists 
	2. Give them ownership of the lock 
	3. Put them in Ready Queue in Ready State
		scheduler->ReadyToRun( /*thread pointer*/); 
	Locks have 2 states
		* FREE - available 
		* BUSY - not available

Part 1 of Project 1
	void Lock::Aquire(){
		disable interrupts
		if(/*I'm the lockowner */){
			//not a problem 
			restore intterupts 
			return; 
		}

		if(/*lock is available*/){
			//I can have the lock
			make lock BUSY 
			make myself lock owner 
		}	

		else{//lock not available 
			//put myself to sleep 
			add myself to lock wait queue //the wait queue is a queue of thread pointers 
			currentThread->Sleep();
		}

		restore interrupts
	}

	//can use the List class in Nachos 

	void Lock::Release(){
		Disable interrupts
		if(/*I'm not the lock owner*/){
			print an error message 
			restore interrupts
			return;
		}

		if(/*is any thread waiting */){
			* Rmove 1 thread from wait Queue 
			* call scheduler-ReadyToRun();
			* make them the lock owner
		}

		else{
			make lock FREE
			remove lock ownership // set lock thread pointer back to null
		}

		retore interrupts
	}

	

	Lock* l;					
		.							|	.
		.							|	.
		l->Aquire();				|	doit(); //this has *l->Aquire ..... l->realease 
			doit();					|





	Solving Sequencing 
		Semaphores 
		Condition varibles  // there is a condition class in Nachos similar to the lock class

	Condition Varibles
		Dont have a state like locks
		Have 3 operations 
		Wait: put myself to sleep waiting on some condition
		Signal: wakeup 1 sleeping thread //if there is a sleeping thread
		Broadcast: wakeup all sleeping threads


	void Condition::Wait(lock* Lock){
		disable interrupts
		if(lock==NULL){
			print message
			restore intterupts
			return;
		}

		if(waitingLock == NULL){//frist thread to call wait
			// waitingLock is for lock* tracking 
			waitingLock = lock;
			the lock used with this condition varible		
		}

		if(waitingLock != lock){//lock passed in doesnt match the lock already used
			print error message 
			restore interrupts
			return;
		}

		lock->Release(); // exit the critical section so 
		add myself to condition varible wait queue and go to sleep
		lock->Aquire();
		restore interrupts
		return;
	}


	myLock->Aquire();
		.
		.
		if(/* I can't proceed*/){
			myCV->Wait(myLock); //myCV is my condition varible 
			//don't wait in the critical section

		}	
		.
		.
		myLock->Release();


	void Condition::Signal(lock* Lock){
		disable interrupts
		if(/*No waiting threads*/){
			restore interrupts
			return;
		}

		if(waitingLock != lock){
			print error
			restore interrupts
			return;
		}

		//wakeup 1 waiting thread
		remove 1 thread from the condition varible wait queue
		scheduler->ReadyToRun

		if(/*no more waiting threads*/){
		waitingLock = NULL;
		}

		restore interrupts
	}


	void Condition::Broadcast(lock* Lock){
		while(/*there ARE waiting threads*/){
			Signal(lock);
		}
	}


	Monitors 
	* surround a critical section with a monitor 
	* Monitors have 3 parts 
		- 1 lock
		- 1 or more condition varibles
		- monitor varibles 
			* data we use to make Sequencing desicions 

	Monitor Form
	Aquire the monitor lock
	use/modify varibles to make Sequencing desicions
	release monitor lock


	Grocery Store Checkout
	- multiple cashiers 
	- each has their own line
	- customer will pick the shortest line 
		* person at the cashier is in line
		
		Use a Monitor
			Lock* lineLock;
			//Lock* lineLock = new Lock("___"); // can do this in Nachoes
			//1 or more - cashier busy-only 1 inline
			//0 - casheir available
			// you need as many locks as you have cashiers
			Condition* lineCV[5]; // Condition class does not have a default constructor 
			int lineCount[5]; //should initialize all these to zero

			Customer
				lineLock->Aquire();
				int ShortCount = LineCount[0];
				int myLine = 0;
				for(int i != 0,i<5;i++){
					if(LineCount[i] < ShortCount){
						ShortCount = LineCount[i];
						myLine = i;
					}
				}
				//Found a line 
				if(ShortCount > 0){
					lineCV[myLine]->Wait(LineLock);
				}
				//my turn
				LineLock->Release();
				//Begin Cashier itneraction critical section


			Cashier //have thier own variable called myLine, which is the line they are assigned to 
				while(true){
					LineLock->Aquire();
					if(LineCount[myLine] > 0){
						lineCV[myLine]->Signal(LineLock);
					}
					LineLock->Release();
					//Begin Cashier interaction critical section
				}