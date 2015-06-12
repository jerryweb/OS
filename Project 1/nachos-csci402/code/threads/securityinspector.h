#ifndef SECURITYINSPECTOR_H
#define SECURITYINSPECTOR_H

#include "copyright.h"
#include "thread.h"
#include "list.h"
#include "airport.h"

//avoid circular dependency
class Passenger;

class SecurityInspector {
public:
	SecurityInspector(int ID, Airport* AIRPORT);
	~SecurityInspector();
	void Inspect();
	int getID();

private:
	int id;
	int qPassengerCount;  //initialize to 0
							//++ when sent passenger to questionsing
							//-- when returned
	Airport* airport;
	//int* passengerCount;          //array storing cleared passenger number for each airline
};

#endif
