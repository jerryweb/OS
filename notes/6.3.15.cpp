There will be a nachos synchronization problem for the airport. We will have to implement a new thread into the code


nachos-csci402
	|
	|
   code - 1st time- gmake
   * in the code folder there should be files: threads, test, vm, filesys, network, and userprog
   for threads: project 1 is in this... compile and run 

   threads, vm, filesys, network, and userprog are nachos and the test is nachos user programs


2 ticket-typs: economy and executive

airline 1: each airline has 5 queue (example) and one executive queue, however, executive queue has priority. Each queue
			has a count 
			can make an array of size 6 for the monitor variable where the length of the line is in the corresponding each queue
			and set the executive queue to index 0
			the economy queues and the executive queue have seperate locks (2 locks in total, this is one option)
			* more locks are good if it doesn't create a race condition 

airline 2

airline 3


Passenger
	LineLock->Aquire();
		int myLine = 0;
		int shortCount = LineCount[0];
		for (){

		}

		if(shortCount > 0){
			LineCV[myLine]->Wait(LineLock)
		}

	LineLock->Release();
	//go up to check-in staff
	cisLock[myLine]->Aquire();
	//write ticket info to a cis specific location 
	cisCV[myLine]->Signal(	);

	cisCV[myLine]->Wait(	);

	//read seat # 
	cisCV[myLine]->Signal(	);
	cisLock[myLine]->Release();



********Wait and Signal calls MUST STAGGER (wait and signal must alternate)********

Check-In Staff
	while(true){
		if anyone in executive queue
			check them in
	}

	else 
		do my economy queue

	//The is the start of the 1st critical section 
	LineLock->Aquire();
		if(LineCount[myLine] > 0){
			LineCV[myLine]->Signal(LineLock); //signaling doesn't give it access to the CPU right away
		}

		//could be a context switch here
		cisLock[myLine]->Aquire(); //this must go here before you call wait in the 2nd critical section 
	LineLock->Release();
	//end of first critical section

	//beginning of 2nd critical sectoin
	//Wait for passenger ticket info
	cisLock[myLine]->Wait(  );
	//check in passenger 
	-Give boarding pass-seat # //this info is transeferd to the passenger (can be part of the ticket info struct we create
	cisCV[myLine]->Signal(	);
	cisCV[myLine]->Wait(	); //must wait for the passenger to read the data
	//end of 2nd critical section 

	cisLock[myLine]->Release();



Condition::Wait(Lock* lock){


	*lock->Release();
	currentThread->Sleep();
	lock->Aquire(); //Goes to sleep waiting on lock 
		|
		|
		|
	//give ticket info to Check-in Staff
}



void doit(void* myFunc){ // this is an example of function pointer passing 
	myFunc();
}


for(int i=0; i < maxPassengers; i++){
	Thread *t = new Thread("______");
		t ->Fork(passenger/*fucnction pointer*/,i); //don't put parrentheses after the passenger function call 
												//i is also the line the employee is assinged as well as the employee's ID when you are instantiating the emplyees 
}


void passenger(int myNumber){
	Ticket myTicket

}



nachos -rs 100 -T -P2//100 is given to Srand 
	//adding -rs will allow for context switiching preemptively, if you you get an error on a given random seed value, use the same value to fix the error
	//test multiple values 
	preemptive 

	Part 1: Test suite




main.cc
int main(	){
	for(	){//iterates over all command line args
		if( -rs )

		else if(/*arg is -T*/){
			TestSuite();
		}	
	}
}


to test the that the passenger gets into the shortest line; all you need is a passenger thread and hardcoded line count values 

**** for the system test, DO NOT PROMPT THE USER FOR THE NUMBER OF THREADS

for each test, terminate nachos once the test is finished 


Semaphores
	1 mechanism for handling ...
	have an int value 
	0 -> positive # // no negative numbers 
	two operations 
		up 		v //increments the value of the semaphore by 1; if some thread waiting-wake1up
		Down	p //decrements semaphore value if > 0; if value < 1, go to sleep first


Mutual Exclusion 
	use a binary semaphore
		initial value is 1
	To enter critical section
		do a Down 
	To exit critical section
		do an Up



many threads all doing the same thing
	Whil(true){
		sem.Down();
			|
			|
		sem.Up();
			|
			|

	}


synchronization
	- any initial value
	- Down or Up


Soda Machine
	capacity 10
	student - take 1 Soda
	SodaGal - add 1 Soda

	-When must SodaGal wait?
	1 - machine full    empty - start @ 10
	Student 
	-machine empty
	2 full -starts @ 10
	3 mutex


SodaGal
	while(true){
		* empty.Down();
		mutex.Down();
		//add 1 soda
		mutex.Up();
		* full.Up();
	}

Student
	* full.Down();
	mutex.Down();
		take 1 soad
	mutex.Up();
	* empty.Up();
