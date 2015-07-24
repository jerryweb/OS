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
    conveyorSize = CreateMonitorVariable("conveyor", 8, 1);
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
    }
    SetMonitorVariable(airlineLockList, 0, CreateLock("airlineLock1", 12));
    SetMonitorVariable(airlineLockList, 1, CreateLock("airlineLock2", 12));
    SetMonitorVariable(airlineLockList, 2, CreateLock("airlineLock3", 12));
    SetMonitorVariable(boardingCVList, 0, CreateCondition("boardingCV1", 11));
    SetMonitorVariable(boardingCVList, 1, CreateCondition("boardingCV2", 11));
    SetMonitorVariable(boardingCVList, 2, CreateCondition("boardingCV3", 11));
    SetMonitorVariable(boardingLockList, 0, CreateLock("boardingLock1", 13));
    SetMonitorVariable(boardingLockList, 1, CreateLock("boardingLock2", 13));
    SetMonitorVariable(boardingLockList, 2, CreateLock("boardingLock3", 13));
    
    /* Passenger variables */
    
    passengerListLock = CreateLock("passengerListLock", 17);
    passengerList = CreateMonitorVariable("passengerList", 13, 21);

    /* Liaison variables */
    
    liaisonList = CreateMonitorVariable("liaisonList", 11, 5);
    liaisonListLock = CreateLock("liaisonListLock", 15);
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
    liaisonLineCVList = CreateMonitorVariable("requestingLiaisonDataList", 25, 5);
    liaisonLockList = CreateMonitorVariable("liaisonLockList", 15, 5);
    liaisonCVList = CreateMonitorVariable("liaisonCVList", 13, 5);
    liaisonStateList = CreateMonitorVariable("liaisonStateList", 16, 5);
    requestingLiaisonDataList = CreateMonitorVariable("requestingLiaisonDataList", 25, 5);
    SetMonitorVariable(liaisonLockList, 0, CreateLock("liaisonLock1", 12));
    SetMonitorVariable(liaisonLockList, 1, CreateLock("liaisonLock2", 12));
    SetMonitorVariable(liaisonLockList, 2, CreateLock("liaisonLock3", 12));
    SetMonitorVariable(liaisonLockList, 3, CreateLock("liaisonLock4", 12));
    SetMonitorVariable(liaisonLockList, 4, CreateLock("liaisonLock5", 12));
    SetMonitorVariable(liaisonCVList, 0, CreateCondition("liaisonCV1", 10));
    SetMonitorVariable(liaisonCVList, 1, CreateCondition("liaisonCV2", 10));
    SetMonitorVariable(liaisonCVList, 2, CreateCondition("liaisonCV3", 10));
    SetMonitorVariable(liaisonCVList, 3, CreateCondition("liaisonCV4", 10));
    SetMonitorVariable(liaisonCVList, 4, CreateCondition("liaisonCV5", 10));
    SetMonitorVariable(liaisonLineCVList, 0, CreateCondition("liaisonLineCV1", 14));
    SetMonitorVariable(liaisonLineCVList, 1, CreateCondition("liaisonLineCV2", 14));
    SetMonitorVariable(liaisonLineCVList, 2, CreateCondition("liaisonLineCV3", 14));
    SetMonitorVariable(liaisonLineCVList, 3, CreateCondition("liaisonLineCV4", 14));
    SetMonitorVariable(liaisonLineCVList, 4, CreateCondition("liaisonLineCV5", 14));
    
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
    for (i = 0; i < 3; i++)
    {
        SetMonitorVariable(checkinStateList, i*4, CI_NONE);
    }
    SetMonitorVariable(checkinLineLockList, 0, CreateLock("checkinLineLock1", 16));
    SetMonitorVariable(checkinLineLockList, 1, CreateLock("checkinLineLock2", 16));
    SetMonitorVariable(checkinLineLockList, 2, CreateLock("checkinLineLock3", 16));
    SetMonitorVariable(checkinLockList, 0, CreateLock("checkinLock1", 12));
    SetMonitorVariable(checkinLockList, 1, CreateLock("checkinLock2", 12));
    SetMonitorVariable(checkinLockList, 2, CreateLock("checkinLock3", 12));
    SetMonitorVariable(checkinLockList, 3, CreateLock("checkinLock4", 12));
    SetMonitorVariable(checkinLockList, 4, CreateLock("checkinLock5", 12));
    SetMonitorVariable(checkinLockList, 5, CreateLock("checkinLock6", 12));
    SetMonitorVariable(checkinLockList, 6, CreateLock("checkinLock7", 12));
    SetMonitorVariable(checkinLockList, 7, CreateLock("checkinLock8", 12));
    SetMonitorVariable(checkinLockList, 8, CreateLock("checkinLock9", 12));
    SetMonitorVariable(checkinLockList, 9, CreateLock("checkinLock10", 13));
    SetMonitorVariable(checkinLockList, 10, CreateLock("checkinLock11", 13));
    SetMonitorVariable(checkinLockList, 11, CreateLock("checkinLock12", 13));
    SetMonitorVariable(checkinCVList, 0, CreateCondition("checkinCV1", 10));
    SetMonitorVariable(checkinCVList, 1, CreateCondition("checkinCV2", 10));
    SetMonitorVariable(checkinCVList, 2, CreateCondition("checkinCV3", 10));
    SetMonitorVariable(checkinCVList, 3, CreateCondition("checkinCV4", 10));
    SetMonitorVariable(checkinCVList, 4, CreateCondition("checkinCV5", 10));
    SetMonitorVariable(checkinCVList, 5, CreateCondition("checkinCV6", 10));
    SetMonitorVariable(checkinCVList, 6, CreateCondition("checkinCV7", 10));
    SetMonitorVariable(checkinCVList, 7, CreateCondition("checkinCV8", 10));
    SetMonitorVariable(checkinCVList, 8, CreateCondition("checkinCV9", 10));
    SetMonitorVariable(checkinCVList, 9, CreateCondition("checkinCV10", 11));
    SetMonitorVariable(checkinCVList, 10, CreateCondition("checkinCV11", 11));
    SetMonitorVariable(checkinCVList, 11, CreateCondition("checkinCV12", 11));
    SetMonitorVariable(checkinLineCVList, 0, CreateCondition("checkinLineCV1", 14));
    SetMonitorVariable(checkinLineCVList, 1, CreateCondition("checkinLineCV2", 14));
    SetMonitorVariable(checkinLineCVList, 2, CreateCondition("checkinLineCV3", 14));
    SetMonitorVariable(checkinLineCVList, 3, CreateCondition("checkinLineCV4", 14));
    SetMonitorVariable(checkinLineCVList, 4, CreateCondition("checkinLineCV5", 14));
    SetMonitorVariable(checkinLineCVList, 5, CreateCondition("checkinLineCV6", 14));
    SetMonitorVariable(checkinLineCVList, 6, CreateCondition("checkinLineCV7", 14));
    SetMonitorVariable(checkinLineCVList, 7, CreateCondition("checkinLineCV8", 14));
    SetMonitorVariable(checkinLineCVList, 8, CreateCondition("checkinLineCV9", 14));
    SetMonitorVariable(checkinLineCVList, 9, CreateCondition("checkinLineCV10", 15));
    SetMonitorVariable(checkinLineCVList, 10, CreateCondition("checkinLineCV11", 15));
    SetMonitorVariable(checkinLineCVList, 11, CreateCondition("checkinLineCV12", 15));
    SetMonitorVariable(checkinBreakCVList, 0, CreateCondition("checkinBreakCV1", 15));
    SetMonitorVariable(checkinBreakCVList, 1, CreateCondition("checkinBreakCV2", 15));
    SetMonitorVariable(checkinBreakCVList, 2, CreateCondition("checkinBreakCV3", 15));
    SetMonitorVariable(checkinBreakCVList, 3, CreateCondition("checkinBreakCV4", 15));
    SetMonitorVariable(checkinBreakCVList, 4, CreateCondition("checkinBreakCV5", 15));
    SetMonitorVariable(checkinBreakCVList, 5, CreateCondition("checkinBreakCV6", 15));
    SetMonitorVariable(checkinBreakCVList, 6, CreateCondition("checkinBreakCV7", 15));
    SetMonitorVariable(checkinBreakCVList, 7, CreateCondition("checkinBreakCV8", 15));
    SetMonitorVariable(checkinBreakCVList, 8, CreateCondition("checkinBreakCV9", 15));
    SetMonitorVariable(checkinBreakCVList, 9, CreateCondition("checkinBreakCV10", 16));
    SetMonitorVariable(checkinBreakCVList, 10, CreateCondition("checkinBreakCV11", 16));
    SetMonitorVariable(checkinBreakCVList, 11, CreateCondition("checkinBreakCV12", 16));
    
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
    SetMonitorVariable(cargoLockList, 0, CreateLock("cargoLock1", 10));
    SetMonitorVariable(cargoLockList, 1, CreateLock("cargoLock2", 10));
    SetMonitorVariable(cargoLockList, 2, CreateLock("cargoLock3", 10));
    SetMonitorVariable(cargoLockList, 3, CreateLock("cargoLock4", 10));
    SetMonitorVariable(cargoLockList, 4, CreateLock("cargoLock5", 10));
    SetMonitorVariable(cargoDataLockList, 0, CreateLock("cargoDataLock1", 14));
    SetMonitorVariable(cargoDataLockList, 1, CreateLock("cargoDataLock2", 14));
    SetMonitorVariable(cargoDataLockList, 2, CreateLock("cargoDataLock3", 14));
    SetMonitorVariable(cargoDataLockList, 3, CreateLock("cargoDataLock4", 14));
    SetMonitorVariable(cargoDataLockList, 4, CreateLock("cargoDataLock5", 14));
    SetMonitorVariable(cargoCVList, 0, CreateCondition("cargoCV1", 8));
    SetMonitorVariable(cargoCVList, 1, CreateCondition("cargoCV2", 8));
    SetMonitorVariable(cargoCVList, 2, CreateCondition("cargoCV3", 8));
    SetMonitorVariable(cargoCVList, 3, CreateCondition("cargoCV4", 8));
    SetMonitorVariable(cargoCVList, 4, CreateCondition("cargoCV5", 8));
    SetMonitorVariable(cargoDataCVList, 0, CreateCondition("cargoDataCV1", 12));
    SetMonitorVariable(cargoDataCVList, 1, CreateCondition("cargoDataCV2", 12));
    SetMonitorVariable(cargoDataCVList, 2, CreateCondition("cargoDataCV3", 12));
    SetMonitorVariable(cargoDataCVList, 3, CreateCondition("cargoDataCV4", 12));
    SetMonitorVariable(cargoDataCVList, 4, CreateCondition("cargoDataCV5", 12));
    SetMonitorVariable(cargoManagerCVList, 0, CreateCondition("cargoManagerCV1", 15));
    SetMonitorVariable(cargoManagerCVList, 1, CreateCondition("cargoManagerCV2", 15));
    SetMonitorVariable(cargoManagerCVList, 2, CreateCondition("cargoManagerCV3", 15));
    SetMonitorVariable(cargoManagerCVList, 3, CreateCondition("cargoManagerCV4", 15));
    SetMonitorVariable(cargoManagerCVList, 4, CreateCondition("cargoManagerCV5", 15));
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

    clearAirlineCount = 0;
	for(i =0; i < 3; i++)
    {
		manager.liaisonBaggageCount[i] = 0;
		manager.cargoHandlersBaggageWeight[i] = 0;
		manager.checkinBaggageWeight[i] = 0;
		manager.cargoHandlersBaggageCount[i] = 0;
    	manager.liaisonPassengerCount[i] = 0;	
    	manager.checkinPassengerCount[i] = 0;
    	manager.securityInspectorPassengerCount[i] = 0;
        clearAirline[i] = false;
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
