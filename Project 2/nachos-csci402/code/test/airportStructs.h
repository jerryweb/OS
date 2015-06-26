/*This is the struct for all of the public airport data*/

typedef enum  {true, false} bool;
typedef enum  {L_FREE, L_BUSY, L_NONE} LiaisonState;
typedef enum  {CI_FREE, CI_BUSY, CI_BREAK, CI_CLOSED, CI_NONE}CheckinState;
typedef enum  {C_BUSY, C_BREAK, C_NONE}CargoState;
typedef enum  {BOARDING, BOARDED}AirlineState;
typedef enum  {SO_BUSY, SO_FREE}ScreenState;
typedef enum  {SC_BUSY, SC_FREE}SecurityState;

struct Airport;

typedef struct           
{
	int airlineCode;     
	int weight;          
}Luggage;

typedef struct             
{
	int airline;         
	bool executive;                          
}Ticket;

typedef struct       
{
	int gate;            
	int seatNum;         
}BoardingPass;

typedef struct {
	int id;
	int airline;
	Ticket ticket;
	Luggage* bags[3];
	/*struct Airport airport;*/
	BoardingPass boardingPass;
}Passenger;

typedef struct {
	int id;
	/*struct Airport airport;*/
	int* passengers[3];
	int* luggageCount[3];
	int* totalLuggageWeight[3];
	LiaisonState state;
}Liaison;

/*This is the struct that holds all of the public data for the airport
typedef struct 
{
	int numAirlines; 
	Passenger* passengerArray[20];
	Liaison* liaisonArray[7];
	
}Airport;*/