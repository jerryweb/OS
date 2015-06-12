//This is the passenger class. It holds all of the information and functions for the passenger class


#ifndef PASSENGER_H
#define PASSENGER_H

#include "copyright.h"
#include "system.h"
#include "thread.h"
#include "list.h"
#include "airport.h"

//To Avoid circular denpendcy
class SecurityInspector;

class Passenger {
	public:
		//For full run
		Passenger(int ID,int qIndex,int airlineCode,Ticket T,List* bags,Airport* A,SecurityInspector** INSPECTORLIST);
		Passenger(int ID, List* bags, Ticket T, Airport* A);        // For full simulation.
		Passenger(int ID, Ticket t, int airlineCode, Airport* a);   // For PassengerFindsCorrectCISLine (test).
		Passenger(int ID, Ticket t);                                // For LiaisonTest.
        Passenger(int ID, Airport* a);                              // For PassengerFindsShortestLiaisonLine (test).
        Passenger(int ID);                                          // For CheckInTest.
        Passenger();                                                // For PassengerFindsCorrectCISLine (test).
		Passenger(int ID,int qIndex,Airport* AIRPORT);                                      //For Screening Test
		Passenger(int ID,int qIndex,Airport* AIRPORT,int airlineCode,SecurityInspector** INSPECTORLIST);    //For Security Test
		Passenger(Airport* AIRPORT);                                                        //For Screening Test(produce dummy passenger to fill arrays)
        ~Passenger();

		/*this function iterates through a given array and returns the location
		of the smallest value and the value itself*/
		int findShortestLine(List** list, bool CISline);
		/*This is the first version of the function for the passenger to find
		the shortest liaison line in the Airport.*/
		void findShortestLiaisonLine();
        void CheckIn(); // Drop off luggage and get boarding pass.


		//these are the get fucntion for the passenger's ticket
		Ticket getTicket() { return ticket; }

		List* getLuggage() { return luggageList; }
		//returns the id of the passenger
		int getID(){ return id; }

		//Luggage getLuggage(){ return *luggageList; }
				//void SetScreenPass(bool pnp);

		/*******For screening and Security**************/
		void Screening();
		void Inspecting();
		void SetQueueIndex(int qIndex);
		void SetSecurityPass(bool pnp);
		bool GetSecurityPass();
		/*****************************************/

		BoardingPass GetBoardingPass() { return boardingPass; }
		void SetBoardingPass(BoardingPass BP) { boardingPass = BP; }

        int GetAirline() { return airline; }
        void SetAirline(int airline_) { airline = airline_; }

	private:

		int id;							//This is the ID of the passenger
        int airline;                    // Code of the airline. Set by liaison.
		List* luggageList;				//This is the array of bags the passenger has
		Ticket ticket;  				//This is the ticket of the passenger
		Airport* airport;   // "Airport" construct, containing all
                            //  public data.
		BoardingPass boardingPass;

		/*******For screening and Security**************/
		bool securityPass;

		int queueIndex;        //This is the index of which queue the passenger is now in, it works for all the queues
							   //Please enforce updating the queueIndex everytime assigning a new queue for the passenger
		SecurityInspector** inspectorList;
		/*******************************************/
};

#endif
