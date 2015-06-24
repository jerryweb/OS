#include "securityinspector.h"
#include "stdlib.h"
#include "time.h"
#include "passenger.h"

SecurityInspector::SecurityInspector(int ID, Airport* AIRPORT) {
	id = ID;
	airport = AIRPORT;
	reportList = new List;
	boardingPassList = new List;

	clearCount = new int[airport->numAirlines];

	for (int i = 0; i < (airport->numAirlines); i++) {
		clearCount[i] = 0;
	}
}

SecurityInspector::~SecurityInspector() {

}

Passenger* SecurityInspector::CheckForPassengers() {

	Passenger* p = NULL;

	//handle return queues first if it's not empty
	if (!(airport->returnQueues[id]->IsEmpty())) {

		//set state busy
		airport->securityState[id] = SC_BUSY;

		//remove passenger and set p point to the removed passenger in return queue
		p = (Passenger*) airport->returnQueues[id]->Remove();

		p->SetSecurityPass(true);
		p->SetBoarding();     //set boarding status to true

		printf(
				"Security inspector %d permits returning passenger %d to board\n",
				id, p->getID());

		//signal passenger waiting in return queue
		airport->returnQueuesCV[id]->Signal(airport->securityLocks[id]);

		//if return queues is empty than handel normal queue
	} else if (!(airport->securityQueues[id]->IsEmpty())) {
		airport->securityState[id] = SC_BUSY;          		//set state busy

		airport->securityQueuesLock->Acquire();
		bool passFlag;

		//if it's normal case read securitypass status from report list
		/*if (airport->screenOfficerList->Size() != 0) {
		 passFlag = reportList->First();
		 reportList->Remove();  //remove security report form screenofficer
		 }			//as we going to remove pasenger in security queue as well

		 //if it's individual test for security class set it to true as default
		 else*/
		//passFlag = true;
		//remove passenger and set p point to removed passenger in normal queue
		p = (Passenger*) airport->securityQueues[id]->Remove();
		airport->securityQueuesLock->Release();

		airport->securityLocks[id]->Acquire();
		passFlag = p->GetSecurityPass();

		//do the security check here
		//assume 20% chance fail
		int randNum = rand() % 100 + 1;
		if (randNum > 0 && randNum < 21) {
			p->SetSecurityPass(false);
			passFlag = false;
		}

		passFlag = false;

		//make decision based on result
		if (passFlag) {
			//if pass set securitypass to true
			//set boarding status to true
			p->SetSecurityPass(true);
			p->SetBoarding();

			printf(
					"Security inspector %d is not suspicious of the passenger %d\n",
					id, p->getID());
			printf("Security inspector %d allows passenger %d to board\n", id,
					p->getID());

		} else { //if fail set securityPass to false
			p->SetSecurityPass(false);

			printf("Security inspector %d is suspicious of the passenger %d\n",
					id, p->getID());
			printf(
					"Security inspector %d asks passenger %d to go for further examination\n",
					id, p->getID());

		}

		//signal the passenger waiting on passenger.cc line 229
		airport->securityQueuesCV[id]->Signal(airport->securityLocks[id]);

	} else { //if both security and return queues are empty
		airport->securityState[id] = SC_FREE;   //set state to free
		p = NULL;
	}

	return p;
}

void SecurityInspector::Inspect() {
	srand(time(NULL));

	while (true) {
		Passenger* p = NULL;

		airport->securityLocks[id]->Acquire();
		p = CheckForPassengers(); //this function remove and return the first passenger in the line if there's one
								  //in either security or return queue
								  //the security check also happens here

		//if passenger pass security check
		//include the passengers return from questioning
		if (p != NULL && (p->GetBoarding())) {

			//wait for confirmation from passeger
			//correspond to passenger.cc line 240
			airport->securityFinishCV[id]->Wait(airport->securityLocks[id]);
			airport->securityQueuesLock->Acquire();

			//update clear count here
			int airlineP = p->GetAirline();
			(clearCount[airlineP])++;

			//handing over boarding pass here
			BoardingPass bp = p->GetBoardingPass();
			boardingPassList->Append(&bp);

			airport->securityQueuesLock->Release();

			//if fail the security pass, go to next cycle
		} else if (p != NULL) {
			airport->securityLocks[id]->Release();

			//if p = NULL, that means both normal and return queues are empty
			//put myself on break,potential manager code here
			//passenger.cc code line
		} else {
			//printf("((((((((((((((((((9security %d on free wait\n",id);
			//airport->securityFreeCV[id]->Wait(airport->securityLocks[id]);
			airport->securityQueuesCV[id]->Signal(airport->securityLocks[id]);
			airport->returnQueuesCV[id]->Signal(airport->securityLocks[id]);
			airport->securityFreeCV[id]->Wait(airport->securityLocks[id]);
			//airport->securityLocks[id]->Release();
			//currentThread->Yield();
		}
	}
}

void SecurityInspector::AppendBool(bool* b) {
	reportList->Append(b);
}

int SecurityInspector::getID() {
	return id;
}

int* SecurityInspector::getClearCount() {
	return clearCount;
}
