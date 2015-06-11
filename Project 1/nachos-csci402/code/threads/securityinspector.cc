#include "securityinspector.h"
#include "stdlib.h"
#include "time.h"
#include "passenger.h"

SecurityInspector::SecurityInspector(int ID, Airport* AIRPORT) {
	id = ID;
	airport = AIRPORT;
	hasReturned = false;
	returnPassenger = NULL;
	//PassengerCount = new int[airport->numAirlines];
}

SecurityInspector::~SecurityInspector() {

}

void SecurityInspector::Inspect() {
	//printf("in SecurityInspector Inspect() line 15 \n");
	//initialize random seed
	srand(time(NULL));
	while (true) {
		//printf("in SecurityInspector Inspect() line 10 \n");
		//starting C.S.(1) for security line
		airport->securityLocks[id]->Acquire();
		Passenger* currentPassenger;

		//check if someone has just returned
		if (hasReturned) {
			//printf("inspector line 29/n");
			airport->returnQueues[id]->Append(returnPassenger);
			hasReturned = false;
			returnPassenger = NULL;
			airport->securityLocks[id]->Release();
		}
		//handle return line first
		else if ((airport->returnQueues[id]->Size()) != 0) {
			currentPassenger = (Passenger*) airport->returnQueues[id]->First();
			currentPassenger->SetSecurityPass(true);

			airport->returnQueues[id]->Remove();

			//starting C.S.(2) for ping-pong with returning passenger
			airport->rePassengerWaitInspectorCV[id]->Signal(airport->securityLocks[id]);
			airport->securityLocks[id]->Release();
			//printf("insepctor line 40 \n");
			airport->securityLocks[id]->Acquire();
			//printf("insepctor line 42 \n");
			airport->inspectorWaitRePassengerCV[id]->Wait(
					airport->securityLocks[id]);
			//printf("insepctor line 45 \n");
			//ending C.S.(2)

			printf(
					"Security inspector %d permits returning passenger %d to board\n",
					id, currentPassenger->getID());

			//if return line is empty handle the security line
		} else if ((airport->securityQueues[id]->Size()) != 0) {
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
			airport->passengerWaitInspectorCV[id]->Signal(airport->securityLocks[id]);
			airport->securityLocks[id]->Release();
			airport->securityLocks[id]->Acquire();
			airport->inspectorWaitPassengerCV[id]->Wait(airport->securityLocks[id]);
			//make decision based on the result above
			if (!passFlag) {
				//airport->returnQueues[id]->Append(currentPassenger);
				//currentPassenger->SetReturn();

				printf(
						"Security inspector %d is suspicious of the passenger %d\n",
						id, currentPassenger->getID());
				printf(
						"Security inpector %d asks passenger %d to go for further examination\n",
						id, currentPassenger->getID());
			} else {
				printf(
						"Security inspector %d is not suspicious of the passenger %d\n",
						id, currentPassenger->getID());
				printf("Security inspector %d allows passenger %d to board\n",
						id, currentPassenger->getID());
			}

		} else {
			continue;
		}
	}
}

void SecurityInspector::setReturn() {
	hasReturned = true;
}

void SecurityInspector::setReturnPassenger(Passenger* p) {
	returnPassenger = p;
}

int SecurityInspector::getID() {
	return id;
}
