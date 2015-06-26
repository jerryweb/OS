/*This is the struct for all of the public airport data*/

typedef enum  {false, true} bool;
typedef enum  {L_FREE, L_BUSY, L_NONE} LiaisonState;
typedef enum  {CI_FREE, CI_BUSY, CI_BREAK, CI_CLOSED} CheckinState;
typedef enum  {C_BUSY, C_BREAK} CargoState;
typedef enum  {SO_BUSY, SO_FREE} ScreenState;
typedef enum  {SC_BUSY, SC_FREE} SecurityState;

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
	int lineLength[5]; 		/*used to help calculate shortest line*/
	int airline;
	int location;	/*used to return the shortest line location*/
	int minValue;
	int myLine;		/*the current line the passenger is in*/
	bool CISline;	/*if true, passenger will find shortest checkin staff line*/
	Ticket* ticket;
	Luggage* bags[3];
	BoardingPass boardingPass;
} Passenger;

typedef struct {
	int id;
	int passengers[3];
    int luggageCount[3];
    int luggageWeight[3];
} Liaison;
