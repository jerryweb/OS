#ifndef SECURITYINSPECTOR_H
#define SECURITYINSPECTOR_H

#include "copyright.h"
#include "thread.h"
#include "list.h"
#include "airport.h"
//#include "passenger.h"
class Passenger;

class SecurityInspector {
public:
	SecurityInspector(int ID, Airport* AIRPORT);
	~SecurityInspector();
	void Inspect();
	void setReturn();    //can only be called by returning passenger
	void setReturnPassenger(Passenger* p);

	int getID();

private:
	int id;
	Airport* airport;
	bool hasReturned;   //initialize to false
						//true means there is passenger has just returned
						//can only be modified by the returning passenger
	Passenger* returnPassenger;
};

#endif
