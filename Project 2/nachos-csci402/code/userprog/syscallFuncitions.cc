//This file holds all the function definitions used in exception.cc for syscalls 
#include "syscall.h"

using namespace std;

int ArrayMaxSize = 20;
//Max size can be changed 
void** lockAndConditionArray = new void*[ArrayMaxSize]();

//this holds the pointer to the lock and the condition
struct SysCondition {
  Lock*      conditionLock;
  Condition* sysCondition;
};

struct kernelLock
{
	Lock* Lock;
	AddrSpace* owner
	bool isToBeDeleted;
};

struct kernelCondtion
{
	
};

void Acquire_Syscall(int id)
{
	//if(lockTable[value /*from user*/].lock->owner == NULL){}
	    Lock* sysLock = (Lock*)lockAndConditionArray[id];
	    if (sysLock != NULL) sysLock->Acquire();
}

void Release_Syscall(int id)
{
	//
    Lock* sysLock = (Lock*)lockAndConditionArray[id];
    if (sysLock != NULL) sysLock->Release();
}

void Exit_Syscall(int status)
{
    // currentThread->Finish();

}