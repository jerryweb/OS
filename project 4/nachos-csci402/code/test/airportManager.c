/* Networked airport simulation.
 *  Initializes the manager.
 */

#include "syscall.h"
#include "airportStructs.h"

#define NULL (void*)0

/* Personal variables */

Manager manager;
int clearAirlineCount;
bool clearAirline[3];

/* General variables */

int numAirlines; /* const (never changes) */
int airlineList; /* mv size 3, Airline* */
int airlineLockList; /* mv size 3, int (lock) */
int conveyor; /* mv size 53, Luggage* */
int conveyorLock; /* lock */
int conveyorSize; /* mv size 1, int */
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
int passengerListLock; /* lock */

/* Liaison variables */

int liaisonList; /* mv size 5, Liaison* */
int liaisonListLock; /* lock */
int liaisonLineList; /* mv size 5, int (MV) */
int liaisonLine1; /* mv size 21, Passenger* */
int liaisonLine2; /* mv size 21, Passenger* */
int liaisonLine3; /* mv size 21, Passenger* */
int liaisonLine4; /* mv size 21, Passenger* */
int liaisonLine5; /* mv size 21, Passenger* */
int liaisonManagerLock; /* lock */
int liaisonManagerCV; /* CV */
int liaisonLineLock; /* lock */
int liaisonLineCVList; /* mv size 5, int (CV) */
int liaisonLockList; /* mv size 5, int (lock) */
int liaisonCVList; /* mv size 5, int (CV) */
int liaisonStateList; /* mv size 5, LiaisonState */
int requestingLiaisonDataList; /* mv size 5, bool */

/* Check-in variables */

int checkinList; /* mv size 12, Checkin* */
int checkinListLock; /* lock */
int checkinLineList; /* mv size 12, int (MV) */
int checkinLine1; /* mv size 21, Passenger* */
int checkinLine2; /* mv size 21, Passenger* */
int checkinLine3; /* mv size 21, Passenger* */
int checkinLine4; /* mv size 21, Passenger* */
int checkinLine5; /* mv size 21, Passenger* */
int checkinLine6; /* mv size 21, Passenger* */
int checkinLine7; /* mv size 21, Passenger* */
int checkinLine8; /* mv size 21, Passenger* */
int checkinLine9; /* mv size 21, Passenger* */
int checkinLine10; /* mv size 21, Passenger* */
int checkinLine11; /* mv size 21, Passenger* */
int checkinLine12; /* mv size 21, Passenger* */
int checkinLineLockList; /* mv size 3, int (lock) */
int checkinLineCVList; /* mv size 12, int (CV) */
int checkinCVList; /* mv size 12, int (CV) */
int checkinLockList; /* mv size 12, int (lock) */
int checkinBreakCVList; /* mv size 12, int (CV) */
int checkinManagerLock; /* lock */
int checkinManagerCV; /* CV */
int checkinStateList; /* mv size 12, CheckinState */
int finalCheckinList; /* mv size 12, bool */
int requestingCheckinDataList; /* mv size 12, bool */

/* Cargo variables */

int cargoListLock; /* lock */
int cargoList; /* mv size 5, Cargo* */
int cargoCVList; /* mv size 5, int (CV) */
int cargoDataCVList; /* mv size 5, int (CV) */
int cargoDataLockList; /* mv size 5, int (lock) */
int cargoManagerLock; /* lock */
int cargoManagerCVList; /* mv size 5, int (CV) */
int cargoLockList; /* mv size 5, int (lock) */
int cargoStateList; /* mv size 5, CargoState */
int requestingCargoDataList; /* mv size 5, bool */

void CreateVariables()
{
    int i;
    
    /* General variables */
    
    numAirlines = 3;
    airlineList = CreateMonitorVariable("airlineList", 11, 3);
    airlineLockList = CreateMonitorVariable("airlineLockList", 15, 3);
    conveyor = CreateMonitorVariable("conveyor", 8, 63);
    conveyorLock = CreateLock("conveyorLock", 12);
    conveyorSize = CreateMonitorVariable("conveyor", 12, 1);
    aircraftList = CreateMonitorVariable("aircraftList", 12, 3);
    aircraft1 = CreateMonitorVariable("aircraft1", 9, 21);
    aircraft2 = CreateMonitorVariable("aircraft2", 9, 21);
    aircraft3 = CreateMonitorVariable("aircraft3", 9, 21);
    SetMonitorVariable(aircraftList, 0, aircraft1);
    SetMonitorVariable(aircraftList, 1, aircraft2);
    SetMonitorVariable(aircraftList, 2, aircraft3);
    aircraftCountList = CreateMonitorVariable("aircraftCountList", 17, 3);
    boardingQueueList = CreateMonitorVariable("boardingQueueList", 17, 3);
    boardingQueue1 = CreateMonitorVariable("boardingQueue1", 14, 21);
    boardingQueue2 = CreateMonitorVariable("boardingQueue2", 14, 21);
    boardingQueue3 = CreateMonitorVariable("boardingQueue3", 14, 21);
    SetMonitorVariable(boardingQueueList, 0, boardingQueue1);
    SetMonitorVariable(boardingQueueList, 1, boardingQueue2);
    SetMonitorVariable(boardingQueueList, 2, boardingQueue3);
    boardingCVList = CreateMonitorVariable("boardingCVList", 14, 3);
    boardingLockList = CreateMonitorVariable("boardingLockList", 16, 3);
    for (i = 0; i < 3; i++)
    {
        if (! GetMonitorVariable(airlineList, i))
        {
            Airline al;
            al.id = i;
            al.seatsAssigned = 0;
            al.ticketsIssued = 7;
            al.totalBagCount = al.ticketsIssued * 3;
            al.totalBagWeight = al.totalBagCount * 30;
            SetMonitorVariable(airlineList, i, (int)&al);
        }
        SetMonitorVariable(airlineLockList, i, CreateLock("airlineLock", 11));
        SetMonitorVariable(boardingCVList, i, CreateCondition("boardingCV", 10));
        SetMonitorVariable(boardingLockList, i, CreateLock("boardingLock", 12));
    }
    
    /* Passenger variables */
    
    passengerListLock = CreateLock("passengerListLock", 17);
    passengerList = CreateMonitorVariable("passengerList", 13, 21);

    /* Liaison variables */
    
    liaisonList = CreateMonitorVariable("liaisonList", 11, 5);
    liaisonListLock = CreateLock("liaisonListLock", 16);
    liaisonLineList = CreateMonitorVariable("liaisonLineList", 15, 5);
    liaisonLine1 = CreateMonitorVariable("liaisonLine1", 12, 21);
    liaisonLine2 = CreateMonitorVariable("liaisonLine2", 12, 21);
    liaisonLine3 = CreateMonitorVariable("liaisonLine3", 12, 21);
    liaisonLine4 = CreateMonitorVariable("liaisonLine4", 12, 21);
    liaisonLine5 = CreateMonitorVariable("liaisonLine5", 12, 21);
    SetMonitorVariable(liaisonLineList, 0, liaisonLine1);
    SetMonitorVariable(liaisonLineList, 1, liaisonLine2);
    SetMonitorVariable(liaisonLineList, 2, liaisonLine3);
    SetMonitorVariable(liaisonLineList, 3, liaisonLine4);
    SetMonitorVariable(liaisonLineList, 4, liaisonLine5);
    liaisonManagerLock = CreateLock("liaisonManagerLock", 18);
    liaisonManagerCV = CreateCondition("liaisonManagerCV", 16);
    liaisonLineLock = CreateLock("liaisonLineLock", 15);
    liaisonLineCVList = CreateMonitorVariable("requestingLiaisonDataList", 17, 5);
    liaisonLockList = CreateMonitorVariable("liaisonLockList", 15, 5);
    liaisonCVList = CreateMonitorVariable("liaisonCVList", 13, 5);
    liaisonStateList = CreateMonitorVariable("liaisonStateList", 16, 5);
    requestingLiaisonDataList = CreateMonitorVariable("requestingLiaisonDataList", 25, 5);
    for (i = 0; i < 5; i++)
    {
        SetMonitorVariable(liaisonLineCVList, i, CreateCondition("liaisonLineCV", 13));
        SetMonitorVariable(liaisonLockList, i, CreateLock("liaisonLock", 11));
        SetMonitorVariable(liaisonCVList, i, CreateCondition("liaisonCV", 9));
    }
    
    /* Check-in variables */
    
    checkinList = CreateMonitorVariable("checkinList", 11, 12);
    checkinListLock = CreateLock("checkinListLock", 15);
    checkinLineList = CreateMonitorVariable("checkinLineList", 15, 12);
    checkinLine1  = CreateMonitorVariable("checkinLine1",  12, 21);
    checkinLine2  = CreateMonitorVariable("checkinLine2",  12, 21);
    checkinLine3  = CreateMonitorVariable("checkinLine3",  12, 21);
    checkinLine4  = CreateMonitorVariable("checkinLine4",  12, 21);
    checkinLine5  = CreateMonitorVariable("checkinLine5",  12, 21);
    checkinLine6  = CreateMonitorVariable("checkinLine6",  12, 21);
    checkinLine7  = CreateMonitorVariable("checkinLine7",  12, 21);
    checkinLine8  = CreateMonitorVariable("checkinLine8",  12, 21);
    checkinLine9  = CreateMonitorVariable("checkinLine9",  12, 21);
    checkinLine10 = CreateMonitorVariable("checkinLine10", 12, 21);
    checkinLine11 = CreateMonitorVariable("checkinLine11", 12, 21);
    checkinLine12 = CreateMonitorVariable("checkinLine12", 12, 21);
    SetMonitorVariable(checkinLineList, 0,  checkinLine1);
    SetMonitorVariable(checkinLineList, 1,  checkinLine2);
    SetMonitorVariable(checkinLineList, 2,  checkinLine3);
    SetMonitorVariable(checkinLineList, 3,  checkinLine4);
    SetMonitorVariable(checkinLineList, 4,  checkinLine5);
    SetMonitorVariable(checkinLineList, 5,  checkinLine6);
    SetMonitorVariable(checkinLineList, 6,  checkinLine7);
    SetMonitorVariable(checkinLineList, 7,  checkinLine8);
    SetMonitorVariable(checkinLineList, 8,  checkinLine9);
    SetMonitorVariable(checkinLineList, 9,  checkinLine10);
    SetMonitorVariable(checkinLineList, 10, checkinLine11);
    SetMonitorVariable(checkinLineList, 11, checkinLine12);
    checkinLineLockList = CreateMonitorVariable("checkinLineLockList", 19, 3);
    checkinLineCVList = CreateMonitorVariable("checkinLineCVList", 17, 12);
    checkinCVList = CreateMonitorVariable("checkinCVList", 13, 12);
    checkinLockList = CreateMonitorVariable("checkinLockList", 15, 12);
    checkinBreakCVList = CreateMonitorVariable("checkinBreakCVList", 18, 12);
    checkinManagerLock = CreateLock("checkinManagerLock", 18);
    checkinManagerCV = CreateCondition("checkinManagerCV", 16);
    checkinStateList = CreateMonitorVariable("checkinStateList", 16, 12);
    finalCheckinList = CreateMonitorVariable("finalCheckinList", 16, 12);
    requestingCheckinDataList = CreateMonitorVariable("requestingCheckinDataList", 25, 12);
    for (i = 0; i < 12; i++)
    {
        if (i%4 == 0) /* 0, 4, 8 */
        {
            SetMonitorVariable(checkinLineLockList, i/4, CreateLock("checkinLineLock", 15));
            SetMonitorVariable(checkinStateList, i, CI_NONE);
        }
        SetMonitorVariable(checkinLineCVList, i, CreateCondition("checkinLineCV", 13));
        SetMonitorVariable(checkinCVList, i, CreateCondition("checkinCV", 9));
        SetMonitorVariable(checkinLockList, i, CreateLock("checkinLock", 11));
        SetMonitorVariable(checkinBreakCVList, i, CreateCondition("checkinBreakCV", 14));
    }
    
    /* Cargo variables */
    
    cargoList = CreateMonitorVariable("cargoList", 9, 5);
    cargoListLock = CreateLock("cargoListLock", 13);
    cargoCVList = CreateMonitorVariable("cargoCVList", 11, 5);
    cargoDataCVList = CreateMonitorVariable("cargoDataCVList", 15, 5);
    cargoDataLockList = CreateMonitorVariable("cargoDataLockList", 17, 5);
    cargoManagerLock = CreateLock("cargoManagerLock", 16);
    cargoManagerCVList = CreateMonitorVariable("cargoManagerCVList", 18, 5);
    cargoLockList = CreateMonitorVariable("cargoLockList", 13, 5);
    cargoStateList = CreateMonitorVariable("cargoStateList", 14, 5);
    requestingCargoDataList = CreateMonitorVariable("requestingCargoDataList", 23, 5);
    for (i = 0; i < 5; i++)
    {
        SetMonitorVariable(cargoDataCVList, i, CreateCondition("cargoDataCV", 11));
        SetMonitorVariable(cargoDataLockList, i, CreateLock("cargoDataLock", 13));
        SetMonitorVariable(cargoManagerCVList, i, CreateCondition("cargoManagerCV", 14));
        SetMonitorVariable(cargoCVList, i, CreateCondition("cargoCV", 7));
        SetMonitorVariable(cargoLockList, i, CreateLock("cargoLock", 9));
    }
}

void ManagerPrint(){
	
	int a;
    int totalLiaisonPassengers  = 0;
    int totalCheckinPassengers  = 0;
    int totalSecurityPassengers = 0;
	Printf("\n",1,0,0);
	for (a = 0; a < 3; a++)
    {
        totalLiaisonPassengers  += manager.liaisonPassengerCount[a];
        totalCheckinPassengers  += manager.checkinPassengerCount[a];
        totalSecurityPassengers += manager.securityInspectorPassengerCount[a];
    }

    Printf("Passenger count reported by airport liaison = %d\n", 49, 1, totalLiaisonPassengers);
    Printf("Passenger count reported by airline check-in staff = %d\n", 56, 1, totalCheckinPassengers);
    Printf("Passenger count reported by security inspector = %d\n", 52, 1, totalSecurityPassengers);

    for(a = 0; a < 3; a++)
    {
        Printf("From setup: Baggage count of airline %d = %d\n", 45, 2, a*100 + ((Airline*)GetMonitorVariable(airlineList, a))->totalBagCount);
        Printf("From airport liaison: Baggage count of airline %d = %d\n", 55, 2, a*100 + manager.liaisonBaggageCount[a]);
        Printf("From cargo handlers: Baggage count of airline %d = %d\n", 54, 2, a*100 + manager.cargoHandlersBaggageCount[a]);
        Printf("From setup: Baggage weight of airline %d = %d\n", 46, 2, a*100 + ((Airline*)GetMonitorVariable(airlineList, a))->totalBagWeight);
        Printf("From airline check-in staff: Baggage weight of airline %d = %d\n", 63, 2, a*100 + manager.checkinBaggageWeight[a]);
        Printf("From cargo handlers: Baggage weight of airline %d = %d\n", 55, 2, a*100 + manager.cargoHandlersBaggageWeight[a]);    	
    }

	Printf("\n",1,0,0);
}

void LiaisonDataRequest()
{
	int i, j, k;
    int liaisonCV, liaisonLock;
    Liaison* l;
		/*Gather data from liaisons*/
	for (i = 0; i < 3; i++)
    {	
		manager.liaisonPassengerCount[i] = 0;
		manager.liaisonBaggageCount[i] = 0;
	}

	for(j = 0; j < 5; j++)
    {
		if(GetMonitorVariable(liaisonStateList, j) == L_FREE)
        {
            liaisonCV = GetMonitorVariable(liaisonCVList, j);
            liaisonLock = GetMonitorVariable(liaisonLockList, j);
            l = (Liaison*)GetMonitorVariable(liaisonList, j);
            
			Acquire(liaisonManagerLock);
			
            SetMonitorVariable(requestingLiaisonDataList, j, true);
			
            Signal(liaisonCV, liaisonLock);

			Wait(liaisonManagerCV, liaisonManagerLock);
			/*Waits for the signal of corresponding Liaison*/
            
			Acquire(liaisonLock);
            
            Release(liaisonManagerLock);
			
			for (k = 0; k < 3; k++)
            {
				manager.liaisonPassengerCount[k] += l->passengers[k];
				manager.liaisonBaggageCount[k] += l->luggage[k];
			}

			Signal(liaisonCV, liaisonLock);

			Release(liaisonLock);
		}
	}
}

void CheckinDataRequest()
{
    int t,u,v,w;
    int checkinBreakCV, checkinLock, checkinCV;
    int newCheckinBaggageWeight[3];
	int newCheckinPassengerCount[3];
    Checkin* ci;
    
	for (t = 0; t < 3; t++)
    {
		newCheckinPassengerCount[t] = 0;
		newCheckinBaggageWeight[t] = 0;
	}

	for (u = 0; u < 12; u++)
    {
		if (u%4 != 0) /* not 0, 4, or 8 */
        {
            if (! GetMonitorVariable(finalCheckinList, u))
            {
                checkinBreakCV = GetMonitorVariable(checkinBreakCVList, u);
                checkinLock = GetMonitorVariable(checkinLockList, u);
                checkinCV = GetMonitorVariable(checkinCVList, u);
                ci = (Checkin*)GetMonitorVariable(checkinList, u);
                
                Acquire(checkinManagerLock);
                
                SetMonitorVariable(requestingCheckinDataList, u, true);
                
                Signal(checkinBreakCV, checkinLock);
            
                Wait(checkinManagerCV, checkinManagerLock);
                
                Acquire(checkinLock);
                
                Release(checkinManagerLock);
                
                newCheckinPassengerCount[ci->airline] += ci->passengers;
                newCheckinBaggageWeight[ci->airline]  += ci->weight;
                
                Signal(checkinCV, checkinLock);
                
                Release(checkinLock);
            }
        }
	}
	for (w = 0; w < 3; w++)
    {
		if (newCheckinPassengerCount[w] > manager.checkinPassengerCount[w])
            manager.checkinPassengerCount[w] = newCheckinPassengerCount[w];
		if (newCheckinBaggageWeight[w]  > manager.checkinBaggageWeight[w])
            manager.checkinBaggageWeight[w]  = newCheckinBaggageWeight[w];
	}
}

void CargoDataRequest()
{
    int i, j, k;
    int cargoDataCV, cargoDataLock, cargoLock, cargoCV, cargoManagerCV;
    Cargo* cargo;
    
	for (i = 0; i < 3; i++)
    {
		manager.cargoHandlersBaggageCount[i] = 0;
		manager.cargoHandlersBaggageWeight[i] = 0;
	}

	for (j = 0; j < 6; j++)
    {
        cargoDataCV = GetMonitorVariable(cargoDataCVList, j);
        cargoDataLock = GetMonitorVariable(cargoDataLockList, j);
        cargoLock = GetMonitorVariable(cargoLockList, j);
        cargoCV = GetMonitorVariable(cargoCVList, j);
        cargoManagerCV = GetMonitorVariable(cargoManagerCVList, j);
        cargo = (Cargo*)GetMonitorVariable(cargoList, j);
        
		Acquire(cargoManagerLock);
        
		SetMonitorVariable(requestingCargoDataList, j, true);
        
		Signal(cargoDataCV, cargoLock);
		
		Wait(cargoManagerCV, cargoManagerLock);

		Acquire(cargoDataLock);
        
        Release(cargoManagerLock);
        
		for (k = 0; k < 3; k++)
        {
			manager.cargoHandlersBaggageWeight[k] += cargo->weight[k];
			manager.cargoHandlersBaggageCount[k] += cargo->luggage[k];
		}

		Signal(cargoCV, cargoDataLock);
        
		Release(cargoDataLock);
	}
}

/* Finds the number of elements in an array */
int findArrayElementCount(int array)
{
	int elementCount = 0;
    
	while(GetMonitorVariable(array, elementCount) != 0)
        elementCount++;
    
	return elementCount;
}

void RunManager()
{
	int i, j, k, l, m, counter;
    int newCheckinBaggageWeight[3];
	int newCheckinPassengerCount[3];
    int airlineLock, boardingLock, boardingCV;
    Airline* al;

	for(i =0; i < 3; i++)
    {
		manager.liaisonBaggageCount[i] = 0;
		manager.cargoHandlersBaggageWeight[i] = 0;
		manager.checkinBaggageWeight[i] = 0;
		manager.cargoHandlersBaggageCount[i] = 0;
    	manager.liaisonPassengerCount[i] = 0;	
    	manager.checkinPassengerCount[i] = 0;
    	manager.securityInspectorPassengerCount[i] = 0;
	}

	while(true)
    {
		Acquire(conveyorLock);

		if(findArrayElementCount(conveyor) > 0)
        {
			counter = 0;

			for ( j = 0; j < 6; ++j)
            {
				if(GetMonitorVariable(cargoStateList, j) == C_BREAK)
					counter++;
			}

			if(counter == 6)
            {
				Printf("Airport manager calls back all the cargo handlers from break\n",61,0,0);
				for(k = 0; k < 6; k++)
                {
					Acquire(GetMonitorVariable(cargoLockList, k));
					Signal(GetMonitorVariable(cargoDataCVList, k), GetMonitorVariable(cargoLockList, k));
				}

				for(k = 0; k < 6; k++)
					Release(GetMonitorVariable(cargoLockList, k));
			}
		}

		Release(conveyorLock);

		LiaisonDataRequest();
		CheckinDataRequest();
		CargoDataRequest();

		for(m = 0; m < 3; m++)
        {
            airlineLock = GetMonitorVariable(airlineLockList, m);
            boardingLock = GetMonitorVariable(boardingLockList, m);
            boardingCV = GetMonitorVariable(boardingCVList, m);
            
			if(! clearAirline[m])
            {
				Acquire(airlineLock);
                
                al = (Airline*)GetMonitorVariable(airlineList, m);
                
				if(manager.checkinPassengerCount[m] >= al->ticketsIssued && manager.cargoHandlersBaggageCount[m] >= al->totalBagCount) /* no security inspector */
                {
					Printf("Airport manager gives a boarding call to airline %d\n", 53, 1, m);
                    
					Acquire(boardingLock);
                    
					Broadcast(boardingCV, boardingLock);
                    
					clearAirline[m] = true;
					clearAirlineCount++;
                    
					Release(boardingLock);
				}
                
				Release(airlineLock);
			}	
		}

		if (clearAirlineCount == 3)
        {
            ManagerPrint();
			Exit(0);
		}

		for(l = 0; l < 5; l++)
			Yield();
	}
}

int main()
{
    CreateVariables();
    RunManager();
    Exit(0); /* should never reach this line */
}
