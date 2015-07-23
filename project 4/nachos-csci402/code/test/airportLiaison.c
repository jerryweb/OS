/* Networked airport simulation.
 *  Initializes a single liaison.
 */

#include "syscall.h"
#include "airportStructs.h"

#define NULL (void*)0

/* Personal variables */

int id;
Liaison l;

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
int checkinLineLockList, /* mv size 3, int (lock) */
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
            SetMonitorVariable(airlineList, i, &al);
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
            SetMonitorVariable(checkinLineLock, i/4, CreateLock("checkinLineLock", 15));
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

/* Removes the first element from an array of 21 ints and moves all other elements down */
void RemoveFromQueue(int array)
{
    for (i = 1; i < 21; i++)
    {
        SetMonitorVariable(array, i-1, GetMonitorVariable(array, i));
    }
    SetMonitorVariable(array, 20, 0);
}

void RunLiaison()
{
    int i, elementCount;
    int liaisonLine, liaisonLineCV, liaisonCV, liaisonLock;
    Passenger* p;
    
    liaisonLine = GetMonitorVariable(liaisonLineList, id);
    liaisonLineCV = GetMonitorVariable(liaisonLineCVList, id);
    liaisonCV = GetMonitorVariable(liaisonCVList, id);
    liaisonLock = GetMonitorVariable(liaisonLockList, id);
    
    while(true)
    {
        Acquire(liaisonLineLock);
        
        p = (Passenger*)GetMonitorVariable(liaisonLine, 0);
        
        if (p != NULL)
        {
            RemoveFromQueue(liaisonLine);
            
			Signal(liaisonLineCV, liaisonLineLock);
            
            SetMonitorVariable(liaisonStateList, id, L_BUSY);
        }
        else
        {
            SetMonitorVariable(liaisonStateList, id, L_FREE);
        }
        
        Acquire(liaisonLock);
        
        Release(liaisonLineLock);
        
        Wait(liaisonCV, liaisonLock);
        
        if (p != NULL)
        {
            p->airline = p->ticket->airline;
            
            Printf("Airport Liaison %d directed passenger %d of airline %d\n", 55, 3, id*100*100 + p->id*100 + p->airline);
            
            l.passengers[p->airline]++;
            for (i = 0; i < 3; i++)
            {
                l.luggage[p->airline]++;
                l.weight[p->airline] += p->bags[i]->weight;
            }

            Signal(liaisonCV, liaisonLock);

            p = NULL;
        }
        
        Release(liaisonLock);
        
        if (GetMonitorVariable(requestingLiaisonDataList, id))
        {
            Acquire(liaisonManagerLock);
            
            Acquire(liaisonLock);
            
            Signal(liaisonManagerCV, liaisonManagerLock);
            
            Release(liaisonManagerLock);
            
            Wait(liaisonCV, liaisonLock);
            /* Wait for manager to signal that all the data has been collected*/

            SetMonitorVariable(requestingLiaisonDataList, id, false);
            
            Release(liaisonLock);
        }
    }
}

int CreateLiaison()
{
    int i;
    
    for (i = 0; i < 3; i++)
    {
        l.passengers[i] = 0;
        l.luggage[i] = 0;
        l.weight[i] = 0;
    }
    
    Acquire(liaisonListLock);
    for (i = 0; i < 5; i++)
    {
        Liaison* liaison = (Liaison*) GetMonitorVariable(liaisonList, i);
        if (! liaison)
        {
            l.id = i;
            SetMonitorVariable(liaisonList, i, &l);
            break;
        }
    }
    Release(liaisonListLock);
    return i;
}

int main()
{
    CreateVariables();
    id = CreateLiaison();
    RunLiaison();
    Exit(0);
}
