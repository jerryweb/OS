/* Airport simulation.
 *  3 cash-starved airlines
 *  21 clueless passengers: 3 bags each, all bags weigh 30 lbs
 *  5 hardworking liaisons
 *  3 lazy check-in staff per airline
 *  6 super lazy cargo handlers
 *  3 screening officers and security inspectors, always suspicious of everything
 *  and of course, 1 evil manager
 */

#include "syscall.h"
#include "airportStructs.h"

#define NULL (void*)0

/* General variables */
 Passenger *Passengers[21];
int numAirlines;
Airline* airlines[3];
int airlineLock[3];
Luggage* conveyor[63];
int conveyorLock;
int conveyorSize;
Luggage* aircraft[3][21];
int aircraftCount[3];
Passenger* boardingQueue[3][21];
int boardingCV[3];
int boardingLock[3];
/* Passenger variables */
    Passenger p0;
	Passenger p1;
	Passenger p2;
	Passenger p3;
	Passenger p4;
	Passenger p5;
	Passenger p6;
	Passenger p7;
	Passenger p8;
	Passenger p9;
    Passenger p10;
	Passenger p11;
	Passenger p12;
	Passenger p13;
	Passenger p14;
	Passenger p15;
	Passenger p16;
	Passenger p17;
	Passenger p18;
	Passenger p19;
	Passenger p20;
	Passenger p21;	
int passengerCount;
int passengerArrayLock;
	/*Used as a FIFO queue of the order of interactions between passengers and liaisons.
	For example, if liaison 0 is the first to execute and wait for passenger 0, then 
	passenger 0 is put in the FIFO in the first position. Liaison 1 would then put 
	passenger 1 in the queue next behind passenger 0.
	This prevents overwriting of Passenger variables. 2D array: first demension
	is the number of interactions between one passenger and one liaison. These 
	are usually before the wait functions. For passenger, there are currently 2
	seperate occasions where one passenger and one liaison will communicate with
	each other. */
	Passenger*  passengerLiaisonInteractionOrder[5][21];
    Passenger*  passengerCheckinInteractionOrder[21];

Passenger* passengerArray[21];
/* Liaison variables */
	Liaison l0;
	Liaison l1;	
	Liaison l2;
	Liaison l3;
	Liaison l4;
int liaisonCount;
int liaisonArrayLock;
	Liaison*  liaisonPassengerInteractionOrder[5];
	Liaison*  liaisonManagerInteractionOrder[5];
    Liaison*  liaisonWaitOrder[5];
Liaison* liaisonArray[5];
Passenger* liaisonLine[5][21];
int liaisonManagerLock;
int liaisonManagerCV;
int liaisonLineLock;
int liaisonLineCV[5];
int liaisonLock[5];
int liaisonCV[5];
LiaisonState liaisonState[5];
bool requestingLiaisonData[5];
/* Check-in variables */
	Checkin ci1;
	Checkin ci2;	
	Checkin ci3;
	Checkin ci5;
	Checkin ci6;
	Checkin ci7;
	Checkin ci9;	
	Checkin ci10;
	Checkin ci11;
int checkinCount;
int checkinArrayLock;
	Checkin*  checkinBreakOrder[12];
	Checkin*  checkinManagerInteractionOrder[12];
Checkin* checkinArray[12];
Passenger* checkinLine[12][21];
int checkinLineLock[3];
int checkinLineCV[12];
int checkinCV[12];
int checkinLock[12];
int checkinBreakCV[12];
int checkinManagerLock;
int checkinManagerCV;
CheckinState checkinState[12];
bool finalCheckin[12];
bool requestingCheckinData[12];
/* Cargo variables */
	Cargo c0;
	Cargo c1;	
	Cargo c2;
	Cargo c3;
	Cargo c4;
	Cargo c5;
int cargoCount;
int cargoArrayLock;
Cargo* cargoArray[6];
int cargoCV;
int cargoDataCV[6];
int cargoDataLock[6];
int cargoManagerLock;
int cargoManagerCV[6];
int cargoLock[6];
CargoState cargoState[6];
bool requestingCargoData[6];

/* Manager variables */
Manager manager;
	bool cargoHandlersOnBreak;
	bool liaisonDone;
	bool CargoDone;
	bool CheckinDone;
	bool ready;
	bool clearAirline[3];
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

	for(i = 5; i<21; i++)
		passengerArray[i] = NULL;

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
    cargoCV = CreateCondition("CargoCV", 7);
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
        cargoDataCV[i] = CreateCondition("CargoDataCV", 11);
        cargoDataLock[i] = CreateLock("CargoDataLock", 13);
        cargoManagerCV[i] = CreateCondition("CargoManagerCV", 14);
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
			Printf("adf\n", 4, 0,0);
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
    /*for(i = 0; i < 5;i++)
    	p.lineLength[i] = 0;

	for(j = 0; j < 21; j++){
		if(liaisonLine[0][j] == NULL)
			break;
		else{
			p.lineLength[0]++;
		}
	}
*/
	p.minValue = 21;
	/*Printf("lineLength: %d\n", 15, 1, p.lineLength[0]);
	Printf("min value: %d\n", 14, 1, p.minValue);
	/*finds the shortest liaison line*/
	if(!CISline){
		for (i = 0; i < 5; i++) {

			for(j = 0; j < 21; j++){
				if(liaisonLine[i][j] == NULL)
					break;
				else
					p.lineLength[i]++;
			}

			if(p.lineLength[i] < p.minValue){
				p.minValue = p.lineLength[i];
				p.location = i;
			}

	/*		Printf("lineLength: %d\n", 15, 1, p.lineLength[i]);
			Printf("min value: %d\n", 14, 1, p.minValue);
			Printf("i: %d\n", 6, 1, i);
			Printf("Location: %d\n", 13,1, p.location);*/
		}
		Printf("Liaison %d has the shortest line with a length of %d\n",
		 53, 2, p.location*100 + p.minValue);
		return p.location;
	}

	/*finds the shortest checkin staff line*/
	else if(CISline){
		int CIS_ID = p.airline * 4 + 1;
		/*
        Printf("CIS_ID %d\n", 10, 1, CIS_ID);
		*/
		for(i = CIS_ID;  i < CIS_ID + 3; i++){
			elementCount = 0;
			while(true){
				if(checkinLine[i][elementCount] == NULL)
					break;
				else
					elementCount++;
			}
			if((p.minValue < 0 || p.minValue > elementCount) &&
				(checkinState[i] != CI_CLOSED || checkinState[i] != CI_NONE)){

				for(j = 0; j < 21; j++){
					if(checkinLine[i][j] == NULL)
						break;
					else
						p.lineLength[i]++;
				}
				if(p.lineLength[i] < p.minValue){
					p.minValue = p.lineLength[i];
					p.location = i;
				}
			}
		}
		Printf("Checkin Staff %d has the shortest line with a length of %d\n",
		 59, 2, p.location*100 + p.minValue);
		return p.location;		
	}
	Exit(0);
}

void PassengerFindShortestCISLine(Passenger *p){
	int i, j, elementCount;
	int tempID = p->id;
	passengerArray[tempID]->myLine = passengerArray[tempID]->airline * 4;
	Acquire(checkinLineLock[passengerArray[tempID]->ticket->airline]);

	if(!passengerArray[tempID]->ticket->executive){
		passengerArray[tempID]->myLine = findShortestLine(true,passengerArray[tempID]->id);
		Printf("Passenger %d of Airline %d chose Airline Check-In staff %d with line length %d\n",
			79, 4, passengerArray[tempID]->id*100*100*100 + passengerArray[tempID]->airline*100*100 
			+ passengerArray[tempID]->myLine*100 + passengerArray[tempID]->minValue);
	}
	else
		Printf("Passenger %d of Airline %d is waiting in the executive line\n",
		 60, 2, passengerArray[tempID]->id*100 + passengerArray[tempID]->airline);
	
	elementCount = 0;
	for (j = 1; j < 21; j++){
		if(checkinLine[passengerArray[tempID]->myLine][j] != NULL)
			elementCount++;			
	}
	/*Printf("elementCount: %d\n", 17, 1, elementCount);	*/
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
	Printf("Passenger %d of airline %d created\n",
	 35, 2, passengerArray[pCount]->id*100
	  + passengerArray[pCount]->ticket->airline);
	Printf("Forking passenger\n", 18, 0,0);
	Release(passengerArrayLock);
	PassengerFindShortestLiaisonLine(passengerArray[pCount]);
}

void RunLiaison()
{
    int i, o, q, elementCount, lCount, tempPassengerID, order;
    int var[10];

    Passenger*  P;
    
    Acquire(liaisonArrayLock);
    lCount = liaisonCount;
    liaisonArray[lCount]->id = liaisonCount;
    liaisonCount++;
    for (i = 0; i < 3; i++)
    {
        liaisonArray[lCount]->passengers[i] = 0;
        liaisonArray[lCount]->luggage[i] = 0;
        liaisonArray[lCount]->weight[i] = 0;
    }    
    for(i = 0; i <10; i++)
    	var[i] = 0;

    order =0;
    Release(liaisonArrayLock);
        
    while(true)
    {
        Acquire(liaisonLineLock);
        tempPassengerID = 0;
        P = liaisonLine[lCount][0];
        Printf("l p->id = %d\n", 13, 1, P->id);
        if (P != NULL){
        	tempPassengerID = P->id;

        	liaisonArray[lCount]->myPassengerID = P->id;


            for (i = 1; i < 21; i++)
            {
                liaisonLine[lCount][i-1] = liaisonLine[lCount][i];
            }
		
            
            liaisonLine[lCount][20] = NULL;
			Signal(liaisonLineCV[lCount], liaisonLineLock);
            liaisonState[lCount] = L_BUSY;

            passengerArray[tempPassengerID]->airline = passengerArray[tempPassengerID]->ticket->airline;
            Acquire(passengerArrayLock);
            var[1] = 0;
            for (i = 0; i < 21; i++){
				if(passengerLiaisonInteractionOrder[0][i] != NULL){
					var[1]++;				
				}
			}
			/*add passenger to the end of the array*/
            passengerLiaisonInteractionOrder[0][var[1]] = passengerArray[tempPassengerID];
            Release(passengerArrayLock);
            Printf("Airport Liaison %d directed passenger %d of airline %d\n", 55, 3,
                    liaisonArray[lCount]->id*100*100 + liaisonArray[lCount]->myPassengerID*100 + passengerArray[tempPassengerID]->airline);
        }
        else
        {
            liaisonState[lCount] = L_FREE;
        }
        Acquire(liaisonLock[lCount]);
        Release(liaisonLineLock);
        if (P != NULL)
        {
        	Printf("Liaison %d is waiting for Passenger %d\n", 39, 2, lCount*100 + liaisonArray[lCount]->myPassengerID);
            Wait(liaisonCV[lCount], liaisonLock[lCount]);
            Acquire(liaisonArrayLock);

            lCount = liaisonPassengerInteractionOrder[0]->id;
            Printf("liaison %d has recieved a response from passenger %d\n",53,2,lCount*100 + liaisonArray[lCount]->myPassengerID);

		    for (i = 1; i < 5; i++)
		    {
		        liaisonPassengerInteractionOrder[i-1] = liaisonPassengerInteractionOrder[i];
		    }
		    liaisonPassengerInteractionOrder[4] = NULL;
			Release(liaisonArrayLock);

            Acquire(liaisonLock[lCount]);
            order = liaisonArray[lCount]->myPassengerID;
            liaisonArray[lCount]->passengers[passengerArray[order]->airline]++;
            Printf("Help\n", 5,0,0);
            for (q = 0; q < 3; q++)
            {
                liaisonArray[lCount]->luggage[passengerArray[order]->airline]++;
                liaisonArray[lCount]->weight[passengerArray[order]->airline] += passengerArray[order]->bags[q]->weight;
            }
                       

            Acquire(passengerArrayLock);
            var[2] = 0;
            for (i = 0; i < 21; i++){
				if(passengerLiaisonInteractionOrder[1][i] != NULL){
					var[2]++;				
				}
				else break;
			}  

			/*add passenger to the end of the array*/
            passengerLiaisonInteractionOrder[1][var[2]] = passengerArray[order];

            Signal(liaisonCV[lCount], liaisonLock[lCount]);

			Release(passengerArrayLock);

            P = NULL;
            Release(liaisonLock[lCount]);

        }
        else
        {
            Wait(liaisonCV[lCount], liaisonLock[lCount]);
            Acquire(liaisonArrayLock);
            lCount = liaisonWaitOrder[0]->id;
		    for (i = 1; i < 5; i++)
		    {
		        liaisonWaitOrder[i-1] = liaisonWaitOrder[i];
		    }
		    liaisonWaitOrder[4] = NULL;
			Release(liaisonArrayLock);
        }
        if (requestingLiaisonData[lCount])
        {
            Acquire(liaisonManagerLock);
            Acquire(liaisonLock[lCount]);
            Signal(liaisonManagerCV, liaisonManagerLock);
            Release(liaisonManagerLock);
            Wait(liaisonCV[lCount], liaisonLock[lCount]);
             Printf("Wait for manager to signal that all the data has been collected\n", 64, 0, 0);
            /* Wait for manager to signal that all the data has been collected*/

            lCount = liaisonManagerInteractionOrder[0]->id;
		    for (o = 1; o < 5; o++)
		        liaisonManagerInteractionOrder[o-1] = liaisonManagerInteractionOrder[o];

		    liaisonManagerInteractionOrder[4] = NULL;

		     Printf("lCount = %d\n", 12, 1, lCount);

            /* manager interaction queue */
            requestingLiaisonData[lCount] = false;
        }
    }
    Exit(0);
}

void RunCheckin()
{
    int i, j, len;
    int elementCount, ciCount, tempPassengerID;
    int execLine;
    Passenger* p;
    BoardingPass bp;
    bool exec;
    bool talked = false;
    
    Acquire(checkinArrayLock);
    while (checkinCount%4 == 0) /* 0, 4, 8 */
    {
        checkinCount++;
    }
    ciCount = checkinCount;
    checkinArray[ciCount]->id = checkinCount;
    checkinArray[ciCount]->airline = checkinCount/4;
    checkinCount++;
    checkinArray[ciCount]->passengers = 0;
    checkinArray[ciCount]->luggage = 0;
    checkinArray[ciCount]->weight = 0;
    Release(checkinArrayLock);
    
    while (true)
    {
        Acquire(checkinLineLock[checkinArray[ciCount]->airline]);
        tempPassengerID = 0;
        execLine = checkinArray[ciCount]->airline * 4;
        
        if (checkinState[checkinArray[ciCount]->id] != CI_CLOSED)
        {
            len = 0;
            if (checkinLine[execLine][0] != NULL)
            {
                p = checkinLine[execLine][0];
                tempPassengerID = p->id;
                for (i = 1; i < 21; i++)
                {
                    checkinLine[execLine][i-1] = liaisonLine[execLine][i];
                    if (checkinLine[execLine][i] != NULL) len++;
                }
                checkinLine[execLine][20] = NULL;
                Printf("Airline check-in staff %d of airline %d serves an executive class passenger and economy class line length = %d\n", 111, 3, checkinArray[ciCount]->id*100*100 + checkinArray[ciCount]->airline*100 + len);
            }
            else if (checkinLine[checkinArray[ciCount]->id][0] != NULL)
            {
                p = checkinLine[checkinArray[ciCount]->id][0];
                tempPassengerID = p->id;
                for (i = 1; i < 21; i++)
                {
                    checkinLine[checkinArray[ciCount]->id][i-1] = liaisonLine[checkinArray[ciCount]->id][i];
                    if (checkinLine[checkinArray[ciCount]->id][i] != NULL) len++;
                }
                checkinLine[checkinArray[ciCount]->id][20] = NULL;
                Printf("Airline check-in staff %d of airline %d serves an economy class passenger and executive class line length = %d\n", 111, 3, checkinArray[ciCount]->id*100*100 + checkinArray[ciCount]->airline*100 + len);
            }
            else
            {
                Acquire(checkinLock[checkinArray[ciCount]->id]);
                checkinState[checkinArray[ciCount]->id] = CI_BREAK;
                Release(checkinLineLock[checkinArray[ciCount]->airline]);
                
                Acquire(checkinArrayLock);
                for(i = 0; i < 12; i++)
                {
                    if (checkinBreakOrder[i] == NULL)
                    {
                        checkinBreakOrder[i] = checkinArray[ciCount];
                        break;
                    }
                }
                Release(checkinArrayLock);
                Wait(checkinBreakCV[checkinArray[ciCount]->id], checkinLock[checkinArray[ciCount]->id]);
                Acquire(checkinArrayLock);
                ciCount = checkinBreakOrder[0]->id;
                for (i = 1; i < 12; i++)
                {
                    checkinBreakOrder[i-1] = checkinBreakOrder[i];
                }
                checkinBreakOrder[11] = NULL;
                Release(checkinArrayLock);
                
                checkinState[checkinArray[ciCount]->id] = CI_BUSY;
                Acquire(checkinLineLock[checkinArray[ciCount]->airline]);
            }
        }
        
        Acquire(checkinLock[checkinArray[ciCount]->id]);
        Release(checkinLineLock[checkinArray[ciCount]->airline]);
        if (p != NULL)
        {
            Acquire(airlineLock[checkinArray[ciCount]->airline]);
            exec = p->ticket->executive;
            checkinArray[ciCount]->passengers++;
            bp.seatNum = airlines[checkinArray[ciCount]->airline]->seatsAssigned;
            airlines[checkinArray[ciCount]->airline]->seatsAssigned++;
            bp.gate = checkinArray[ciCount]->airline;
            p->boardingPass = &bp;
            
            if (exec)
            {
                Printf("Airline check-in staff %d of airline %d informs executive class passenger %d to board at gate %d\n", 97, 4, checkinArray[ciCount]->id*100*100*100 + checkinArray[ciCount]->airline*100*100 + p->id*100 + p->boardingPass->gate);
                Signal(checkinLineCV[execLine], checkinLineLock[checkinArray[ciCount]->airline]);
            }
            else
            {
                Printf("Airline check-in staff %d of airline %d informs economy class passenger %d to board at gate %d\n", 95, 4, checkinArray[ciCount]->id*100*100*100 + checkinArray[ciCount]->airline*100*100 + p->id*100 + p->boardingPass->gate);
                Signal(checkinLineCV[checkinArray[ciCount]->id], checkinLineLock[checkinArray[ciCount]->airline]);
            }
            Release(airlineLock[checkinArray[ciCount]->airline]);
            
            Acquire(conveyorLock);
            for (i = 0; i < 3; i++)
            {
                Luggage* bag = p->bags[i];
                p->bags[i] = NULL;
                bag->airlineCode = checkinArray[ciCount]->airline;
                conveyor[conveyorSize] = bag;
                checkinArray[ciCount]->luggage++;
                checkinArray[ciCount]->weight += bag->weight;
            }
            Printf("Airline check-in staff %d of airline %d dropped bags to the conveyor system\n", 76, 2, checkinArray[ciCount]->id*100 + checkinArray[ciCount]->airline);
            p = NULL;
            Release(conveyorLock);
        }
        
        if (requestingCheckinData[checkinArray[ciCount]->id])
        {
        	Printf("Checkin %d is going to give manager data\n", 41, 1, checkinArray[ciCount]->id);
            Acquire(checkinManagerLock);
            Acquire(checkinLock[checkinArray[ciCount]->id]);
            Signal(checkinManagerCV, checkinManagerLock);
            Release(checkinManagerLock);
            Wait(checkinCV[checkinArray[ciCount]->id], checkinLock[checkinArray[ciCount]->id]);
            Acquire(checkinArrayLock);
            ciCount = checkinManagerInteractionOrder[0]->id;
            for (i = 1; i < 12; i++)
            {
                checkinManagerInteractionOrder[i-1] = checkinManagerInteractionOrder[i];
            }
            checkinManagerInteractionOrder[11] = NULL;
            Release(checkinArrayLock);
            
            requestingCheckinData[checkinArray[ciCount]->id] = false;
        }
        
        Acquire(airlineLock[checkinArray[ciCount]->airline]);
        if (airlines[checkinArray[ciCount]->id]->seatsAssigned >= airlines[checkinArray[ciCount]->id]->ticketsIssued)
        {
            Acquire(checkinLock[checkinArray[ciCount]->id]);
            checkinState[checkinArray[ciCount]->id] = CI_CLOSED;
            Release(airlineLock[checkinArray[ciCount]->airline]);
            if (finalCheckin[checkinArray[ciCount]->id]) Exit(0);
            else
            {
                Wait(checkinBreakCV[checkinArray[ciCount]->id], checkinLock[checkinArray[ciCount]->id]);
                /* manager interaction queue */
            }
            Printf("Airline check-in staff %d is closing the counter\n", 49, 1, checkinArray[ciCount]->id);
            Acquire(airlineLock[checkinArray[ciCount]->airline]);
            finalCheckin[checkinArray[ciCount]->id] = true;
        }
        Release(airlineLock[checkinArray[ciCount]->airline]);
    }
    Exit(0); /* should never reach this line */
}

void RunCargo()
{
    int i, cCount;
    Luggage* bag;
    
    Acquire(cargoArrayLock);
    cargoArray[cCount]->id = cargoCount;
    cargoCount++;
    for (i = 0; i < 3; i++)
    {
        cargoArray[cCount]->luggage[i] = 0;
        cargoArray[cCount]->weight[i] = 0;
    }
    /*cargoArray[cargoArray[cCount]->id] = &c;*/
    Release(cargoArrayLock);
    
    while (true)
    {
        Acquire(conveyorLock);
        if (conveyorSize == 0)
        {
            Acquire(cargoLock[cargoArray[cCount]->id]);
            if (cargoState[cargoArray[cCount]->id] != C_BREAK)
            {
                Printf("Cargo Handler %d is going for a break\n", 38, 1, cargoArray[cCount]->id);
                cargoState[cargoArray[cCount]->id] = C_BREAK;
            }
            Release(conveyorLock);
            Wait(cargoDataCV[cargoArray[cCount]->id], cargoLock[cargoArray[cCount]->id]);
            
            Acquire(conveyorLock);
            if (conveyorSize > 0)
            {
                Printf("Cargo Handler %d returned from break\n", 37, 1, cargoArray[cCount]->id);
                cargoState[cargoArray[cCount]->id] = C_BUSY;
            }
            Release(conveyorLock);
        }
        else if (cargoState[cargoArray[cCount]->id] == C_BUSY)
        {
            bag = conveyor[conveyorSize - 1];
            conveyor[conveyorSize - 1] = NULL;
            conveyorSize--;
            Printf("Cargo Handler %d picked bag of airline %d weighing %d lbs\n", 58, 3, cargoArray[cCount]->id*100*100 + bag->airlineCode*100 + bag->weight);
            aircraft[bag->airlineCode][aircraftCount[bag->airlineCode]] = bag;
            cargoArray[cCount]->luggage[bag->airlineCode]++;
            cargoArray[cCount]->weight[bag->airlineCode] += bag->weight;
            Release(conveyorLock);
        }
        if (requestingCargoData[cargoArray[cCount]->id])
        {
            Acquire(cargoManagerLock);
            Acquire(cargoDataLock[cargoArray[cCount]->id]);
            Signal(cargoManagerCV[cargoArray[cCount]->id], cargoManagerLock);
            Release(cargoManagerLock);
            /* manager interaction queue */
            Wait(cargoDataCV[cargoArray[cCount]->id], cargoDataLock[cargoArray[cCount]->id]);
            /* manager interaction queue */
            requestingCargoData[cargoArray[cCount]->id] = false;
        }
    }
    Exit(0);
}

void ManagerPrint(){
	
	int a;
    int totalLiaisonPassengers  = 0;
    int totalCheckinPassengers  = 0;
    int totalSecurityPassengers = 0;
	Printf("\n",1,0,0);
	for (a = 0; a < 3; a++){
        totalLiaisonPassengers  += manager.liaisonPassengerCount[a];
        totalCheckinPassengers  += manager.checkinPassengerCount[a];
        totalSecurityPassengers += manager.securityInspectorPassengerCount[a];
    }

    Printf("Passenger count reported by airport liaison = %d\n", 49, 1, totalLiaisonPassengers);
    Printf("Passenger count reported by airline check-in staff = %d\n", 56, 1, totalCheckinPassengers);
    Printf("Passenger count reported by security inspector = %d\n", 52, 1, totalSecurityPassengers);

    for(a = 0; a < 3; a++){
        Printf("From setup: Baggage count of airline %d = %d\n", 45, 2, a*100 + airlines[a]->totalBagCount);
        Printf("From airport liaison: Baggage count of airline %d = %d\n", 55, 2, a*100 + manager.liaisonBaggageCount[a]);
        Printf("From cargo handlers: Baggage count of airline %d = %d\n", 54, 2, a*100 + manager.cargoHandlersBaggageCount[a]);
        Printf("From setup: Baggage weight of airline %d = %d\n", 46, 2, a*100 + airlines[a]->totalBagWeight);
        Printf("From airline check-in staff: Baggage weight of airline %d = %d\n", 63, 2, a*100 + manager.checkinBaggageWeight[a]);
        Printf("From cargo handlers: Baggage weight of airline %d = %d\n", 55, 2, a*100 + manager.cargoHandlersBaggageWeight[a]);    	
    }

	Printf("\n",1,0,0);
}

void LiaisonDataRequest(){
	int i, j, k, elementNum[2];
		/*Gather data from liaisons*/
	for (i = 0; i < 3; i++) {	
		manager.liaisonPassengerCount[i] = 0;
		manager.liaisonBaggageCount[i] = 0;
	}

	for(j = 0; j < 5; j++){
		if(liaisonState[j] == L_FREE){
			Acquire(liaisonManagerLock);
			requestingLiaisonData[j] = true;
			Signal(liaisonCV[j], liaisonLock[j]);

			elementNum[0] = 0;
		    for (i = 0; i < 5; i++){
				if(liaisonManagerInteractionOrder[i] != NULL)
					elementNum[0]++;	
				else
					break;
			}
			/*Printf("elementNum = %d\n", 16, 1, elementNum[0]);*/
			/*put the fifo queue here if needed*/
			liaisonManagerInteractionOrder[elementNum[0]] = liaisonArray[j];
			Printf("Requesting data from Liaison %d\n", 32, 1,liaisonArray[j]->id);
			Wait(liaisonManagerCV, liaisonManagerLock);
			

			/*Waits for the signal of corresponding Liaison*/
			Acquire(liaisonLock[j]);
			
			for (k = 0; k < 3; k++) {
				manager.liaisonPassengerCount[k] += liaisonArray[j]->passengers[k];
				manager.liaisonBaggageCount[k] += liaisonArray[j]->luggage[k];
			}

			Signal(liaisonCV[j], liaisonLock[j]);


			Release(liaisonLock[j]);
		}
	}
}

void CheckinDataRequest()
{
    int i, j, k;
    int newCheckinBaggageWeight[3];
	int newCheckinPassengerCount[3];
    
	for (i = 0; i < 3; i++)
    {
		newCheckinPassengerCount[i] = 0;
		newCheckinBaggageWeight[i] = 0;
	}

	for (j = 0; j < 12; j++)
    {
		if (j%4 != 0) /* not 0, 4, or 8 */
        {
            if (! finalCheckin[j])
            {
                Acquire(checkinManagerLock);
                requestingCheckinData[j] = true;
                Signal(checkinBreakCV[j], checkinLock[j]);
                
                Acquire(checkinArrayLock);
                for(i = 0; i < 12; i++)
                {
                    if (checkinManagerInteractionOrder[i] == NULL)
                    {
                        checkinManagerInteractionOrder[i] = checkinArray[j];
                        break;
                    }
                }
                Release(checkinArrayLock);
            
                Wait(checkinManagerCV, checkinManagerLock);
                
                
                Acquire(checkinLock[j]);
                newCheckinPassengerCount[checkinArray[j]->airline] += checkinArray[j]->passengers;
                newCheckinBaggageWeight[checkinArray[j]->airline]  += checkinArray[j]->weight;
                Signal(checkinCV[j], checkinLock[j]);
                Release(checkinLock[j]);
            }
        }
	}
	for (k = 0; k < 3; k++)
    {
		if (newCheckinPassengerCount[k] > manager.checkinPassengerCount[k])
            manager. checkinPassengerCount[k] = newCheckinPassengerCount[k];
		if (newCheckinBaggageWeight[k]  > manager.checkinBaggageWeight[k])
            manager.checkinBaggageWeight[k]  = newCheckinBaggageWeight[k];
	}
}

void CargoDataRequest()
{
    int i, j, k;
    
	for (i = 0; i < 3; i++)
    {
		manager.cargoHandlersBaggageCount[i] = 0;
		manager.cargoHandlersBaggageWeight[i] = 0;
	}

	for (j = 0; j < 6; j++) {

			Acquire(cargoManagerLock);
			requestingCargoData[j] = true;
			Signal(cargoDataCV[j], cargoLock[j]);
            
			Wait(cargoManagerCV[j], cargoManagerLock);
			Acquire(cargoDataLock[j]);
			for (k = 0; k < 3; k++) {
				manager.cargoHandlersBaggageWeight[k] += cargoArray[j]->weight[k];
				manager.cargoHandlersBaggageCount[k] += cargoArray[j]->luggage[k];
			}
			Signal(cargoDataCV[j], cargoDataLock[j]);
			Release(cargoDataLock[j]);

	}
}

void RunManager(){
	int i,j,k,l,m,arrayCount;
	cargoHandlersOnBreak = false;
	liaisonDone = false;
	CargoDone = false;
	CheckinDone = false;
	ready = true;
	counter = 0;
	clearAirlineCount =0;

	for(i =0; i < 3; i++){
		manager.liaisonBaggageCount[i] = 0;
		manager.cargoHandlersBaggageWeight[i] = 0;
		manager.checkinBaggageWeight[i] = 0;
		manager.cargoHandlersBaggageCount[i] = 0;
    	manager.liaisonPassengerCount[i] = 0;	
    	manager.checkinPassengerCount[i] = 0;
    	manager.securityInspectorPassengerCount[i] = 0;
	}

	while(true){
		Acquire(conveyorLock);

		arrayCount = 0;
	    for (i = 0; i < 63; i++){
			if(conveyor[i] != NULL){
				arrayCount++;	
				Printf("poo\n", 4,0,0);			
			}
		}

		if(arrayCount > 0){
			counter = 0;

			for ( j = 0; j < 6; ++j){
				if(cargoState[j] == C_BREAK)
					counter++;
			}

			if(counter == 6){
				Printf("Airport manager calls back all the cargo handlers from break\n",61,0,0);
				for(k = 0; k < 6; k++){
					Acquire(cargoLock[k]);
					Signal(cargoDataCV[k], cargoLock[k]);
				}

				for(k = 0; k < 6; k++)
					Release(cargoLock[k]);
			}
		}

		Release(conveyorLock);

		LiaisonDataRequest();
		CheckinDataRequest();
		CargoDataRequest();

		for(m = 0; m < 3; m++){
			if(!clearAirline[m]){
				Acquire(airlineLock[m]);
				if(manager.securityInspectorPassengerCount[m] >= airlines[m]->ticketsIssued
					&& 21 >= airlines[m]->totalBagCount) {
					Printf("Airport manager gives a boarding call to airline %d\n", 53, 1, m);
					Acquire(boardingLock[m]);
					Broadcast(boardingCV[m], boardingLock[m]);
					clearAirline[m] = true;
					clearAirlineCount++;
					Release(boardingLock[m]);
				}
				Release(airlineLock[m]);
			}	
		}

		if (clearAirlineCount == 3) {
            ManagerPrint();
			Exit(0);
		}

		for(l = 0; l < 10; l++)
			Yield();
	}
}

int main()
{
    int i;
    Printf("Initializing Airport\n",21,0,0);
    Init();
    for (i = 0; i < 5; i++)
    {
    	Fork(forkPassenger, "Passenger", 9);
	}	
    for (i = 0; i < 5; i++)
    {
        Fork(RunLiaison, "Liaison", 7);
    }
    for (i = 0; i < 9; i++)
    {
        Fork(RunCheckin, "CheckIn", 7);
    }
    for (i = 0; i < 6; i++)
    {
        Fork(RunCargo, "Cargo", 5);
    }
	Fork(RunManager, "Manager",7);  
}
