#include "syscall.h"


struct airport
{
	int numAirlines; 
	Passenger* passengerArray[20];
};

struct Luggage          
{
	int airlineCode;     
	int weight;          
};

struct Ticket            
{
	int airline;         
	bool executive;                          
};

struct BoardingPass      
{
	int gate;            
	int seatNum;         
};

enum LiaisonState {L_FREE, L_BUSY, L_NONE};
enum CheckinState {CI_FREE, CI_BUSY, CI_BREAK, CI_CLOSED, CI_NONE};
enum CargoState   {C_BUSY, C_BREAK, C_NONE};
enum AirlineState {BOARDING, BOARDED};
enum ScreenState  {SO_BUSY, SO_FREE};
enum SecurityState  {SC_BUSY, SC_FREE};


struct passenger{
	int id;
	int airline;
	Ticket ticket;
	Airport airport;
	BoardingPass boardingPass;
};


int main(){


	/*Fork(forkPassenger, "Passenger 0", 11);*/

}