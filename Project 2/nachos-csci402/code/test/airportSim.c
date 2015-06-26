/* Airport simulation.
 *  3 cash-starved airlines
 *  20 clueless passengers: 3 bags each, all bags weigh 30 lbs
 *  5 hardworking liaisons
 *  3 lazy check-in staff per airline
 *  6 super lazy cargo handlers
 *  3 screening officers and security inspectors, always suspicious of everything
 *  and of course, 1 evil manager
 */

#include "syscall.h"
#include "airportStructs.h"

#define NULL (void*)0

int i;
int numAirlines;
int passengerCount;
int passengerArrayLock;
struct Passenger* passengerArray[20];
int liaisonCount;
int liaisonArrayLock;
struct Liaison* liaisonArray[5];
struct Passenger* liaisonLine[5][20];
int liaisonManagerLock;
int liaisonManagerCV;
int liaisonLineLock;
int liaisonLineCV[5];
int liaisonLock[5];
int liaisonCV[5];
LiaisonState liaisonState[5];
bool requestingLiaisonData[5];

void RunLiaison()
{
    struct Liaison l;
    Acquire(liaisonArrayLock);
    l.id = liaisonCount;
    liaisonCount++;
    l.passengers = {0};
    l.luggageCount = {0};
    l.luggageWeight = {0};
    liaisonArray[l.id] = &l;
    Release(liaisonArrayLock);
    
    Passenger* p = NULL;
    while(true)
    {
        Acquire(liaisonLineLock);
        p = liaisonLine[l.id][0];
        if (p != NULL)
        {
            /* TODO: shift everyone else down one */
            Printf("Airport Liaison %d directed passenger %d of airline %d\n", 55, 3,
                    l.id*1000*1000 + p.id*1000 + p.ticket.airline);
            liaisonState[l.id] = L_BUSY;
            p.airline = p.ticket.airline;
        }
        else
        {
            p = NULL;
            liaisonState[l.id] = L_FREE;
        }
        Acquire(liaisonLock[l.id]);
        Release(liaisonLineLock);
        if (p != NULL)
        {
            Wait(liaisonCV[l.id], liaisonLock[l.id]);
            Acquire(liaisonLock[l.id]);
            l.passengers[p.airline]++;
            /* assumes 3 bags weighing 30 lbs each */
            l.luggageCount[p.airline] += 3;
            l.luggageWeight[p.airline] += 3*30;
            Release(liaisonLock[l.id]);
        }
        else
        {
            Wait(liaisonCV[l.id], liaisonLock[l.id]);
        }
        if (requestingLiaisonData[l.id])
        {
            Acquire(liaisonManagerLock);
            Acquire(liaisonLock[l.id]);
            Signal(liaisonmanagerCV, liaisonManagerLock);
            Release(liaisonManagerLock);
            Wait(liaisonCV[l.id], liaisonLock[l.id]);
            requestingLiaisonDaga[l.id] = false;
        }
    }
}

int main()
{
    numAirlines = 3;
    passengerCount = 0;
    passengerArrayLock = CreateLock("PassengerArrayLock", 18);
    passengerArray = {NULL};
    liaisonCount = 0;
    liaisonArrayLock = CreateLock("LiaisonArrayLock", 16);
    liaisonArray = {NULL};
    liaisonQueues = {NULL};
    liaisonManagerLock = CreateLock("LiaisonManagerLock", 18);
    liaisonManagerCV = CreateCondition("LiaisonManagerCV", 16);
    liaisonLineLock = CreateLock("LiaisonLineLock", 15);
    for (i = 0; i < 5; i++)
    {
        liaisonLineCV = CreateCondition("LiaisonLineCV", 13);
        liaisonLock = CreateLock("LiaisonLock", 11);
        liaisonCV = CreateCondition("LiaisonCV", 9);
    }
    liaisonState = {L_BUSY};
    requestingLiaisonData = {false};
}
