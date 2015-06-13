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
	Passenger* CheckForPassengers();
	void Inspect();
	void AppendBool(bool* b);
	int getID();

private:
	int id;
	bool isBoarding;
	int* clearCount;   //Array for clear passenger count
	Airport* airport;
	List* reportList;   //array of bool
	List* boardingPassList;
};

#endif
