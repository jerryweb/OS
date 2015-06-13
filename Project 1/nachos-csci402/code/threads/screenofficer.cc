#include "screenofficer.h"
#include "stdlib.h"
#include "time.h"

ScreenOfficer::ScreenOfficer(int ID, Airport* AIRPORT) {
	id = ID;
	airport = AIRPORT;
	airport->screenState[id] = SO_BUSY;
}

ScreenOfficer::~ScreenOfficer() {
	
}

Passenger* ScreenOfficer::CheckForPassengers(){
	Passenger* P;printf("hey\n");
    if (airport->screenQueues[id]->Size() > 0){printf("hey\n");
    	 airport->screenlineCV[id]->Signal(airport->screenQueuesLock);
        P = (Passenger*)airport->screenQueues[id]->Remove();

			        airport->screenState[id] = SO_BUSY; 
    }
    else {
    	P = NULL;
    	airport->screenState[id] = SO_FREE;
    }

    return P;

}


void ScreenOfficer::Screen(){
	Passenger* p = NULL;
	int location = 0;
	 int randNum = rand() % (101);
	bool luggageTest = true;
	while(true){
		printf("hey %p\n", airport->screenLineLock);
		airport->screenLineLock->Acquire();
		printf("hey\n");
		p = CheckForPassengers();
		airport->screenLocks[id]->Acquire();
		 
		 if(p != NULL){
		 	airport->screenLineLock->Release();
		 	airport->screenCV[id]->Wait(airport->screenLocks[id]);
		 	//Wait for passenger to acknowledge so that he can direct to proper line

		 	airport->screenLocks[id]->Acquire();
		 	//Find the shortest inspector line 
		 	airport->securityQueuesLock->Acquire();
			//this is the size and location of the smallest line 
			
			int minValue = airport->securityQueues[0]->Size();

			//need to change
			for(int i = 0; i < 3; i++){
				printf("minValue %d\n");
				//printf("Size: %d\n", securityQueues[i]->Size());
				if(minValue > airport->securityQueues[i]->Size()){
					minValue = airport->securityQueues[i]->Size();
					location = i;
				}
			}

			//add passenger to the line
			airport->securityQueues[location]->Append((void *)p);
			//printf("%s\n", );

			if (randNum > 0 && randNum < 21) {
				// P->SetSecurityPass(false);
        		luggageTest = false;
				printf("Screening officer %d is suspicious of the hand luggage of passenger %d\n",
						id, p->getID());
			} else {
				printf("Screening officer %d is not suspicious of the hand luggage of passenger %d\n",
						id, p->getID());
			}

			//add luggage test report to the list 
			// for (int i = 0; i < airport->securityInspectorList->Size(); i++) {
			// 	SecurityInspector *S = (SecurityInspector*) airport->securityInspectorList->Remove();
			// 	airport->securityInspectorList->Append((void *) S);
			// 	S->AppendBool(luggageTest);
			// }
			//airport->securityQueues[location]->AppendBool(luggageTest);
			//Notify passenger of inspector line and add him to the ready queue
			airport->screenCV[id]->Signal(airport->screenLocks[id]);
			printf("Screening officer %d directs passenger %d to security inspector %d\n",
				id, p->getID(), location );
			airport->securitlineCV[location]->Signal(airport->securityQueuesLock);
			//Allow passenger, then inspector to attempt to aquire the locks 
			airport->screenLocks[id]->Release();
			airport->securityQueuesLock->Release();

		 }

		 else{
		 	airport->screenLineLock->Release();
		 	airport->screenCV[id]->Wait(airport->screenLocks[id]);
		 }


	}
}

// void ScreenOfficer::Screen() {
// 	//initialize random seed
// 	srand(time(NULL));
// 	//keep checking his own line
// 	while (true) {
// 		//the index in C.S. parenthesis is for this file only

// 		//start C.S.(1) between passenger
// 		//assuming officer ID is the same as his index in screenLocks array
// 		int myLine = id;
// 		airport->screenLocks[myLine]->Acquire();
// 		Passenger* currentPassenger;

// 		if ((airport->screenQueues[myLine]->Size()) != 0) {
// 			currentPassenger = (Passenger*) airport->screenQueues[id]->First();

// 			//assume 20% chance fail the screening
// 			int randNum = rand() % 100 + 1;
// 			if (randNum > 0 && randNum < 21) {
// 				currentPassenger->SetSecurityPass(false);
// 				printf(
// 						"Screening officer %d is suspicious of the hand luggage of passenger %d\n",
// 						id, currentPassenger->getID());
// 			} else {
// 				printf(
// 						"Screening officer %d is not suspicious of the hand luggage of passenger %d\n",
// 						id, currentPassenger->getID());
// 			}

// 			//remove the passenger in current line

// 			//starting C.S.(2) to remove passenger from current line
// 			airport->screenQueuesLock->Acquire();
// 			airport->screenQueues[myLine]->Remove();
// 			airport->screenQueuesLock->Release();
// 			//ending C.S.(2) to remove passenger from current line
// 			//}

// 			//starting C.S.(3) to find shortest security line and append
// 			airport->securityQueuesLock->Acquire();

// 			int shortest = 0;		//shortest line's id
// 			int minimumSize = -1;		//for comparsion in the following loop

// 			//find the shortest line
// 			for (int i = 0; i < 3; i++) {
// 				if (minimumSize < 0
// 						|| minimumSize > airport->securityQueues[i]->Size()) {
// 					minimumSize = airport->securityQueues[i]->Size();
// 					shortest = i;
// 				}
// 			}

// 			airport->securityQueues[shortest]->Append(currentPassenger);
// 			//also update the queueIndex for current passenger
// 			currentPassenger->SetQueueIndex(shortest);

// 			airport->securityQueuesLock->Release();
// 			//ending C.S.(3)

// 			//signal the passenger
// 			airport->passengerWaitOfficerCV[myLine]->Signal(
// 					airport->screenLocks[myLine]);
// 			airport->screenLocks[myLine]->Release();

// 			//wait for passenger thread
// 			airport->screenLocks[myLine]->Acquire();
// 			airport->officerWaitPassengerCV[myLine]->Wait(
// 					airport->screenLocks[myLine]);
// 			//ending C.S.(1)

// 			//airport->screenLocks[myLine]->Acquire();
// 			//Assuming the index in securityQueues array is the same as security inspector's id
// 			printf(
// 					"Screening officer %d directs passenger %d to security inspector %d\n",
// 					id, currentPassenger->getID(), shortest);
// 			//airport->screenLocks[myLine]->Release();
// 		}
// 	}
// }
