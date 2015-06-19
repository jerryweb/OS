#include "securityinspector.h"
#include "stdlib.h"
#include "time.h"
#include "passenger.h"

SecurityInspector::SecurityInspector(int ID, Airport* AIRPORT) {
	id = ID;
	airport = AIRPORT;
	reportList = new List;
	boardingPassList = new List;
	//isBoarding = false;  //true means handling a returning passenger

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

		//remove passenger and set p point to the removed passenger in return queue
		p = (Passenger*) airport->returnQueues[id]->Remove();
		p->SetSecurityPass(true);
		p->SetBoarding();     //set boarding status to true

		printf(
				"Security inspector %d permits returning passenger %d to board\n",
				id, p->getID());

		//set state busy
		airport->securityState[id] = SC_BUSY;
		//signal passenger waiting in return queue
		airport->returnQueuesCV[id]->Signal(airport->securityQueuesLock);

		//if return queues is empty than handel normal queue
	} else if (!(airport->securityQueues[id]->IsEmpty())) {
		bool passFlag;

		//if it's normal case read securitypass status from report list
		if (airport->screenOfficerList->Size() != 0) {
			passFlag = reportList->First();
			reportList->Remove();  //remove security report form screenofficer
		}			//as we going to remove pasenger in security queue as well

		//if it's individual test for security class set it to true as default
		else
			passFlag = true;

		//remove passenger and set p point to removed passenger in normal queue
		p = (Passenger*) airport->securityQueues[id]->Remove();

		//do the security check here if he
		//assume 20% chance fail
		int randNum = rand() % 100 + 1;
		if (randNum > 0 && randNum < 21) {
			p->SetSecurityPass(false);
			passFlag = false;
		}

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
					"Security inpector %d asks passenger %d to go for further examination\n",
					id, p->getID());

		}

		//signal the passenger waiting on passenger.cc line 229
		airport->securityQueuesCV[id]->Signal(airport->securityQueuesLock);
		airport->securityState[id] = SC_BUSY;          		//set state busy

	} else { //if both security and return queues are empty
		p = NULL;
		airport->securityState[id] = SC_FREE;   //set state to free
	}

	return p;
}

void SecurityInspector::Inspect() {
	srand(time(NULL));

	while (true) {
		Passenger* p = NULL;
		airport->securityQueuesLock->Acquire();
		p = CheckForPassengers();     		//this function remove and return the first passenger in the line if there's one
											//in either security or return queue
											//the security check also happens here

		//if passenger pass security check
		//include the passengers return from questioning
		if (p != NULL && (p->GetBoarding())) {
			airport->securityQueuesLock->Release();
			airport->securityLocks[id]->Acquire();

			//wait for confirmation from passeger
			//correspond to passenger.cc line 240
			airport->boardCV[id]->Wait(airport->securityLocks[id]);
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
			airport->securityQueuesLock->Release();

		//if p = NULL, that means both normal and return queues are empty
		//put mysel on break,potential manager code here
		//passenger.cc code line
		} else {
			airport->securityQueuesLock->Release();
			airport->securityLocks[id]->Acquire();
			airport->securityFreeCV[id]->Wait(airport->securityLocks[id]);
		}
	}
}

/*void SecurityInspector::Inspect() {
 //initialize random seed
 srand(time(NULL));
 while (true) {
 //starting C.S.(1) for security line
 airport->securityLocks[id]->Acquire();
 Passenger* currentPassenger;

 //handle return queue first if it is not empty
 if (!(airport->returnQueues[id]->IsEmpty())) {
 currentPassenger = (Passenger*) airport->returnQueues[id]->First();
 currentPassenger->SetSecurityPass(true);

 airport->returnQueues[id]->Remove();
 qPassengerCount--;

 //starting C.S.(2) for ping-pong with returning passenger
 airport->rePassengerWaitInspectorCV[id]->Signal(
 airport->securityLocks[id]);
 airport->securityLocks[id]->Release();
 airport->securityLocks[id]->Acquire();
 airport->inspectorWaitRePassengerCV[id]->Wait(
 airport->securityLocks[id]);
 //ending C.S.(2)

 printf(
 "Security inspector %d permits returning passenger %d to board\n",
 id, currentPassenger->getID());

 //starting C.S.(6) to update cleared passenger count for manager
 //this lock shared by all the inspectors and manager
 airport->updateClearCount->Acquire();
 (airport->clearPassengerCount[currentPassenger->GetAirline()])++;
 airport->updateClearCount->Release();
 //ending C.S.(6)

 //if return line is empty handle the security line
 } else if (!(airport->securityQueues[id]->IsEmpty())) {
 currentPassenger =
 (Passenger*) airport->securityQueues[id]->First();

 //this flag use locally in this function
 bool passFlag = currentPassenger->GetSecurityPass();

 //if it's orginally clean from screening do wand-hand test
 if (passFlag) {
 //assume 20% chance fail the screening
 int randNum = rand() % 100 + 1;
 if (randNum > 0 && randNum < 21) {
 currentPassenger->SetSecurityPass(false);
 passFlag = false;
 }
 }

 //starting C.S.(3) for removing currentPassenger from myline
 airport->securityQueuesLock->Acquire();
 airport->securityQueues[id]->Remove();
 airport->securityQueuesLock->Release();
 //ending C.S.(3)

 //starting C.S.(4) for ping-pong with Passenger
 airport->passengerWaitInspectorCV[id]->Signal(
 airport->securityLocks[id]);
 airport->securityLocks[id]->Release();
 airport->securityLocks[id]->Acquire();
 airport->inspectorWaitPassengerCV[id]->Wait(
 airport->securityLocks[id]);
 //ending C.S.(4)

 //make decision based on the result above
 if (!passFlag) {
 printf(
 "Security inspector %d is suspicious of the passenger %d\n",
 id, currentPassenger->getID());
 printf(
 "Security inpector %d asks passenger %d to go for further examination\n",
 id, currentPassenger->getID());
 if (qPassengerCount == (-1)) {
 qPassengerCount = 0;
 }
 qPassengerCount++;
 } else {
 printf(
 "Security inspector %d is not suspicious of the passenger %d\n",
 id, currentPassenger->getID());
 printf("Security inspector %d allows passenger %d to board\n",
 id, currentPassenger->getID());

 //starting C.S.(5) to update cleared passenger count for manager
 airport->updateClearCount->Acquire();
 (airport->clearPassengerCount[currentPassenger->GetAirline()])++;
 airport->updateClearCount->Release();
 //ending C.S.(5)
 }

 //if both security and return queues are empty but
 //qPassengerCount is larger than 0, means still has
 //passenger being questioned,wait those passengers
 //to come back
 } else if (qPassengerCount > 0) {
 airport->inspectorWaitQuestioningCV[id]->Wait(
 airport->securityLocks[id]);
 //ending C.S.(1) and goes to wait

 //if it goes to else it ends
 } else {
 airport->securityLocks[id]->Release();
 //ending C.S.(1)

 //add manager control code here

 break;
 }
 }
 }*/

void SecurityInspector::AppendBool(bool* b) {
	reportList->Append(b);
}

int SecurityInspector::getID() {
	return id;
}

int* SecurityInspector::getClearCount() {
	return clearCount;
}
