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
	Liaison*  LiaisonPassengerInteractionOrder[5];
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
int checkinManagerCV[12];
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
	int counter;

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
            conveyor[(i+1)*(j+1)-1] = NULL;
            aircraft[i][j] = NULL;
        }
    }
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
        checkinArray[i] = NULL;
        checkinLineCV[i] = CreateCondition("CheckinLineCV", 13);
        checkinCV[i] = CreateCondition("CheckinCV", 9);
        checkinLock[i] = CreateLock("CheckinLock", 11);
        checkinBreakCV[i] = CreateCondition("CheckinBreakCV", 14);
        checkinManagerCV[i] = CreateCondition("CheckinManagerCV", 16);
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
        cargoArray[i] = NULL;
        cargoDataCV[i] = CreateCondition("CargoDataCV", 11);
        cargoDataLock[i] = CreateLock("CargoDataLock", 13);
        cargoManagerCV[i] = CreateCondition("CargoManagerCV", 14);
        cargoLock[i] = CreateLock("CargoLock", 9);
        cargoState[i] = C_BUSY;
        requestingCargoData[i] = false;
    }

    /* Manager variables */
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
    for(i = 0; i < 5;i++)
    	p.lineLength[i] = 0;

	for(j = 0; j < 21; j++){
		if(liaisonLine[0][j] == NULL)
			break;
		else{
			p.lineLength[0]++;
		}
	}

	p.minValue = p.lineLength[0];
	/*Printf("lineLength: %d\n", 15, 1, p.lineLength[0]);
	Printf("min value: %d\n", 14, 1, p.minValue);
	/*finds the shortest liaison line*/
	if(!CISline){
		for (i = 1; i < 5; i++) {

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



		for(i = CIS_ID;  i < CIS_ID +3; i++){
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
		 53, 2, p.location*100 + p.minValue);
		return p.location;		
	}
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
    /*
    Printf("p id is now: %d\n", 16,1,p->id);
    */
	Acquire(liaisonLock[passengerArray[p->id]->myLine]);
	/*Give liaison information
	*/
	Signal(liaisonCV[passengerArray[p->id]->myLine], liaisonLock[passengerArray[p->id]->myLine]);
	/*wait for liaison confirmation*/
	/*Printf("FU\n",3,0,0);*/
    Acquire(liaisonArrayLock);		/*prevent alteration while calculating the # of elements in
    								  the array*/

    elementCount = 0;
    for (i = 0; i < 5; i++){
		if(LiaisonPassengerInteractionOrder[i] != NULL){
			elementCount++;	
			/*Printf("daf %d\n", 7,1,passengerArray[p->id]->id);*/			
		}
	}
	/*add passenger to the end of the array*/
    LiaisonPassengerInteractionOrder[elementCount] = liaisonArray[p->myLine];

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

	/*move to the designated checkin counter
	PassengerFindShortestCISLine(passengerArray[p->id]);
	*/
	Exit(0);
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
}

void RunLiaison()
{
    int i, elementCount, lCount, tempPassengerID, order;
    int var[10];

    Passenger* p;
    
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
        p = liaisonLine[lCount][0];
		/*      	for(i = 0; i <21; i++){
      		Printf("Passenger %d\n", 13, 1, liaisonLine[1][i]->id);
      	}
 		*/		
      	/*tempPassengerID = p->id;*/
        if (p != NULL){
        	tempPassengerID = p->id;
        	/*Printf("id: %d\n", 7,1,p->id);*/

        	liaisonArray[lCount]->myPassengerID = p->id;


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
        	/*
        	Printf("var[1]: %d\n", 17, 1, var[1] + 1);*/
        }
        else
        {
            liaisonState[lCount] = L_FREE;
        }
        Acquire(liaisonLock[lCount]);
        Release(liaisonLineLock);
        if (p != NULL)
        {
        	Printf("Liaison %d is waiting for Passenger %d\n", 39, 2, lCount*100 + liaisonArray[lCount]->myPassengerID);
            Wait(liaisonCV[lCount], liaisonLock[lCount]);

            Acquire(liaisonArrayLock);
            lCount = LiaisonPassengerInteractionOrder[0]->id;
		    for (i = 1; i < 5; i++)
		    {
		        LiaisonPassengerInteractionOrder[i-1] = LiaisonPassengerInteractionOrder[i];
		    }
		    LiaisonPassengerInteractionOrder[4] = NULL;
			Release(liaisonArrayLock);

            Acquire(liaisonLock[lCount]);

            Printf("liaison %d has recieved a response from passenger %d\n",53,2,lCount*100 + liaisonArray[lCount]->myPassengerID);
            order = liaisonArray[lCount]->myPassengerID;

            liaisonArray[lCount]->passengers[passengerArray[order]->airline]++;
            
            for (i = 0; i < 3; i++)
            {
                liaisonArray[lCount]->luggage[passengerArray[order]->airline]++;
                liaisonArray[lCount]->weight[passengerArray[order]->airline] += passengerArray[tempPassengerID]->bags[i]->weight;
            }

            Acquire(passengerArrayLock);
            var[2] = 0;
            for (i = 0; i < 21; i++){
				if(passengerLiaisonInteractionOrder[1][i] != NULL){
					var[2]++;				
				}
			}
			/*add passenger to the end of the array*/
            passengerLiaisonInteractionOrder[1][var[2]] = passengerArray[order];
            
            Signal(liaisonCV[lCount], liaisonLock[lCount]);
			Release(passengerArrayLock);
            p = NULL;
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
    
    execLine = checkinArray[ciCount]->airline * 4;
    
    while (true)
    {
        Acquire(checkinLineLock[checkinArray[ciCount]->airline]);
        tempPassengerID = 0;
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
            Acquire(checkinManagerLock);
            Acquire(checkinLock[checkinArray[ciCount]->id]);
            Signal(checkinManagerCV[checkinArray[ciCount]->id], checkinManagerLock);
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
    cargoArray[cargoArray[cCount]->id] = &c;
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
            Wait(cargoDataCV[cargoArray[cCount]->id], cargoDataLock[cargoArray[cCount]->id]);
            /* manager interaction queue */
            requestingCargoData[cargoArray[cCount]->id] = false;
        }
    }
    Exit(0);
}

void RunManager(){
	int i,j,k,l,arrayCount;
	cargoHandlersOnBreak = false;
	liaisonDone = false;
	CargoDone = false;
	CheckinDone = false;
	ready = true;
	counter = 0;
	/*clearAirline = false;
	clearAirlineCount =0;*/

	for(i =0; i < 3; i++){
		liaisonBaggageCount[i] = 0;
		cargoHandlersBaggageWeight[i] = 0;
		checkinBaggageWeight[i] = 0;
		cargoHandlersBaggageCount[i] = 0;
    	liaisonPassengerCount[i] = 0;	
    	checkinPassengerCount[i] = 0;
    	securityInspectorPassengerCount[i] = 0;
	}

	while(true){
		Acquire(ConveyorLock);

		arrayCount = 0;
	    for (i = 0; i < 68; i++){
			if(conveyor[i] != NULL){
				arrayCount++;	
				/*Printf("daf %d\n", 7,1,passengerArray[p->id]->id);*/			
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

		Release(ConveyorLock);

		for(l = 0; l < 10; l++)
			Yield();
	}
}

void LiaisonDataRequest(){
	
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
		if (newCheckinPassengerCount[k] > checkinPassengerCount[k])
            checkinPassengerCount[k] = newCheckinPassengerCount[k];
		if (newCheckinBaggageWeight[k]  > checkinBaggageWeight[k])
            checkinBaggageWeight[k]  = newCheckinBaggageWeight[k];
	}
}

void CargoDataRequest()
{
    int i, j, k;
    
	for (i = 0; i < 3; i++)
    {
		cargoHandlersBaggageCount[i] = 0;
		cargoHandlersBaggageWeight[i] = 0;
	}

	for (j = 0; j < 6; j++) {
\
			Acquire(cargoManagerLock);
			requestingCargoData[j] = true;
			Signal(cargoDataCV[j], cargoLock[j]);
			Wait(cargoManagerCV[j], cargoManagerLock);
			Acquire(cargoDataLock[j]);
			for (k = 0; k < 3; k++) {
				cargoHandlersBaggageWeight[k] += cargoArray[j]->weight[k];
				cargoHandlersBaggageCount[k] += cargoArray[j]->luggage[k];
			}
			Signal(cargoDataCV[j], cargoDataLock[j]);
			Release(cargoDataLock[j]);

	}
}

void ManagerPrint(){
	Printf("\n",1,0,0);
	int a;
    int totalLiaisonPassengers  = 0;
    int totalCheckinPassengers  = 0;
    int totalSecurityPassengers = 0;
	for (int a = 0; a < 3; a++)
	    {
	        totalLiaisonPassengers  += liaisonPassengerCount[a];
	        totalCheckinPassengers  += checkinPassengerCount[a];
	        totalSecurityPassengers += securityInspectorPassengerCount[a];
	    }
}

int main()
{
    int i;
    
    Init();
    for (i = 0; i < 5; i++)
    {
    	Fork(forkPassenger, "Passenger", 9);
	}	
    
    for (i = 0; i < 5; i++)
    {
        Fork(RunLiaison, "Liaison", 7);
    }
   /* for (i = 0; i < 9; i++)
    {
        Fork(RunCheckin, "CheckIn", 7);
    }
    for (i = 0; i < 6; i++)
    {
        Fork(RunCargo, "Cargo", 5);
    }
	Fork(RunManager, "Manager",7);
    */
}
