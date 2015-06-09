#include "screenofficer.h"
#include "stdlib.h"
#include "time.h"

Screenofficer::ScreenOfficer(int id)
{
	this.id = id;
}

Screenofficer::~ScreenOfficer() {

}

void Screenofficer::Screen(int* liaisonList[7]) {
	//keep checking his own line
	while (true) {
		//the index in C.S. parenthesis is for this file only

		//start C.S.(1) between passenger
		//assuming officer ID is the same as his index in screenLocks array
        int myLine = this.id;
        screenLocks[myLine].Acquire();

		if (screenLine.size != 0) {
			Passenger* currentPassenger = (Passenger*) screenLine.First();

			//assume 10% chance fail the screening
			srand(time(NULL));
			int randNum = rand() % 100 + 1;
			if (randNum > 0 && randNum < 11) {
				currentPassenger->SetSecurityPass(false);
				printf(
						"Screening officer %d is suspicious of the hand luggage of passenger %d\n",
						this.id, currentPassenger->GetID());
			} else {
				printf(
						"Screening officer %d is not suspicious of the hand luggage of passenger %d\n",
						this.id, currentPassenger->GetID());
			}

			//remove the passenger in current line

			//starting C.S.(2) to remove passenger from current line
			screenQueuesLock.Acquire();
			screenQueues[myLine].Remove();
			screenQueuesLock.Release();
			//ending C.S.(2) to remove passenger from current line
		}

		//starting C.S.(3) to find shortest security line and append
		securityQueuesLock.Acquire();

		int shortest = 0; //shortest line's id
		int minimumSize = -1;  //for comparsion in the following loop

		//find the shortest line
		for (int i = 0; i < 3; i++) {
			if (minimumSize < 0 || minimumSize > screenQueues[i].Size()) {
				minimumSize = screenQueues[i].Size();
				shrotest = i;
			}
		}

		securityQueues[shortest].Append(currentPassenger);
		//also update the queueIndex for current passenger
		currentPassenger->SetQueueIndex(shortest);

		securityQueuesLock.Release();
		//ending C.S.(3)

		//signal the passenger
		screenCV[myLine].Signal(screenLocks[myLine]);
		screenLocks[myLine].Release();

		//wait for passenger thread
		screenCV[myLine].Wait(screenLocks[myLine]);
		screenLocks[myLine].Release();
		//ending C.S.(1)

		//Assuming the index in securityQueues array is the same as security inspector's id
		printf(
				"Screening officer %d directs passenger %d to security inspector %d\n",
				this.id, currentPassenger->GetID(), shortest);

	}
}
