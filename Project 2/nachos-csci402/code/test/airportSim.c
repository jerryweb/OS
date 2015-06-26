#include "syscall.h"
#include "airportStructs.h"
#define NULL (void*)0

void createGlobalVariables(){
	int i;
	int id;
	int numAirlines;
	struct Passenger* Passenger;
	int* passengersPerAirline[20];  
	Liaison* liaisonArray[5];
}

void Passenger0(){
	Passenger->id = 0;
	/*airline = 0;*/
	Passenger->Ticket.airline =0;


	for(i = 0; i <3; i++){
		Luggage* bag_1;
		bag_1.airlineCode = 0;
		bag_1.weight = 30;
		Passenger->bags[i] =  bag_1;
	}

	Printf("Forking passenger\n", 18, 0,0);
	Exit(0);
}

int i;
int numAirlines; 
struct Passenger* passengerArray[20];
struct Liaison* liaisonArray[5];
struct Passenger* liaisonQueues[5][20];
int liaisonManagerLock;
int liaisonManagerCV;
int liaisonLineLock;
int liaisonLineCV[5];
int liaisonLock[5];
int liaisonCV[5];
LiaisonState liaisonState[5];

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

    createGlobalVariables();

	Fork(Passenger0, "Passenger 0", 11);
}