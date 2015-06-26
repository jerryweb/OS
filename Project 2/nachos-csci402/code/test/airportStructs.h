/*This is the struct for all of the public airport data*/

typedef enum  {false, true} bool;
typedef enum  {L_FREE, L_BUSY, L_NONE} LiaisonState;
typedef enum  {CI_FREE, CI_BUSY, CI_BREAK, CI_CLOSED} CheckinState;
typedef enum  {C_BUSY, C_BREAK} CargoState;
typedef enum  {SO_BUSY, SO_FREE} ScreenState;
typedef enum  {SC_BUSY, SC_FREE} SecurityState;

/*This is the struct that holds all of the public data for the airport*/
typedef struct 
{
	int numAirlines; 
	struct Passenger* passengerArray[20];
    struct Liaison* liaisonArray[5];
    struct Passenger* liaisonQueues[5];
    int liaisonManagerLock;
    int liaisonManagerCV;
    int liaisonLineLock;
    int liaisonLineCV[5];
    int liaisonLock[5];
    int liaisonCV[5];
    LiaisonState liaisonState[5];
} Airport;

typedef struct           
{
	int airlineCode;     
	int weight;          
} Luggage;

typedef struct             
{
	int airline;         
	bool executive;                          
} Ticket;

typedef struct       
{
	int gate;            
	int seatNum;         
} BoardingPass;

typedef struct {
	int id;
	int airline;
	Airport airport;
	Ticket ticket;
	BoardingPass boardingPass;
} Passenger;

typedef struct {
	int id;
	Airport airport;
	int passengers[3];
    int luggageCount[3];
    int luggageWeight[3];
} Liaison;
