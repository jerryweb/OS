/*This is the struct for all of the public airport data*/

typedef enum {false,   true   } bool;
typedef enum {L_BUSY,  L_FREE } LiaisonState;
typedef enum {CI_BUSY, CI_FREE, CI_BREAK, CI_CLOSED, CI_NONE} CheckinState;
typedef enum {C_BUSY,  C_BREAK} CargoState;
typedef enum {SO_BUSY, SO_FREE} ScreenState;
typedef enum {SC_BUSY, SC_FREE} SecurityState;
typedef enum {LIAISON, CHECKIN} LineType;

typedef struct
{
    int id;
    int seatsAssigned;
    int ticketsIssued;
    int totalBagCount;
    int totalBagWeight;
} Airline;

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
	Ticket* ticket;
	Luggage* bags[3];
	BoardingPass* boardingPass;
} Passenger;

typedef struct {
	int id;
	int passengers[3];
    int luggage[3];
    int weight[3];
} Liaison;

typedef struct {
	int id;
    int airline;
	int passengers;
    int luggage;
    int weight;
} Checkin;

typedef struct {
	int id;
    int luggage[3];
    int weight[3];
} Cargo;

typedef struct {
	/*Baggage Data*/
	int liaisonBaggageCount[3];
	int cargoHandlersBaggageCount[3];
	int cargoHandlersBaggageWeight[3];
	int checkinBaggageWeight[3];
	/*Passenger Data*/
    int liaisonPassengerCount[3];	
    int checkinPassengerCount[3];
    int securityInspectorPassengerCount[3];
} Manager;
