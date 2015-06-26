#include "syscall.h"
#include "airportStructs.h"

void createGlobalVariables(){
	int i;
	int id;
	int numAirlines;
	struct Passenger* Passenger;
	int* passengersPerAirline[20];  
	struct Passenger* passengerArray[20];
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

int main(){
	createGlobalVariables();

	Fork(Passenger0, "Passenger 0", 11);

}