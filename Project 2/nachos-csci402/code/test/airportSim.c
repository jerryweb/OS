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

/*passenger variables*/

/*liaison variables*/

int i, j, k;
int numAirlines;
int passengerCount;
int passengerArrayLock;
Passenger* passengerArray[20];
int liaisonCount;
int liaisonArrayLock;
Liaison* liaisonArray[5];
Passenger* liaisonLine[5][20];
int liaisonManagerLock;
int liaisonManagerCV;
int liaisonLineLock;
int liaisonLineCV[5];
int liaisonLock[5];
int liaisonCV[5];
LiaisonState liaisonState[5];
bool requestingLiaisonData[5];

void createGlobalVariables(){
	int id;
	typedef struct Passenger* Passenger;
	int* passengersPerAirline[20];
	/*struct Luggage* passengerLuggage[20][3];*/
}


void Passenger0(){
	Passenger p;
	p.id = 0;
	p.ticket->airline = 0;
	p.ticket->executive = false;
	p.myLine = 0;
	p.CISline = false;

	for(i = 0; i < 3; i++){
		p.bags[i]->airlineCode = 0;
		p.bags[i]->weight = 30 + i;
	}

	/*p.boardingPass.gate = NULL;
	p.boardingPass.seatNum = NULL;*/
	Printf("Passenger %d of airline %d created\n", 35, 2, );
	Printf("Forking passenger\n", 18, 0,0);
	Exit(0);
}


int findShortestLine(bool CISline){
	Passenger p;
	for(j = 0; j < 20; j++){
		if(liaisonLine[i][j] == NULL)
			break;
		else
			p.lineLength[i]++;
	}

	p.minValue = p.lineLength[i];

	if(!CISline){
		for (i = 0; i < 5; i++) {

			for(j = 0; j < 20; j++){
				if(liaisonLine[i][j] == NULL)
					break;
				else
					p.lineLength[i]++;
			}
			if(p.lineLength[i] < p.minValue){
				p.minValue = p.lineLength[i];
				p.location = i;
			}
		}
		return p.location;
	}
}

void RunPassenger(){

}

void RunLiaison()
{
    Liaison l;
    Acquire(liaisonArrayLock);
    l.id = liaisonCount;
    liaisonCount++;
    for (i = 0; i < 3; i++)
    {
        l.passengers[i] = 0;
        l.luggageCount[i] = 0;
        l.luggageWeight[i] = 0;
    }
    liaisonArray[l.id] = &l;
    Release(liaisonArrayLock);
    
    while(true)
    {
        Passenger* p;
        Acquire(liaisonLineLock);
        p = liaisonLine[l.id][0];
        if (p != NULL)
        {
            /* TODO: shift everyone else down one */
            liaisonState[l.id] = L_BUSY;
            p->airline = p->ticket->airline;
            Printf("Airport Liaison %d directed passenger %d of airline %d\n", 55, 3,
                    l.id*1000*1000 + p->id*1000 + p->airline);
        }
        else
        {
            liaisonState[l.id] = L_FREE;
        }
        Acquire(liaisonLock[l.id]);
        Release(liaisonLineLock);
        if (p != NULL)
        {
            Wait(liaisonCV[l.id], liaisonLock[l.id]);
            Acquire(liaisonLock[l.id]);
            l.passengers[p->airline]++;
            /* assumes 3 bags weighing 30 lbs each */
            l.luggageCount[p->airline] += 3;
            l.luggageWeight[p->airline] += 3*30;
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
            Signal(liaisonManagerCV, liaisonManagerLock);
            Release(liaisonManagerLock);
            Wait(liaisonCV[l.id], liaisonLock[l.id]);
            requestingLiaisonData[l.id] = false;
        }
    }
    Exit(0);
}


int main()
{
    numAirlines = 3;
    passengerCount = 0;
    passengerArrayLock = CreateLock("PassengerArrayLock", 18);
    for (i = 0; i < 20; i++)
    {
        passengerArray[i] = NULL;
    }
    liaisonCount = 0;
    liaisonArrayLock = CreateLock("LiaisonArrayLock", 16);
    liaisonManagerLock = CreateLock("LiaisonManagerLock", 18);
    liaisonManagerCV = CreateCondition("LiaisonManagerCV", 16);
    liaisonLineLock = CreateLock("LiaisonLineLock", 15);
    for (i = 0; i < 5; i++)
    {
        liaisonArray[i] = NULL;
        liaisonLineCV[i] = CreateCondition("LiaisonLineCV", 13);
        liaisonLock[i] = CreateLock("LiaisonLock", 11);
        liaisonCV[i] = CreateCondition("LiaisonCV", 9);
        liaisonState[i] = L_BUSY;
        requestingLiaisonData[i] = false;
        for (j = 0; j < 20; j++)
        {
            liaisonLine[i][j] = NULL;
        }
    }

    Fork(Passenger0, "Passenger 0", 11);
    for (i = 0; i < 5; i++)
    {
        Fork(RunLiaison, "Liaison", 7);
    }
}
