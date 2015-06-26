#include "syscall.h"
#include "airportStructs.h"
#define NULL (void*)0

int i;
int numAirlines; 
/*liaison variables*/
typedef struct Passenger* passengerArray[20];
typedef struct Liaison* liaisonArray[5];
typedef struct Passenger* liaisonQueues[5][20];
int liaisonManagerLock;
int liaisonManagerCV;
int liaisonLineLock;
int liaisonLineCV[5];
int liaisonLock[5];
int liaisonCV[5];
LiaisonState liaisonState[5];

void createGlobalVariables(){
	int id;
	typedef struct Passenger* Passenger;
	int* passengersPerAirline[20];
	/*struct Luggage* passengerLuggage[20][3];*/
}

void Passenger0(){
	passengerArray[0]->id = 0;
	passengerArray[0]->ticket->airline = 0;
	passengerArray[0]->ticket->executive = false;

	for(i = 0; i < 3; i++){
		passengerArray[0]->bags[i]->airlineCode = 0;
		passengerArray[0]->bags[i]->weight = 30 + i;
	}

	passengerArray[0]->boardingPass->gate = NULL;
	passengerArray[0]->boardingPass->seatNum = NULL;

	Printf("Forking passenger\n", 18, 0,0);
	Exit(0);
}



void RunLiaison()
{
    int id; /* HOW DO WE KNOW WHAT THIS IS? */
    Passenger* p = NULL;
    while(true)
    {
        Acquire(liaisonLineLock);
        if (liaisonQueues[id][0] != NULL)
        {
            p = liaisonQueues[id][0];
            /* shift everyone else down one */
            Printf("Airport Liaison %d directed passenger %d of airline %d\n", 
                id, p->getID(), p->getTicket().airline);
        }
        else
        {
            p = NULL;
            liaisonState[id] = L_FREE;
        }
    }
}


int main()
{
    numAirlines = 3;
    passengerArray = {NULL};
    /*passenger variables*/
    createGlobalVariables();


    /*liaison variables*/
    liaisonArray = {NULL};
    liaisonQueues = {NULL};
    liaisonManagerLock = CreateLock("LiaisonManagerLock", 18);
    liaisonManagerCV = CreateLock("LiaisonManagerCV", 16);
    liaisonLineLock = CreateLock("LiaisonLineLock", 15);
    for (i = 0; i < 5; i++)
    {
        liaisonLineCV = CreateLock("LiaisonLineCV", 13);
        liaisonLock = CreateLock("LiaisonLock", 11);
        liaisonCV = CreateLock("LiaisonCV", 9);
    }
    liaisonState = {L_BUSY};


	Fork(Passenger0, "Passenger 0", 11);
}