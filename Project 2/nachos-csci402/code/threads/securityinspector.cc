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

void SecurityInspector::Inspect() {
	srand(time(NULL));

	while (true) {
		Passenger* p;

		//start 1st C.S.
		airport->securityQueuesLock->Acquire();
		//if someone return from questioning
		if (!(airport->returnQueues[id]->IsEmpty())) {
			//start 3rd C.S. (corresponds to passenger code)
			airport->securityLocks[id]->Acquire();
			airport->securityState[id] = SC_BUSY;
			p = (Passenger*) airport->returnQueues[id]->Remove();
			airport->securityQueuesCV[id]->Signal(airport->securityQueuesLock);
			airport->securityQueuesLock->Release();
			airport->securityCV[id]->Wait(airport->securityLocks[id]); //end 3rd C.S.

			//start 2nd C.S. (corresponds to passenger code)
			airport->securityLocks[id]->Acquire();

			//update clear count here
			airport->securityManagerLock->Acquire();
			int airlineP = p->GetAirline();
			(clearCount[airlineP])++;
			airport->securityManagerLock->Release();

			//handing over boarding pass here
			BoardingPass bp = p->GetBoardingPass();
			boardingPassList->Append(&bp);

			airport->securityCV[id]->Signal(airport->securityLocks[id]);
			printf(
					"Security inspector %d permits returning passenger %d to board\n",
					id, p->getID());
			//wait for passenger's acknowledgement
			airport->securityCV[id]->Wait(airport->securityLocks[id]);  //end 2nd C.S.

		//normal passenger
		} else if (!(airport->securityQueues[id]->IsEmpty())) {
			//start 2nd C.S.
			airport->securityLocks[id]->Acquire();
			airport->securityState[id] = SC_BUSY;
			p = (Passenger*) airport->securityQueues[id]->Remove();
			airport->securityQueuesCV[id]->Signal(airport->securityQueuesLock);
			airport->securityQueuesLock->Release();
			airport->securityCV[id]->Wait(airport->securityLocks[id]);

			airport->securityLocks[id]->Acquire();
			bool passFlag;
			passFlag = p->GetSecurityPass();

			//do the security check here
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
				p->SetBoarding();

				printf(
						"Security inspector %d is not suspicious of the passenger %d\n",
						id, p->getID());
				printf("Security inspector %d allows passenger %d to board\n",
						id, p->getID());

				//update clear count here
				airport->securityManagerLock->Acquire();
				int airlineP = p->GetAirline();
				(clearCount[airlineP])++;
				airport->securityManagerLock->Release();

				//handing over boarding pass here
				BoardingPass bp = p->GetBoardingPass();
				boardingPassList->Append(&bp);

			} else { //if fail set securityPass to false

				printf(
						"Security inspector %d is suspicious of the passenger %d\n",
						id, p->getID());
				printf(
						"Security inspector %d asks passenger %d to go for further examination\n",
						id, p->getID());
			}

			airport->securityCV[id]->Signal(airport->securityLocks[id]);
			airport->securityCV[id]->Wait(airport->securityLocks[id]);  //end 2nd C.S.

		//finish when all passengers boarded
		} else if (airport->allFinished) {
			currentThread->Finish();
		//if both return and normal lines are empty, yield
		} else {
			airport->securityState[id] = SC_FREE;
			airport->securityQueuesLock->Release();
			currentThread->Yield();
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
