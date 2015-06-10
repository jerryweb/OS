#include "manager.h"

Manager::Manager(Airport* airport_)
{
    airport = airport_;
    airlineLuggage = new List[airport->numAirlines];
}

Manager::~Manager()
{
    
}

void Manager::Run()
{
    while(true){



    	for(int i = 0; i < 100; i++) 		//this makes the manager give up the CPU otherwise he would hog the CPU
			currentThread->Yield();
		
    }
}
