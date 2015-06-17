#include "screenofficer.h"


ScreenOfficer::ScreenOfficer(int ID, Airport* AIRPORT) {
	id = ID;
	airport = AIRPORT;
	airport->screenState[id] = SO_BUSY;
	printf(" screen airport pointer: %p\n", airport);
}

ScreenOfficer::~ScreenOfficer() {
	
}

Passenger* ScreenOfficer::CheckForPassengers(){
	Passenger* P;
    if (!airport->screenQueues[id]->IsEmpty()){
    	 airport->screenQueuesCV[id]->Signal(airport->screenQueuesLock);
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
		printf(" screen airport pointer: %p\n", airport);
//		airport->screenLineLock->Acquire();
		
		airport->screenQueuesLock->Acquire();

		p = CheckForPassengers();
		airport->screenLocks[id]->Acquire();
		 
		 if(p != NULL){
		 	airport->screenQueuesLock->Release();
		 	airport->screenCV[id]->Wait(airport->screenLocks[id]);
		 	//Wait for passenger to acknowledge so that he can direct to proper line

		 	airport->screenLocks[id]->Acquire();
		 	//Find the shortest inspector line 
		 	airport->securityQueuesLock->Acquire();
			//this is the size and location of the smallest line 
			
			int minValue = airport->securityQueues[0]->Size();

			//need to change
			for(int i = 0; i < 3; i++){
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
			 for (int i = 0; i < airport->securityInspectorList->Size(); i++) {
			 	SecurityInspector *S = (SecurityInspector*) airport->securityInspectorList->Remove();
			 	//--->something definitely wrong here<--------------
			 	airport->securityInspectorList->Append((void *) S);
			 	//--------><--------------------
			 	S->AppendBool(&luggageTest);
			 }
			//Notify passenger of inspector line and add him to the ready queue
			airport->screenCV[id]->Signal(airport->screenLocks[id]);
			printf("Screening officer %d directs passenger %d to security inspector %d\n",
				id, p->getID(), location );
			//airport->securityQueuesCV[location]->Signal(airport->securityQueuesLock);
			//Allow passenger, then inspector to attempt to aquire the locks 
			airport->screenLocks[id]->Release();
			//airport->securityQueuesLock->Release();

		 }

		 else{
		 	airport->screenQueuesLock->Release();
		 	airport->screenCV[id]->Wait(airport->screenLocks[id]);
		 }


	}
}

