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
	/**/
	Passenger*  passengerLiaisonInteractionOrder[21];
Passenger* passengerArray[21];
/* Liaison variables */
	Liaison l0;
	Liaison l1;	
	Liaison l2;
	Liaison l3;
	Liaison l4;
int liaisonCount;
int liaisonArrayLock;
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
int checkinCount;
int checkinArrayLock;
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
/* Screener variables */
/* Security variables */
/* Manager variables */

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
    for (i = 5; i < 21; i++)
    {
    	passengerLiaisonInteractionOrder[i] = NULL;
        passengerArray[i] = NULL;
    }
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
        liaisonLineCV[i] = CreateCondition("LiaisonLineCV", 13);
        liaisonLock[i] = CreateLock("LiaisonLock", 11);
        liaisonCV[i] = CreateCondition("LiaisonCV", 9);
        liaisonState[i] = L_BUSY;
        requestingLiaisonData[i] = false;
        for (j = 0; j < 21; j++)
        {
            liaisonLine[i][j] = NULL;
        }
    }
    /* Check-in variables */
    checkinCount = 0;
    checkinArrayLock = CreateLock("CheckinArrayLock", 16);
    checkinManagerLock = CreateLock("CheckinManagerLock", 18);
    for (i = 0; i < 12; i++)
    {
        if (i%4 == 0) /* 0, 4, 8 */
        {
            checkinLineLock[i/4] = CreateLock("CheckinLineLock", 15);
            checkinState[i] = CI_NONE;
        }
        else checkinState[i] = CI_BUSY;
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
            checkinLine[i][j] = NULL;
        }
    }
    /* Cargo variables */
    cargoCount = 0;
    cargoArrayLock = CreateLock("CargoArrayLock", 14);
    cargoCV = CreateCondition("CargoCV", 7);
    cargoManagerLock = CreateLock("CargoManagerLock", 16);
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
    /* Screener variables */
    /* Security variables */
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
    int i, elementCount, hackVar;/*
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
		Release(passengerArrayLock);*/
	hackVar = 0;
	Acquire(liaisonLineLock);
	passengerArray[p->id]->myLine = findShortestLine(false, p->id);
	elementCount = 0;

	for (i = 0; i < 21; i++){
		if(liaisonLine[passengerArray[p->id]->myLine][i] != NULL)
			elementCount++;	
		else
			break;		
	}
	/*for(i = 0; i <21; i++){
		if(liaisonLine[passengerArray[p->id]->myLine][i] != NULL)
  			Printf("Passenger %d for position %d\n", 29, 2, liaisonLine[passengerArray[p->id]->myLine][i]->id*100 + i);
  		else
  			break;
  	}*/
	Printf("Passenger %d chose liaison %d with a line length of %d\n",
	 55, 3, passengerArray[p->id]->id*100*100 + passengerArray[p->id]->myLine*100 + elementCount);

	/*Printf("elementCount: %d\n", 17, 1, elementCount);*/
	liaisonLine[p->myLine][elementCount] = passengerArray[p->id];
	/*Printf("Passenger %d\n", 13, 1, passengerArray[p->id]->id);*/
	if(liaisonState[passengerArray[p->id]->myLine] == L_BUSY){
		/*Wait for an available liaison*/

		Wait(liaisonLineCV[passengerArray[p->id]->myLine], liaisonLineLock);

	}
	else
		Release(liaisonLineLock);
	Acquire(passengerArrayLock);
    p = passengerLiaisonInteractionOrder[0];
    for (i = 1; i < 21; i++)
    {
        passengerLiaisonInteractionOrder[i-1] = passengerLiaisonInteractionOrder[i];
    }
    passengerLiaisonInteractionOrder[20] = NULL;
    Release(passengerArrayLock);

    Printf("p id is now: %d\n", 16,1,p->id);
	Acquire(liaisonLock[passengerArray[p->id]->myLine]);
	/*Give liaison information
	
	Signal(liaisonCV[passengerArray[p->id]->myLine], liaisonLock[passengerArray[p->id]->myLine]);
	/*wait for liaison confirmation*/
	/*Printf("FU\n",3,0,0);
	Wait(liaisonCV[passengerArray[p->id]->myLine], liaisonLock[passengerArray[p->id]->myLine]);
*/
	Printf("Passenger %d of Airline %d is directed to the airline counter.\n",
		63, 2, passengerArray[p->id]->id*100 + passengerArray[p->id]->airline);

	/*move to the designated checkin counter*/
	/*PassengerFindShortestCISLine(passengerArray[pCount]);*/

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

void PassengerFindShortestCISLine(Passenger p){
	int j, elementCount;
	p.myLine = p.airline * 4;

	Acquire(checkinLineLock[p.ticket->airline]);
	if(!p.ticket->executive){
		p.myLine = findShortestLine(true,p.id);
		Printf("Passenger %d of Airline %d chose Airline Check-In staff %d with line length %d\n",
			79, 4, p.id*100*100*100 + p.airline*100*100 + p.myLine*100 + p.minValue);
	}
	else
		Printf("Passenger %d of Airline %d is waiting in the executive line\n", 60, 2, p.id*100 + p.airline);
	
	elementCount = 0;
	for (j = 1; j < 21; j++){
		if(checkinLine[p.myLine][elementCount] != NULL)
			elementCount++;			
	}
	/*Printf("elementCount: %d\n", 17, 1, elementCount);	*/
	checkinLine[p.myLine][elementCount] = passengerArray[p.id];

	Wait(checkinLineCV[passengerArray[p.id]->myLine], checkinLineLock[passengerArray[p.id]->airline]);
	
}

void RunLiaison()
{
    int i, elementCount, lCount, tempPassengerID, order;

    Passenger* p;
    
    Liaison l;
    lCount = liaisonCount;
    Acquire(liaisonArrayLock);
    l.id = liaisonCount;
    liaisonArray[lCount]->id = liaisonCount;
    liaisonCount++;
    for (i = 0; i < 3; i++)
    {
        liaisonArray[lCount]->passengers[i] = 0;
        liaisonArray[lCount]->luggage[i] = 0;
        liaisonArray[lCount]->weight[i] = 0;
    }
    /*liaisonArray[l.id] = &l;*/order = 0;
    Release(liaisonArrayLock);
        
    while(true)
    {
        Acquire(liaisonLineLock);
        
        tempPassengerID  = liaisonLine[lCount][0]->id;
/*      	for(i = 0; i <21; i++){
      		Printf("Passenger %d\n", 13, 1, liaisonLine[1][i]->id);
      	}
*/		
      	/*tempPassengerID = p->id;*/

        if (passengerArray[tempPassengerID] != NULL)
        {
            for (i = 1; i < 21; i++)
            {
                liaisonLine[lCount][i-1] = liaisonLine[lCount][i];
            }
            elementCount = 0;
		    for (i = 0; i < 21; i++){
				if(liaisonLine[lCount][i] != NULL){
					elementCount++;				
				}
			}
			
			Printf("elemenlCount: %d\n", 17, 1, elementCount);
			
            Signal(liaisonLineCV[lCount], liaisonLineLock);
            liaisonLine[lCount][20] = NULL;

            liaisonState[lCount] = L_BUSY;
            passengerArray[tempPassengerID]->airline = passengerArray[tempPassengerID]->ticket->airline;
            elementCount = 0;
            for (i = 0; i < 21; i++){
				if(passengerLiaisonInteractionOrder[i] != NULL){
					elementCount++;				
				}
			}
			/*add passenger to the end of the array*/
			
            passengerLiaisonInteractionOrder[elementCount] = passengerArray[tempPassengerID];
            Printf("Airport Liaison %d directed passenger %d of airline %d\n", 55, 3,
                    liaisonArray[lCount]->id*100*100 + passengerArray[tempPassengerID]->id*100 + passengerArray[tempPassengerID]->airline);
        	
        	Printf("elementCount: %d\n", 17, 1, elementCount + 1);
        }
        else
        {
            liaisonState[lCount] = L_FREE;
        }
        Acquire(liaisonLock[lCount]);
        Release(liaisonLineLock);
        if (passengerArray[tempPassengerID] != NULL)
        {
            Wait(liaisonCV[lCount], liaisonLock[lCount]);

            Acquire(liaisonLock[lCount]);
            liaisonArray[lCount]->passengers[passengerArray[tempPassengerID]->airline]++;
            for (i = 0; i < 3; i++)
            {
                liaisonArray[lCount]->luggage[passengerArray[tempPassengerID]->airline]++;
                liaisonArray[lCount]->weight[passengerArray[tempPassengerID]->airline] += passengerArray[tempPassengerID]->bags[i]->weight;
            }
            Signal(liaisonCV[lCount], liaisonLock[lCount]);
            Release(liaisonLock[lCount]);
        }
        else
        {
            Wait(liaisonCV[lCount], liaisonLock[lCount]);
        }
        if (requestingLiaisonData[lCount])
        {
            Acquire(liaisonManagerLock);
            Acquire(liaisonLock[lCount]);
            Signal(liaisonManagerCV, liaisonManagerLock);
            Release(liaisonManagerLock);
            Wait(liaisonCV[lCount], liaisonLock[lCount]);
            requestingLiaisonData[lCount] = false;
        }
    }
    Exit(0);
}

void RunCheckin()
{
    int i, j, len;
    int execLine;
    Passenger* p;
    BoardingPass bp;
    bool exec;
    bool talked = false;
    
    Checkin ci;
    Acquire(checkinArrayLock);
    while (checkinCount%4 == 0) /* 0, 4, 8 */
    {
        checkinCount++;
    }
    ci.id = checkinCount;
    ci.airline = ci.id/4;
    checkinCount++;
    ci.passengers = 0;
    ci.luggage = 0;
    ci.weight = 0;
    checkinArray[ci.id] = &ci;
    Release(checkinArrayLock);
    
    execLine = ci.airline * 4;
    
    while (true)
    {
        Acquire(checkinLineLock[ci.airline]);
        if (checkinState[ci.id] != CI_CLOSED)
        {
            len = 0;
            if (checkinLine[execLine][0] != NULL)
            {
                p = checkinLine[execLine][0];
                for (i = 1; i < 21; i++)
                {
                    checkinLine[execLine][i-1] = liaisonLine[execLine][i];
                    if (checkinLine[execLine][i] != NULL) len++;
                }
                checkinLine[execLine][20] = NULL;
                Printf("Airline check-in staff %d of airline %d serves an executive class passenger and economy class line length = %d\n", 111, 3, ci.id*100*100 + ci.airline*100 + len);
            }
            else if (checkinLine[ci.id][0] != NULL)
            {
                p = checkinLine[ci.id][0];
                for (i = 1; i < 21; i++)
                {
                    checkinLine[ci.id][i-1] = liaisonLine[ci.id][i];
                    if (checkinLine[ci.id][i] != NULL) len++;
                }
                checkinLine[ci.id][20] = NULL;
                Printf("Airline check-in staff %d of airline %d serves an economy class passenger and executive class line length = %d\n", 111, 3, ci.id*100*100 + ci.airline*100 + len);
            }
            else
            {
                Acquire(checkinLock[ci.id]);
                checkinState[ci.id] = CI_BREAK;
                Release(checkinLineLock[ci.airline]);
                Wait(checkinBreakCV[ci.id], checkinLock[ci.id]);
                checkinState[ci.id] = CI_BUSY;
                Acquire(checkinLineLock[ci.airline]);
            }
        }
        Acquire(checkinLock[ci.id]);
        Release(checkinLineLock[ci.airline]);
        if (p != NULL)
        {
            Acquire(airlineLock[ci.airline]);
            exec = p->ticket->executive;
            ci.passengers++;
            bp.seatNum = airlines[ci.airline]->seatsAssigned;
            airlines[ci.airline]->seatsAssigned++;
            bp.gate = ci.airline;
            p->boardingPass = &bp;
            
            if (exec)
            {
                Printf("Airline check-in staff %d of airline %d informs executive class passenger %d to board at gate %d\n", 97, 4, ci.id*100*100*100 + ci.airline*100*100 + p->id*100 + p->boardingPass->gate);
                Signal(checkinLineCV[execLine], checkinLineLock[ci.airline]);
            }
            else
            {
                Printf("Airline check-in staff %d of airline %d informs economy class passenger %d to board at gate %d\n", 95, 4, ci.id*100*100*100 + ci.airline*100*100 + p->id*100 + p->boardingPass->gate);
                Signal(checkinLineCV[ci.id], checkinLineLock[ci.airline]);
            }
            Release(airlineLock[ci.airline]);
            
            Acquire(conveyorLock);
            for (i = 0; i < 3; i++)
            {
                Luggage* bag = p->bags[i];
                p->bags[i] = NULL;
                bag->airlineCode = ci.airline;
                conveyor[conveyorSize] = bag;
                ci.luggage++;
                ci.weight += bag->weight;
            }
            Printf("Airline check-in staff %d of airline %d dropped bags to the conveyor system\n", 76, 2, ci.id*100 + ci.airline);
            p = NULL;
            Release(conveyorLock);
        }
        
        if (requestingCheckinData[ci.id])
        {
            Acquire(checkinManagerLock);
            Acquire(checkinLock[ci.id]);
            Signal(checkinManagerCV[ci.id], checkinManagerLock);
            Release(checkinManagerLock);
            Wait(checkinCV[ci.id], checkinLock[ci.id]);
            requestingCheckinData[ci.id] = false;
        }
        
        Acquire(airlineLock[ci.airline]);
        if (airlines[ci.id]->seatsAssigned >= airlines[ci.id]->ticketsIssued)
        {
            Acquire(checkinLock[ci.id]);
            checkinState[ci.id] = CI_CLOSED;
            Release(airlineLock[ci.airline]);
            if (finalCheckin[ci.id]) Exit(0);
            else Wait(checkinBreakCV[ci.id], checkinLock[ci.id]);
            Printf("Airline check-in staff %d is closing the counter\n", 49, 1, ci.id);
            Acquire(airlineLock[ci.airline]);
            finalCheckin[ci.id] = true;
        }
        Release(airlineLock[ci.airline]);
    }
    Exit(0); /* should never reach this line */
}

void RunCargo()
{
    int i;
    Luggage* bag;
    
    Cargo c;
    Acquire(cargoArrayLock);
    c.id = cargoCount;
    cargoCount++;
    for (i = 0; i < 3; i++)
    {
        c.luggage[i] = 0;
        c.weight[i] = 0;
    }
    cargoArray[c.id] = &c;
    Release(cargoArrayLock);
    
    while (true)
    {
        Acquire(conveyorLock);
        if (conveyorSize == 0)
        {
            Acquire(cargoLock[c.id]);
            if (cargoState[c.id] != C_BREAK)
            {
                Printf("Cargo Handler %d is going for a break\n", 38, 1, c.id);
                cargoState[c.id] = C_BREAK;
            }
            Release(conveyorLock);
            Wait(cargoDataCV[c.id], cargoLock[c.id]);
            Acquire(conveyorLock);
            if (conveyorSize > 0)
            {
                Printf("Cargo Handler %d returned from break\n", 37, 1, c.id);
                cargoState[c.id] = C_BUSY;
            }
            Release(conveyorLock);
        }
        else if (cargoState[c.id] == C_BUSY)
        {
            bag = conveyor[conveyorSize - 1];
            conveyor[conveyorSize - 1] = NULL;
            conveyorSize--;
            Printf("Cargo Handler %d picked bag of airline %d weighing %d lbs\n", 58, 3, c.id*100*100 + bag->airlineCode*100 + bag->weight);
            aircraft[bag->airlineCode][aircraftCount[bag->airlineCode]] = bag;
            c.luggage[bag->airlineCode]++;
            c.weight[bag->airlineCode] += bag->weight;
            Release(conveyorLock);
        }
        if (requestingCargoData[c.id])
        {
            Acquire(cargoManagerLock);
            Acquire(cargoDataLock[c.id]);
            Signal(cargoManagerCV[c.id], cargoManagerLock);
            Release(cargoManagerLock);
            Wait(cargoDataCV[c.id], cargoDataLock[c.id]);
            requestingCargoData[c.id] = false;
        }
    }
    Exit(0);
}

int main()
{
    int i;
    
    Init();
    /*for (i = 0; i < 5; i++)
    {
    	Fork(PassengerFindShortestLiaisonLine, "Passenger", 9);
	}	*/

	Fork(forkPassenger, "Passenger0", 10);
	Fork(forkPassenger, "Passenger1", 10);
	Fork(forkPassenger, "Passenger2", 10);
	Fork(forkPassenger, "Passenger3", 10);
	Fork(forkPassenger, "Passenger4", 10);
    
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
    }*/
}
