// synchorinzation objects used for server

#ifndef SERVERSYNCH_H
#define SERVERSYNCH_H

#include "list.h"
#include "network.h"
#include "post.h"
//TODO:need to include Table here

enum lockState {
	FREE, BUSY
};

//tableType: 1 for lockTable, 2 for CVTable
bool tableItemExist(string tName, Table* table, int tableType) {
	bool toReturn = false;

	for (int i = 0; i < table->Size(); i++) {
		if (tableType == 1)
			serverLock* tableItem = (serverLock*) sTable->Get(i);
		else
			serverCV* tableItem = (serverCV*) sTable->Get(i);
		if (tableItem->name == lName) {
			toReturn = true;
			break;
		}
	}

	return toReturn;
}

bool ServerReply(char* msg,int outMachine,int outMailbox,int fromMailbox) {
	PacketHeader outPktHdr, inPktHdr;
	MailHeader outMailHdr, inMailHdr;
	char buffer[MaxMailSize];
	stringstream ss;
	ss.str("");
	ss.clear;

	outPktHdr.to = outMachine;
	outPktHdr.from = 0;
	outMailHdr.to = outMailbox;
	outMailHdr.from = fromMailbox;

	postOffice->Send(outPktHdr, outMailHdr, msg);
}

class serverLock {
public:
	serverLock(char* dName, int owner, int mailbox);
	~serverLock();

	void Acquire(int out);
	void Release();

	string name;
	lockState state;
	int owenerID;       //owner machine id
	int mailboxID;      //owner mailbox number
	List* waitQue;

};

class serverCV {
public:
	serverCV(char* dName);
	~serverCV();

	void Signal(serverLock *sLock,int out);
	void Wait(serverLock *sLock,int out);

	char* name;
	serverLock* lock;
	List* waitQue;
	Table* lockTable;
};

#endif
