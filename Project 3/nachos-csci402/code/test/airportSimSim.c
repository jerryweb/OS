#include "syscall.h"
#include "simSimStructs.h"

#define NULL (void*)0

/*class data*/
int passengerCount;
Passenger *passengers[5];
Passenger p0;
Passenger p1;
Passenger p2;
Passenger p3;
Passenger p4;

int screenCount;
Screen *screens[3];
Screen sn0;
Screen sn1;
Screen sn2;

int screenLine[3][5];

/*synchonization data*/
int screenLineLock;
int screenLock[3];
int screenLineCV[3];
int screenCV[3];

/*airport helper functions*/
int findShortestLine(int list[][5],int num) {
	int shortest = -1;
	int i;

	for (i = 0; i < num; i++) {
		if (shortest == -1) {
			shortest = sizeOf(list[i]);
		} else {
			if (sizeOf(list[i]) < shortest) {
				shortest = sizeOf(list[i]);
			}
		}
	}

	return shortest;
}

/*array helper functions*/
int sizeOf(int* list) {
	int counter = 0;
	int toPrint;
	Printf("in size function line 49\n",25,0,0);
	while (1) {
		toPrint = list[counter];
		Printf("list[counter] = %d\n",19,1,toPrint);
		if (list[counter] == -1) {
			Printf("in size function line 53\n",25,0,0);
			break;
		} else {
			/*Printf("in size function line 56\n",25,0,0);*/
			counter++;
		}
	}
	Printf("in size function line 60\n",25,0,0);
	return counter;
}

int remove(int* list) {
	int i;
	int toRemove;
	int size = sizeOf(list);

	toRemove = list[0];
	for (i=0;i<size;i++) {
		if (i == size - 1) {
			list[i] = -1;
		} else {
			list[i] = list[i+1];
		}
	}

	return toRemove;
}

void append(int* list,int item) {
	int size = sizeOf(list);

	list[size] = item;
}

void init() {
	int i,j;

	/*initialize passenger*/
	passengerCount = 0;
	passengers[0] = &p0;
	passengers[1] = &p1;
	passengers[2] = &p2;
	passengers[3] = &p3;
	passengers[4] = &p4;

	for (i = 0; i < 5; i++) {
		passengers[i]->id = i;
		passengers[i]->myLine = 0;
	}

	/*initialize screen*/
	screenCount = 0;
	screens[0] = &sn0;
	screens[1] = &sn1;
	screens[2] = &sn2;

	for (i = 0; i < 3; i++) {
		screens[i]->id = i;
	}

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 5; j++) {
			screenLine[i][j] = -1;
		}
	}

	/*initialize synch variables*/
	screenLineLock = CreateLock("screenLineLock", 14);

	for (i = 0; i < 3; i++) {
		screenLock[i] = CreateLock("screenLock", 10);
		screenLineCV[i] = CreateCondition("screenLineCV", 12);
		screenCV[i] = CreateCondition("screenCV", 8);
	}
}

void forkPassenger() {
	int id,myLine;

	id = passengerCount;
	myLine = 0;

	Printf("passenger %d acquire line 131\n",30,1,id);
	Acquire(screenLineLock);
	Printf("passenger %d acquire line 132\n",30,1,id);
	myLine = findShortestLine(screenLine, 3);
	append(screenLine[myLine], id);
	Printf("passenger %d wait line 134\n",27,1,id);
	Wait(screenLineCV[myLine], screenLineLock);

	Acquire(screenLock[myLine]);
	Signal(screenCV[myLine], screenLock[myLine]);
	Printf(
			"Passenger %d gives the hand-luggage to screening officer %d\n",60 ,2,100*id+myLine);
	Wait(screenCV[myLine], screenLock[myLine]);

	Acquire(screenLock[myLine]);
	Signal(screenCV[myLine], screenLock[myLine]);

	Release(screenLock[myLine]);

	Printf("Passenger %d proceed to security\n",33 ,1,id);

	Exit(0);
}

void forkScreen() {
	int id;
	int p;    /*current passenger*/
	int luggageTest ;
	int size;

	id = screenCount;

	while (1) {
		Printf("screen line 163\n",16,0,0);
		Acquire(screenLineLock);
		Printf("screen line 164\n",16,0,0);

		size = sizeOf(screenLine[id]);
		Printf("screen line size is %d\n",23,1,size);

		if (sizeOf(screenLine[id]) != 0) {
			Printf("screen line 165\n",16,0,0);
			Acquire(screenLock[id]);
			Printf("screen line 166\n",16,0,0);
			p = remove(screenLine[id]);
			Printf("screen line 168\n",16,0,0);
			Signal(screenLineCV[id],screenLineLock);
			Release(screenLineLock);

			Printf("screen %d wait line 166\n",24,1,id);
			Wait(screenCV[id],screenLock[id]);
			Acquire(screenLock[id]);

			luggageTest = 0;
			if ((p % 3) < 1 ) {
				luggageTest = -1;
			} else {
				luggageTest = 1;
			}

			if (luggageTest > 0) {
				Printf("Screening officer %d is suspicious of the hand luggage of passenger %d\n",71 ,2,100*id+p);
			} else {
				Printf("Screening officer %d is not suspicious of the hand luggage of passenger %d\n",75 ,2,100*id+p);
			}

			/*find security line here*/

			Signal(screenCV[id],screenLock[id]);
			Wait(screenCV[id],screenLock[id]);
		} else {
			Release(screenLineLock);
			Yield();
		}
	}

	Exit(0);
}

int main() {
	int i,j;

	init();

	for (i =0;i<3;i++) {
		Fork(forkScreen,"Screen",6);
		Printf("screen %d forked\n",17,1,screenCount);
		screenCount++;
	}

	for (i=0;i<5;i++) {
		Fork(forkPassenger,"Passenger",9);
		Printf("passenger %d forked\n",20,1,passengerCount);
		passengerCount++;
	}
}
