// This is the main file where all of the components to the airport can be tested

#include "passenger.h"
#include "airportobjects.h"



/*void CreatePassenger(int* liasionList){

	Luggage luggage[3];

	for(int i =0; i <3; i++){
		luggage[i].airlineCode = 1;
		luggage[i].weight = 45 + i;
	}

	Ticket ticket;
	ticket.airline = 1;
	ticket.executive = false;
	int passengerID = 0;

	Passenger *passenger = new Passenger(0, luggage, ticket);
}*/

void PassengerFindsShortestLiaisonLine(){
	//This is the initialization of a passenger with the following secifications
	Lock *LineLock = new Lock("line lock");
	Condition* lineCV[7];

	int liasionList[7] = {3, 2, 5, 8, 1, 6, 9};  			//there are 7 airport
															//liasions including 
															//executive
	Luggage luggage[3];										//3 bags 

	for(int i =0; i <3; i++){
		luggage[i].airlineCode = 1;
		luggage[i].weight = 45 + i;							 //weight ranges from 45 -47lbs
	}

	Ticket ticket;
	ticket.airline = 1;
	ticket.executive = false;
	int passengerID = 0;

	Passenger *passenger = new Passenger(0, luggage, ticket);

	//Beginning of shortest line test and start of critical section for finding shortest line
	LineLock->Aquire();
		
		int myLine, lineSize = 0; 				//These are the line and its size that the passenger enters
		// int lineSize =0// = LineCount[0];
		myLine = passenger->findShortestLiaisonLine(liasionList);		//function to find shortest line
		lineSize = liasionList[myLine];

		//Should be the first print statment 
		printf("Passenger %d chose liaison %d with a line length of %d\n", Passenger.getID(), myLine, lineSize);

		if(lineSize > 0)
			lineCV[myLine]->Wait(LineLock);
		

	LineLock->Release();
	//end of passenger crtitical section 

	//CreatePassenger(liasionList);

}