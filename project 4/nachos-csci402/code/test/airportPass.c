/* Networked airport simulation.
 *  Initializes a single passenger.
 */

#include "syscall.h"
#include "airportStructs.h"

#define NULL (void*)0

/* General variables */
int numAirlines;
int airlineList; /* mv size 3, Airline* */
int airlineLockList; /* mv size 3, int (lock) */
int conveyor; /* mv size 63, Luggage* */
int conveyorLock;
int conveyorSize;
int aircraftList; /* mv size 3, Luggage** */
int aircraft1; /* mv size 21, Luggage* */
int aircraft2; /* mv size 21, Luggage* */
int aircraft3; /* mv size 21, Luggage* */
int aircraftCountList; /* mv size 3, int */
int boardingQueueList; /* mv size 3, int (MV) */
int boardingQueue1; /* mv size 21, Passenger* */
int boardingQueue2; /* mv size 21, Passenger* */
int boardingQueue3; /* mv size 21, Passenger* */
int boardingCVList; /* mv size 3, int (CV) */
int boardingLockList; /* mv size 3, int (lock) */
/* Passenger variables */
int passengerList; /* mv size 21, Passenger* */
int passengerListLock;
int passengerCount;
/*Used as a FIFO queue of the order of interactions between passengers and liaisons.
For example, if liaison 0 is the first to execute and wait for passenger 0, then 
passenger 0 is put in the FIFO in the first position. Liaison 1 would then put 
passenger 1 in the queue next behind passenger 0.
This prevents overwriting of Passenger variables. 2D array: first dimension
is the number of interactions between one passenger and one liaison. These 
are usually before the wait functions. For passenger, there are currently 2
separate occasions where one passenger and one liaison will communicate with
each other. */
int passengerLiaisonInteractionOrderList; /* mv size 5, int (MV) */
int passengerLiaisonInteractionOrder1; /* mv size 21, Passenger* */
int passengerLiaisonInteractionOrder2; /* mv size 21, Passenger* */
int passengerLiaisonInteractionOrder3; /* mv size 21, Passenger* */
int passengerLiaisonInteractionOrder4; /* mv size 21, Passenger* */
int passengerLiaisonInteractionOrder5; /* mv size 21, Passenger* */
int passengerCheckinInteractionOrderList; /* mv size 21, Passenger* */
/* Liaison variables */
int liaisonCount;
int liaisonList; /* mv size 5, Liaison* */
int liaisonListLock;
int liaisonPassengerInteractionOrder; /* mv size 5, Liaison* */
int liaisonManagerInteractionOrder; /* mv size 5, Liaison* */
int liaisonWaitOrder; /* mv size 5, Liaison* */
int liaisonLineList; /* mv size 5, int (MV) */
int liaisonLine1; /* mv size 21, Passenger* */
int liaisonLine2; /* mv size 21, Passenger* */
int liaisonLine3; /* mv size 21, Passenger* */
int liaisonLine4; /* mv size 21, Passenger* */
int liaisonLine5; /* mv size 21, Passenger* */
int liaisonManagerLock;
int liaisonManagerCV;
int liaisonLineLock;
int liaisonLineCVList; /* mv size 5, int (CV) */
int liaisonLockList; /* mv size 5, int (lock) */
int liaisonCVList; /* mv size 5, int (CV) */
int liaisonStateList; /* mv size 5, LiaisonState */
int requestingLiaisonDataList; /* mv size 5, bool */
/* Check-in variables */
int checkinCount;
int checkinList; /* mv size 12, Checkin* */
int checkinListLock;
int checkinBreakOrder; /* mv size 12, Checkin* */
int checkinManagerInteractionOrder; /* mv size 12, Checkin* */
int checkinLineList; /* mv size 12, int (MV) */
int liaisonLine1; /* mv size 21, Passenger* */
int liaisonLine2; /* mv size 21, Passenger* */
int liaisonLine3; /* mv size 21, Passenger* */
int liaisonLine4; /* mv size 21, Passenger* */
int liaisonLine5; /* mv size 21, Passenger* */
int liaisonLine6; /* mv size 21, Passenger* */
int liaisonLine7; /* mv size 21, Passenger* */
int liaisonLine8; /* mv size 21, Passenger* */
int liaisonLine9; /* mv size 21, Passenger* */
int liaisonLine10; /* mv size 21, Passenger* */
int liaisonLine11; /* mv size 21, Passenger* */
int liaisonLine12; /* mv size 21, Passenger* */
int checkinLineLockList, /* mv size 3, int (lock) */
int checkinLineCVList; /* mv size 12, int (CV) */
int checkinCVList; /* mv size 12, int (CV) */
int checkinLockList; /* mv size 12, int (lock) */
int checkinBreakCVList; /* mv size 12, int (CV) */
int checkinManagerLock;
int checkinManagerCV;
int checkinStateList; /* mv size 12, CheckinState */
int finalCheckinList; /* mv size 12, bool */
int requestingCheckinDataList; /* mv size 12, bool */
/* Cargo variables */
int cargoCount;
int cargoListLock;
int cargoList; /* mv size 6, Cargo* */
int cargoManagerInteractionOrderList; /* mv size 2, int (MV) */
int cargoManagerInteractionOrder1; /* mv size 6, Cargo* */
int cargoManagerInteractionOrder2; /* mv size 6, Cargo* */
int cargoCVList; /* mv size 6, int (CV) */
int cargoDataCVList; /* mv size 6, int (CV) */
int cargoDataLockList; /* mv size 6, int (lock) */
int cargoManagerLock;
int cargoManagerCVList; /* mv size 6, int (CV) */
int cargoLockList; /* mv size 6, int (lock) */
int cargoStateList; /* mv size 6, CargoState */
int requestingCargoDataList; /* mv size 6, bool */
/* Manager variables */
Manager manager;
bool cargoHandlersOnBreak;
bool liaisonDone;
bool CargoDone;
bool CheckinDone;
bool ready;
int clearAirline; /* mv size 3, bool */
int counter;
int clearAirlineCount;

void Init()
{
    int i, j;
    /* General variables */
    numAirlines = 3;
    conveyorLock = CreateLock("ConveyorLock", 12);
    conveyorSize = 0;
    for (i = 0; i < 3; i++)
    {
        Airline al;
        al.id = i;
        al.seatsAssigned = 0;
        al.ticketsIssued = 7;
        al.totalBagCount = al.ticketsIssued * 3;
        al.totalBagWeight = al.totalBagCount * 30;
        airlines[i] = &al;
        airlineLock[i] = CreateLock("AirlineLock", 11);
        boardingCV[i] = CreateCondition("BoardingCV", 10);
        boardingLock[i] = CreateLock("BoardingLock", 12);
        aircraftCount[i] = 0;
        for (j = 0; j < 21; j++)
        {
            boardingQueue[i][j] = NULL;
            /*conveyor[(i+1)*(j+1)-1] = NULL;*/
            aircraft[i][j] = NULL;
        }
    }
    for(i = 0; i < 63; i++)
    	conveyor[i] = NULL;
    /* Passenger variables */
    passengerCount = 0;
    passengerArrayLock = CreateLock("PassengerArrayLock", 18);

	passengerArray[0] = &p0;
	passengerArray[1] = &p1;
	passengerArray[2] = &p2;
	passengerArray[3] = &p3;
	passengerArray[4] = &p4;
	passengerArray[5] = &p5;
	passengerArray[6] = &p6;
	passengerArray[7] = &p7;
	passengerArray[8] = &p8;
	passengerArray[9] = &p9;
	passengerArray[10] = &p10;
	passengerArray[11] = &p11;
	passengerArray[12] = &p12;
	passengerArray[13] = &p13;
	passengerArray[14] = &p14;
	passengerArray[15] = &p15;
	passengerArray[16] = &p16;
	passengerArray[17] = &p17;
	passengerArray[18] = &p18;
	passengerArray[19] = &p19;
	passengerArray[20] = &p20;
	passengerArray[21] = &p21;

	/*for(i = 5; i<21; i++)
		passengerArray[i] = NULL;*/

    for(i = 0; i < 6; i++){
    	for (j = 0; j < 21; j++)
    		passengerLiaisonInteractionOrder[i][j] = NULL;
    }
      /*  	passengerCheckinInteractionOrder[i] = NULL;*/


    /* Liaison variables */
    liaisonCount = 0;
    liaisonArrayLock = CreateLock("LiaisonArrayLock", 16);
    liaisonManagerLock = CreateLock("LiaisonManagerLock", 18);
    liaisonManagerCV = CreateCondition("LiaisonManagerCV", 16);
    liaisonLineLock = CreateLock("LiaisonLineLock", 15);
    /*must be statically declared*/
    liaisonArray[0] = &l0;
    liaisonArray[1] = &l1;
    liaisonArray[2] = &l2;
    liaisonArray[3] = &l3;
    liaisonArray[4] = &l4;
    for (i = 0; i < 5; i++)
    {
        /*liaisonArray[i] = NULL;*/
        liaisonPassengerInteractionOrder[i] = NULL;
        liaisonManagerInteractionOrder[i] = NULL;
        liaisonWaitOrder[i] = NULL;
        liaisonLineCV[i] = CreateCondition("LiaisonLineCV", 13);
        liaisonLock[i] = CreateLock("LiaisonLock", 11);
        liaisonCV[i] = CreateCondition("LiaisonCV", 9);
        liaisonState[i] = L_BUSY;
        requestingLiaisonData[i] = false;
        for (j = 0; j < 21; j++)
        {
            liaisonLine[i][j] = NULL;
    		passengerLiaisonInteractionOrder[i][j] = NULL;
        }
    }
    /* Check-in variables */
    checkinCount = 0;
    checkinArrayLock = CreateLock("CheckinArrayLock", 16);
    checkinManagerLock = CreateLock("CheckinManagerLock", 18);
    checkinManagerCV = CreateCondition("CheckinManagerCV", 16);
    /*must be statically declared*/
    checkinArray[1] = &ci1;
    checkinArray[2] = &ci2;
    checkinArray[3] = &ci3;
    checkinArray[5] = &ci5;
    checkinArray[6] = &ci6;
    checkinArray[7] = &ci7;
    checkinArray[9] = &ci9;
    checkinArray[10] = &ci10;
    checkinArray[11] = &ci11;
    for (i = 0; i < 12; i++)
    {
        if (i%4 == 0) /* 0, 4, 8 */
        {
            checkinLineLock[i/4] = CreateLock("CheckinLineLock", 15);
            checkinState[i] = CI_NONE;
        }
        else checkinState[i] = CI_BUSY;
        checkinBreakOrder[i] = NULL;
        checkinManagerInteractionOrder[i] = NULL;
        checkinLineCV[i] = CreateCondition("CheckinLineCV", 13);
        checkinCV[i] = CreateCondition("CheckinCV", 9);
        checkinLock[i] = CreateLock("CheckinLock", 11);
        checkinBreakCV[i] = CreateCondition("CheckinBreakCV", 14);
        finalCheckin[i] = false;
        requestingCheckinData[i] = false;;
        for (j = 0; j < 21; j++)
        {
    		if (i == 0) passengerCheckinInteractionOrder[j] = NULL;
            checkinLine[i][j] = NULL;
        }
    }
    /* Cargo variables */
    cargoCount = 0;
    cargoArrayLock = CreateLock("CargoArrayLock", 14);
    cargoManagerLock = CreateLock("CargoManagerLock", 16);
    /*must be statically declared*/
    cargoArray[0] = &c0;
    cargoArray[1] = &c1;
    cargoArray[2] = &c2;
    cargoArray[3] = &c3;
    cargoArray[4] = &c4;
    cargoArray[5] = &c5;
    for (i = 0; i < 6; i++)
    {
    	for(j = 0; j <2; j++)
    		cargoManagerInteractionOrder[j][i] = NULL;

        cargoDataCV[i] = CreateCondition("CargoDataCV", 11);
        cargoDataLock[i] = CreateLock("CargoDataLock", 13);
        cargoManagerCV[i] = CreateCondition("CargoManagerCV", 14);
        cargoCV[i] = CreateCondition("CargoCV", 7);
        cargoLock[i] = CreateLock("CargoLock", 9);
        cargoState[i] = C_BUSY;
        requestingCargoData[i] = false;
    }

    /* Manager variables */
    for(i = 0; i < 3; i++)
    	clearAirline[i] = false;
    
}

/*Used to find the number of elements in an array*/
int findArrayElementCount(Passenger*** array, int pLine){
	int elementCount = 0;
	int i;
	for(i = 0; i < 21; i++){
		if(array[pLine][elementCount] == NULL)
			break;
		else
			elementCount++;
	}

	return elementCount;
}

int findShortestLine(bool CISline, int pCount){
	Passenger p;
    int i, j, k, elementCount;
    i = 0;
    p.location = 0;
	p.minValue = 21;
    
	if(!CISline){
		for (i = 0; i < 5; i++) {

            elementCount = 0;
            while(liaisonLine[i][elementCount] != NULL) elementCount++;

			if(elementCount < p.minValue){
				p.minValue = elementCount;
				p.location = i;
			}
		}
		return p.location;
	}

	/*finds the shortest checkin staff line*/
	else if(CISline){
		int CIS_ID = p.airline * 4 + 1;
		for(i = CIS_ID;  i < CIS_ID + 3; i++){
			elementCount = 0;
			while(checkinLine[i][elementCount] != NULL) elementCount++;
			if((p.minValue < 0 || p.minValue > elementCount) &&
				(checkinState[i] != CI_CLOSED || checkinState[i] != CI_NONE)){
					p.minValue = elementCount;
					p.location = i;
			}
		}
		return p.location;		
	}
}

void PassengerFindShortestCISLine(Passenger *p){
	int i, j, elementCount;
	int tempID = p->id;
	passengerArray[tempID]->myLine = passengerArray[tempID]->airline * 4;
	Acquire(checkinLineLock[passengerArray[tempID]->ticket->airline]);

	if(!passengerArray[tempID]->ticket->executive){
		passengerArray[tempID]->myLine = findShortestLine(true,passengerArray[tempID]->id);
        elementCount = 0;
        while(checkinLine[passengerArray[tempID]->myLine][elementCount] != NULL) elementCount++;
		Printf("Passenger %d of Airline %d chose Airline Check-In staff %d with line length %d\n",
			79, 4, passengerArray[tempID]->id*100*100*100 + passengerArray[tempID]->airline*100*100 
			+ passengerArray[tempID]->myLine*100 + elementCount);
	}
	else
		Printf("Passenger %d of Airline %d is waiting in the executive line\n",
		 60, 2, passengerArray[tempID]->id*100 + passengerArray[tempID]->airline);
	
    elementCount = 0;
    while(checkinLine[passengerArray[tempID]->myLine][elementCount] != NULL) elementCount++;
    
	checkinLine[passengerArray[tempID]->myLine][elementCount] = passengerArray[passengerArray[tempID]->id];

	Wait(checkinLineCV[passengerArray[passengerArray[tempID]->id]->myLine], checkinLineLock[passengerArray[passengerArray[tempID]->id]->airline]);
    
	Acquire(passengerArrayLock);
    p = passengerCheckinInteractionOrder[0];
    for (i = 1; i < 21; i++)
    {
        passengerCheckinInteractionOrder[i-1] = passengerCheckinInteractionOrder[i];
    }
    passengerCheckinInteractionOrder[20] = NULL;
    Release(passengerArrayLock);

    /*go to next step*/
    Exit(0);
}

void PassengerFindShortestLiaisonLine(Passenger *p){

    int i, elementCount;
	Acquire(liaisonLineLock);
	passengerArray[p->id]->myLine = findShortestLine(false, p->id);
	elementCount = 0;

	for (i = 0; i < 21; i++){
		if(liaisonLine[passengerArray[p->id]->myLine][i] != NULL)
			elementCount++;	
		else
			break;		
	}

	Printf("Passenger %d chose liaison %d with a line length of %d\n",
	 55, 3, passengerArray[p->id]->id*100*100 + passengerArray[p->id]->myLine*100 + elementCount);

	liaisonLine[p->myLine][elementCount] = passengerArray[p->id];

	if(liaisonState[passengerArray[p->id]->myLine] == L_BUSY){
		/*Wait for an available liaison*/

		Wait(liaisonLineCV[passengerArray[p->id]->myLine], liaisonLineLock);

	}
	else
		Release(liaisonLineLock);

	Acquire(passengerArrayLock);
    p = passengerLiaisonInteractionOrder[0][0];
    for (i = 1; i < 21; i++)
    {
        passengerLiaisonInteractionOrder[0][i-1] = passengerLiaisonInteractionOrder[0][i];
    }
    passengerLiaisonInteractionOrder[0][20] = NULL;
    Release(passengerArrayLock);
    
    /*Printf("p id is now: %d\n", 16,1,p->id);*/
    
	Acquire(liaisonLock[passengerArray[p->id]->myLine]);
		/*wait for liaison confirmation*/
	
    Acquire(liaisonArrayLock);		/*prevent alteration while calculating the # of elements in
    								  the array*/

    elementCount = 0;
    for (i = 0; i < 5; i++){
		if(liaisonPassengerInteractionOrder[i] != NULL){
			elementCount++;	
			/*Printf("daf %d\n", 7,1,passengerArray[p->id]->id);*/			
		}
	}
	/*add passenger to the end of the array*/
    liaisonPassengerInteractionOrder[elementCount] = liaisonArray[p->myLine];
	/*Give liaison information
	*/
	Signal(liaisonCV[passengerArray[p->id]->myLine], liaisonLock[passengerArray[p->id]->myLine]);

    Release(liaisonArrayLock);
	
	Wait(liaisonCV[passengerArray[p->id]->myLine], liaisonLock[passengerArray[p->id]->myLine]);

	Acquire(passengerArrayLock);
    p = passengerLiaisonInteractionOrder[1][0];
    for (i = 1; i < 21; i++)
    {
        passengerLiaisonInteractionOrder[1][i-1] = passengerLiaisonInteractionOrder[1][i];
    }
    passengerLiaisonInteractionOrder[1][20] = NULL;
	Release(passengerArrayLock);

	Printf("Passenger %d of Airline %d is directed to the airline counter.\n",
		63, 2, passengerArray[p->id]->id*100 + passengerArray[p->id]->airline);

	/*move to the designated checkin counter*/
	PassengerFindShortestCISLine(passengerArray[p->id]);	
}

void forkPassenger(){
	int i, elementCount, pCount;
	Acquire(passengerArrayLock);
	pCount = passengerCount;

	passengerCount++;
	passengerArray[pCount]->id = pCount;
	passengerArray[pCount]->ticket->airline = pCount % 3;
	passengerArray[pCount]->ticket->executive = false;
	passengerArray[pCount]->myLine = 0;
	passengerArray[pCount]->CISline = false;
	
	for(i = 0; i < 3; i++){
		passengerArray[pCount]->bags[i]->airlineCode = 0;
		passengerArray[pCount]->bags[i]->weight = 30 + i;
	}

	passengerArray[pCount]->boardingPass->gate = 0;
	passengerArray[pCount]->boardingPass->seatNum = 0;
	Release(passengerArrayLock);
	PassengerFindShortestLiaisonLine(passengerArray[pCount]);
}

int main()
{
    int i;
    Printf("Initializing Passengers\n", 24, 0, 0);
    CreateVariables();
    for (i = 0; i < 21; i++)
    {
    	Fork(forkPassenger, "Passenger", 9);
	}
}
