#include "serversynch.h"
#include <sstream>
#include <string>

using namespace std;

//tableType: 1 for lockTable, 2 for CVTable
//routine to determine if the item with input name exist
bool tableItemExist(char* tName, Table* table, int tableType) {
	bool toReturn = false;

	for (int i = 0; i < table->Size(); i++) {
		if (tableType == 1) {
			serverLock* tableItem = NULL;
			tableItem = (serverLock*) table->Get(i);
			if (strcmp(tableItem->name, tName) == 0) {
				toReturn = true;
				break;
			}
		} else {
			serverCV* tableItem = NULL;
			tableItem = (serverCV*) table->Get(i);
			if (strcmp(tableItem->name, tName) == 0) {
				toReturn = true;
				break;
			}
		}
	}

	return toReturn;
}

//get table item's index by name, 1 for lockTable, 2 for CVTable
int getTableIndex(char* tName, Table* table, int tableType) {
	int toReturn = -1;

	for (int i = 0; i < table->Size(); i++) {
		if (tableType == 1) {
			serverLock* tableItem = (serverLock*) table->Get(i);
			if (strcmp(tableItem->name, tName) == 0) {
				toReturn = i;
				break;
			}
		} else {
			serverCV* tableItem = (serverCV*) table->Get(i);
			if (strcmp(tableItem->name, tName) == 0) {
				toReturn = i;
				break;
			}
		}

	}

	return toReturn;
}

//routine to send reply to sender from server utlizing postoffice
void ServerReply(char* sMsg, int outMachine, int outMailbox, int fromMailbox) {
	PacketHeader outPktHdr;
	MailHeader outMailHdr;

	outPktHdr.to = outMachine;
	outMailHdr.to = outMailbox;
	outMailHdr.from = fromMailbox;
	outMailHdr.length = strlen(sMsg) + 1;

	postOffice->Send(outPktHdr, outMailHdr, sMsg);

	delete[] sMsg;  //since all the msg used in this function is from "new"
}

serverLock::serverLock(char* dName, int owner, int mailbox) {
	name = dName;
	ownerID = owner;
	mailboxID = mailbox;
	waitQue = new List();
	state = SL_FREE;
}

serverLock::~serverLock() {
	delete waitQue;
}

void serverLock::Acquire(int outAddr, int outBox) {
	char* msg = new char[MaxMailSize];

	//free or sender already has the lock
	if (state == SL_FREE || (ownerID == outAddr && mailboxID == outBox)) {
		//proceed
		state = SL_BUSY;
		ownerID = outAddr;
		mailboxID = outBox;

		//encode success msg and send reply
		msg = "0";
		ServerReply(msg, outAddr, outBox, 0);
	} else {
		//produce and append msg to wait queue
		string toAppend;
		stringstream ss;
		ss << outAddr << " " << outBox;
		toAppend = ss.str();
		msg = (char*) toAppend.c_str();
		waitQue->Append((void*) msg);
	}
}

void serverLock::Release(int outAddr, int outBox) {

	char* msg = new char[MaxMailSize];
	msg = "0"; //default to success

	//if sender not the owner
	if (outAddr != ownerID || outBox != mailboxID) {
		msg = "1";

		//wait queue not empty, remove the first one and send reply to it
	} else if (!waitQue->IsEmpty()) {
		//send reply to waiting userprog
		char* waitMsg = new char[MaxMailSize];
		waitMsg = (char*) waitQue->Remove();

		int waitAddr, waitBox;
		stringstream ss;
		ss << waitMsg;
		ss >> waitAddr >> waitBox;

		//change owner
		ownerID = waitAddr;
		mailboxID = waitBox;

		waitMsg = "0";
		ServerReply(waitMsg, waitAddr, waitBox, 0);

		//wait queue empty, change state and clear owner&mailbox
	} else {
		state = SL_FREE;
		ownerID = -1;
		mailboxID = -1;
	}

	//send reply to sender
	ServerReply(msg, outAddr, outBox, 0);
}

serverCV::serverCV(char* dName) {
	name = dName;
	waitLock = NULL;
	waitQue = new List();
}

serverCV::~serverCV() {
	delete waitQue;
}

void serverCV::Signal(serverLock *sLock, int outAddr, int outBox) {

	char* msg = new char[MaxMailSize];
	msg = "0";      //default to success
	bool success = true;

	//check if lock is valid
	if (sLock == NULL) {
		printf("serverCV %s Signal:pass in lock is not valid\n", name);
		success = false;
	} else if (waitLock != sLock) {
		printf("serverCV %s Signal: pass in lock not the same as waiting on\n",
				name);
		success = false;
	}

	if (!success) {
		msg = "1";
	} else {
		//remove one msg from waitQue
		char* waitMsg = new char[MaxMailSize];
		waitMsg = (char*) waitQue->Remove();

		int waitAddr, waitBox;
		stringstream ss;
		ss << waitMsg;
		ss >> waitAddr >> waitBox;

		//acquire the lock for it (will also send the reply)
		sLock->Acquire(waitAddr, waitBox);
	}

	//send reply to sender
	ServerReply(msg, outAddr, outBox, 0);
}

void serverCV::Wait(serverLock *sLock, int outAddr, int outBox) {
	char* msg = new char[MaxMailSize];

	bool success = true;

	//check if lock is vaild
	if (sLock == NULL) {
		printf("serverCV %s Wait:pass in lock is not valid\n", name);
		success = false;
	}

	if (waitLock == NULL) {
		waitLock = sLock;
	} else if (waitLock != sLock) {
		printf("serverCV %s Wait: pass in lock not the same as waiting on\n",
				name);
		success = false;
	}

	if (!success) {
		msg = "1";
		ServerReply(msg, outAddr, outBox, 0);
	} else {
		waitLock->Release(outAddr, outBox);
		//append msg to wait queue
		string toAppend;
		stringstream ss;
		ss << outAddr << " " << outBox;
		toAppend = ss.str();
		msg = (char*) toAppend.c_str();
		waitQue->Append((void*) msg);
	}
}

void serverCV::Boardcast(serverLock *sLock,int outAddr,int outBox) {
	char* msg = new char[MaxMailSize];
	msg = "0";      //default to success

	bool success = true;

	//check if lock is vaild
	if (sLock == NULL) {
		printf("serverCV %s Boardcast:pass in lock is not valid\n", name);
		success = false;
	}

	if (waitLock != sLock) {
		printf("serverCV %s Boardcast: pass in lock not the same as waiting on\n",
				name);
		success = false;
	}

	if (!success) {
		msg = "1";
	} else {
		while (!waitQue->IsEmpty()) {
			//remove one msg from waitQue
			char* waitMsg = new char[MaxMailSize];
			waitMsg = (char*) waitQue->Remove();

			int waitAddr, waitBox;
			stringstream ss;
			ss << waitMsg;
			ss >> waitAddr >> waitBox;

			//acquire the lock for it (will also send the reply)
			sLock->Acquire(waitAddr, waitBox);
		}
	}

	//send reply to sender
	ServerReply(msg, outAddr, outBox, 0);
}
