#include "serversynch.h"

using namespace std;

//TODO: owen the lock when I create it?
serverLock::serverLock(char* dName, int owner, int mailbox) {
	name = dName;
	ownerID = owner;
	mailboxID = mailbox;
	waitQue = new List();
	state = FREE;
}

serverLock::~serverLock() {
	delete waitQue;
}

void serverLock::Acquire(int outAddr, int outBox) {
	char* msg = new char[MaxMailSize];

	//free or sender already has the lock
	if (state == FREE || (ownerID == outAddr && mailboxID == outBox)) {
		//proceed
		state = BUSY;
		ownerID = outAddr;
		mailboxID = outBox;

		msg = "0";
		ServerReply(msg, outAddr, mailbox, 0);
	} else {
		//append msg to wait queue
		string toAppend;
		stringstream ss;
		ss << outAddr << " " << outBox;
		toAppend = ss.str();
		msg = (char*) toAppend.c_str();
		waitQue->Append((void*) toAppend);
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
		ownerId = waitAddr;
		mailboxId = waitBox;

		waitMsg = "0";
		ServerReply(waitMsg, waitAddr, waitBox, 0);

		//wait queue empty, change state and clear owner&mailbox
	} else {
		state = FREE;
		ownerID = -1;
		mailboxId = -1;
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
		printf("serverCV %d Signal:pass in lock is not valid\n", name);
		success = false;
	} else if (waitLock != sLock) {
		printf("serverCV %d Signal: pass in lock not the same as waiting on\n",
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
	serverReply(msg, outAddr, outBox, 0);
}

void serverCV::Wait(serverLock *sLock, int outAddr, int outBox) {
	char* msg = new char[MaxMailSize];

	bool success = true;

	//check if lock is vaild
	if (sLock == NULL) {
		printf("serverCV %d Wait:pass in lock is not valid\n", name);
		success = false;
	}

	if (waitLock == NULL) {
		waitLock = sLock;
	} else if (waitLock != sLock) {
		printf("serverCV %d Wait: pass in lock not the same as waiting on\n",
				name);
		success = false;
	}

	if (!success) {
		msg = "1";
		serverReply(msg, outAddr, outBox, 0);
	} else {
		waitLock->Release(outAddr, outBox);
		//append msg to wait queue
		string toAppend;
		stringstream ss;
		ss << outAddr << " " << outBox;
		toAppend = ss.str();
		msg = (char*) toAppend.c_str();
		waitQue->Append((void*) toAppend);
	}
}

void serverCV::Boardcast(serverLock *sLock,int outAddr,int outBox) {
	char* msg = new char[MaxMailSize];
	msg = "0";      //default to success

	bool success = true;

	//check if lock is vaild
	if (sLock == NULL) {
		printf("serverCV %d Boardcast:pass in lock is not valid\n", name);
		success = false;
	}

	if (waitLock != sLock) {
		printf("serverCV %d Boardcast: pass in lock not the same as waiting on\n",
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
	serverReply(msg, outAddr, outBox, 0);
}
