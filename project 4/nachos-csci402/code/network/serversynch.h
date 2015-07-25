// synchorinzation objects used for server

#ifndef SERVERSYNCH_H
#define SERVERSYNCH_H

#include "list.h"
#include "table.h"
#include "network.h"
#include "system.h"
#include "post.h"
//#include <string>
//#include <sstream>

//using namespace std;

enum lockState {
	SL_FREE, SL_BUSY
};

bool tableItemExist(char* tName, Table* table, int tableType);
int getTableIndex(char* tName, Table* table, int tableType);
void ServerReply(char* sMsg, int outMachine, int outMailbox, int fromMailbox);
unsigned int getTimeStamp();

class serverLock {
public:
	serverLock(char* dName, int owner, int mailbox);
	~serverLock();

	void Acquire(int outAddr, int outBox,int fromBox);
	void Release(int outAddr, int outBox,int fromBox);

	char* name;
	int index;   //in lockTable
	lockState state;
	int ownerID;       //owner machine id
	int mailboxID;      //owner mailbox number
	List* waitQue;

};

class serverCV {
public:
	serverCV(char* dName);
	~serverCV();

	void Signal(serverLock *sLock, int outAddr, int outBox,int fromBox);
	void Wait(serverLock *sLock, int outAddr, int outBox,int fromBox);
	void Boardcast(serverLock *sLock, int outAddr, int outBox,int fromBox);

	char* name;
	serverLock* waitLock;
	List* waitQue;
};

class serverMV {
public:
	serverMV(char* mName, int size);
	~serverMV();
	void Read(int pos,int outAddr,int outBox,int fromBox);
	void Set(int toSet,int pos,int outAddr,int outBox,int fromBox);

	char* name;
	int index;
	int* array; // array of size len
    int len;
};

#endif
