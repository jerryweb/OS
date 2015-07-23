/* Networked airport simulation.
 *  Initializes a single passenger.
 */

#include "syscall.h"
#include "airportStructs.h"

#define NULL (void*)0

/* Personal variables */

int id;
Cargo c;

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

/* Manager variables */

int manager; /* mv size 1, Manager* */
int clearAirline; /* mv size 3, bool */
int counter; /* mv size 1, int */
int clearAirlineCount; /* mv size 1, int */

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

    /* Manager variables */
    
    manager = CreateMonitorVariable("manager", 7, 1);
    clearAirline = CreateMonitorVariable("clearAirline", 12, 3);
    counter = CreateMonitorVariable("counter", 7, 1);
    clearAirlineCount = CreateMonitorVariable("clearAirlineCount", 17, 1);
}

/* Removes the first element from an array of 21 passengers and moves all other elements down */
void RemoveFromQueue(int array)
{
    for (i = 1; i < 21; i++)
    {
        SetMonitorVariable(array, i-1, GetMonitorVariable(array, i));
    }
    SetMonitorVariable(array, 20, 0);
}

/* Finds the number of elements in an array of 21 passengers */
int findArrayElementCount(int array)
{
	int elementCount = 0;
    
	while((Passenger*)GetMonitorVariable(array, elementCount) != NULL)
        elementCount++;
    
	return elementCount;
}

void RunCheckin()
{
    int i, j, len;
    int checkinLine, execLine;
    int checkinLineLock, checkinLineCV, checkinCV, checkinLock, checkinBreakCV, airlineLock;
    Airline* al;
    Passenger* p, execP, econP;
    BoardingPass bp;
    bool exec;
    bool talked = false;
    execLine = 0;
    
    checkinLine = GetMonitorVariable(checkinLineList, id);
    execLine = GetMonitorVariable(checkinLineList, ci.airline*4);
    checkinLineLock = GetMonitorVariable(checkinLineLockList, ci.airline);
    checkinLineCV = GetMonitorVariable(checkinLineCVList, id);
    checkinCV = GetMonitorVariable(checkinCVList, id);
    checkinLock = GetMonitorVariable(checkinLockList, id);
    checkinBreakCV = GetMonitorVariable(checkinBreakCVList, id);
    airlineLock = GetMonitorVariable(airlineLockList, ci.airline);
    al = GetMonitorVariable(airlineList, ci.airline);
    
    while (true)
    {
        Acquire(checkinLineLock);
        
        if (GetMonitorVariable(checkinStateList, id) != CI_CLOSED)
        {
            execP = (Passenger*)GetMonitorVariable(execLine, 0);
            econP = (Passenger*)GetMonitorVariable(checkinLine, 0);
            
            if (! execP)
            {
                p = execP;
                
                RemoveFromQueue(execLine);
                
                len = findArrayElementCount(checkinLine);
                
                Printf("Airline check-in staff %d of airline %d serves an executive class passenger and economy class line length = %d\n", 111, 3, id*100*100 + ci.airline*100 + len);
                
                Acquire(checkinLock);
            }
            else if (! checkinLine)
            {
                p = econP;
                
                RemoveFromQueue(checkinLine);
                
                len = fineArrayElementCount(execLine);
                
                Printf("Airline check-in staff %d of airline %d serves an economy class passenger and executive class line length = %d\n", 111, 3, id*100*100 + ci.airline*100 + len);
                
                Acquire(checkinLock);
            }
            else /* if (! execLine && ! checkinLine) */
            {
                p = NULL;
                
                Acquire(checkinLock);
                
                SetMonitorVariable(checkinState, id, CI_BREAK);
                
                Release(checkinLineLock);
                
                Wait(checkinBreakCV, checkinLock);
                
                SetMonitorVariable(checkinState, id, CI_BUSY);
                
                Acquire(checkinLineLock);
            }
        }
        
        Release(checkinLineLock);
        
        if (p != NULL)
        {
            Acquire(airlineLock);
            
            ci.passengers++;
            
            exec = p->ticket->executive;
            
            bp.seatNum = al->seatsAssigned;
            al->seatsAssigned++;
            
            bp.gate = ci.airline;
            
            p->boardingPass = &bp;
            
            if (exec)
            {
                Printf("Airline check-in staff %d of airline %d informs executive class passenger %d to board at gate %d\n", 97, 4, id*100*100*100 + ci.airline*100*100 + p->id*100 + p->boardingPass->gate);
                
                Signal(checkinLineCV, checkinLineLock);
            }
            else
            {
                Printf("Airline check-in staff %d of airline %d informs economy class passenger %d to board at gate %d\n", 95, 4, id*100*100*100 + ci.airline*100*100 + p->id*100 + p->boardingPass->gate);
                
                Signal(checkinLineCV, checkinLineLock);
            }
            
            Release(airlineLock);
            
            Acquire(conveyorLock);
            for (i = 0; i < 3; i++)
            {
                Luggage* bag = p->bags[i];
                p->bags[i] = NULL;
                
                bag->airlineCode = ci.airline;
                
                bagIndex = GetMonitorVariable(conveyorSize, 0);
                SetMonitorVariable(conveyor, bagIndex, (int)bag);
                SetMonitorVariable(conveyorSize, 0, bagIndex + 1);
                
                ci.luggage++;
                ci.weight += bag->weight;
            }
            Printf("Airline check-in staff %d of airline %d dropped bags to the conveyor system\n", 76, 2, id*100 + ci.airline);
            
            p = NULL;
            
            Release(conveyorLock);
        }
        
        if (GetMonitorVariable(requestingCheckinDataList, id))
        {
            Acquire(checkinManagerLock);
            
            Acquire(checkinLock);
            
            Signal(checkinManagerCV, checkinManagerLock);
            
            Release(checkinManagerLock);
            
            Wait(checkinCV, checkinLock);
            
            SetMonitorVariable(requestingCheckinDataList, id, false);
            
            Release(checkinLock);
        }
        
        Acquire(airlineLock);
        
        if (al->seatsAssigned >= al->ticketsIssued)
        {
            Acquire(checkinLock);
            
            SetMonitorVariable(checkinStateList, id, CI_CLOSED);
            
            Release(airlineLock);
            
            if (GetMonitorVariable(finalCheckinList, id))
            {
                Exit(0);
            }
            else
            {
                Wait(checkinBreakCV, checkinLock);
            }
            
            Printf("Airline check-in staff %d is closing the counter\n", 49, 1, id);
            
            Acquire(airlineLock);
            
            SetMonitorVariable(finalCheckinList, id, true);
            
            Release(checkinLock);
        }
        Release(airlineLock);
    }
}

int CreateCheckin()
{
    ci.passengers = 0;
    ci.luggage = 0;
    ci.weight = 0;
    
    Acquire(checkinListLock);
    for (i = 0; i < 12; i++)
    {
        Checkin* checkin = (Checkin*) GetMonitorVariable(checkinList, i);
        if (! checkin && i%4 != 0)
        {
            ci.id = i;
            ci.airline = i/4;
            SetMonitorVariable(checkinList, i, &ci);
            break;
        }
    }
    Release(checkinListLock);
    return i;
}

int main()
{
    CreateVariables();
    id = CreateCheckin();
    RunCheckin();
    Exit(0); /* should never reach here, exiting inside RunCheckin instead */
}
