//This file holds all the function definitions used in exception.cc for syscalls 
#include "syscall.h"

using namespace std;

struct kernelLock
{
	Lock* Lock;
	AddrSpace* owner
	bool isToBeDeleted;
};

struct kernelCondtion
{
	Condition* Condition
	AddrSpace* owner
	bool isToBeDeleted;
};

void Fork_Syscall(unsigned int vaddr, int len)
{
    char *buf = new char[len+1];	// Kernel buffer: func

    if (! buf)
    {
        printf("%s","Can't allocate kernel buffer in Fork\n");
        return -1;
    }

    if( copyin(vaddr, len, buf) == -1 )
    {
        printf("%s","Bad pointer passed to Fork\n");
        delete[] buf;
        return -1;
    }

    buf[len]='\0';
}

void Exec_Syscall(unsigned int vaddr, int len)
{
    char *buf = new char[len+1];	// Kernel buffer: filename

    if (! buf)
    {
        printf("%s","Can't allocate kernel buffer in Exec\n");
        return -1;
    }

    if( copyin(vaddr, len, buf) == -1 )
    {
        printf("%s","Bad pointer passed to Exec\n");
        delete[] buf;
        return -1;
    }

    buf[len]='\0';
}

void Yield_Syscall()
{
    currentThread->Yield();
    printf("Yield_Syscall was called; yielding current thread.\n");
}

void Exit_Syscall(int status)
{
    // currentThread->Finish();

}

void Acquire_Syscall(int id)
{
    Lock* sysLock = (Lock*)lockAndConditionArray[id];
    if (sysLock != NULL) sysLock->Acquire();
}

void Release_Syscall(int id)
{
    Lock* sysLock = (Lock*)lockAndConditionArray[id];
    if (sysLock != NULL) sysLock->Release();
}

void Wait_Syscall(int id)
{
    SysCondition* sysCond = (SysCondition*)lockAndConditionArray[id];
    if (sysCond != NULL) sysCond->sysCondition->Wait(sysCond->conditionLock);
}
void Signal_Syscall(int id)
{
    SysCondition* sysCond = (SysCondition*)lockAndConditionArray[id];
    if (sysCond != NULL) sysCond->sysCondition->Signal(sysCond->conditionLock);
}
void Broadcast_Syscall(int id)
{
    SysCondition* sysCond = (SysCondition*)lockAndConditionArray[id];
    if (sysCond != NULL) sysCond->sysCondition->Broadcast(sysCond->conditionLock);
}

int CreateLock_Syscall(unsigned int vaddr, int len)
{
    char *buf = new char[len+1];	// Kernel buffer: name

    if (! buf)
    {
        printf("%s","Can't allocate kernel buffer in CreateLock\n");
        return -1;
    }

    if( copyin(vaddr, len, buf) == -1 )
    {
        printf("%s","Bad pointer passed to CreateLock\n");
        delete[] buf;
        return -1;
    }

    buf[len]='\0';
    
    int index = 0;
    Lock* sysLock = new Lock(buf);
    
    //looks for the first empty location in the array
    while(ArrayMaxSize > index){
        if(lockAndConditionArray[index] == NULL)
        {
            lockAndConditionArray[index] = (void*) sysLock;
            break;
        }
        else
            index++;
    }
    return index;

}

int CreateCondition_Syscall(unsigned int vaddr, int len)
{

    char *buf = new char[len+1];	// Kernel buffer: name

    if (! buf)
    {
        printf("%s","Can't allocate kernel buffer in CreateCondition\n");
        return -1;
    }

    if( copyin(vaddr, len, buf) == -1 )
    {
        printf("%s","Bad pointer passed to CreateCondition\n");
        delete[] buf;
        return -1;
    }

    buf[len]='\0';
    
    char* name;
    int index = 0;
    Condition* c = new Condition(buf);
    Lock* L = new Lock(buf);
    SysCondition* conditionSyscall = new SysCondition();

    conditionSyscall->sysCondition = c;
    conditionSyscall->conditionLock = L;

    while(ArrayMaxSize > index)
    {
        if(lockAndConditionArray[index] == NULL)
        {
            lockAndConditionArray[index] = (void*) conditionSyscall;
            break;
        }
        else
            index++;

    }

    return index;
}
void DestroyLock_Syscall(int id)
{
    if((id <= ArrayMaxSize) || (lockAndConditionArray[id] = NULL)){
      printf("ID for lock %d is out of lockAndConditionArray index or the value is NULL!\n", id);
    }
    else{
      Lock* sysLock = (Lock*)lockAndConditionArray[id];
      delete sysLock;
      lockAndConditionArray[id] = NULL;
    }
}
void DestroyCondition_Syscall(int id)
{
    if((id <= ArrayMaxSize) || (lockAndConditionArray[id] = NULL)){
      printf("ID for condition and lock %d is out of lockAndConditionArray index or the value is NULL!\n", id);
    }
    else{
      SysCondition* sysC = (SysCondition*)lockAndConditionArray[id];
      delete sysC->conditionLock;
      delete sysC->sysCondition;
      delete sysC;
      lockAndConditionArray[id] = NULL;
    }
}

void Printf_Syscall(unsigned int vaddr, int len, int param1, int param2)
{char *buf = new char[len+1];	// Kernel buffer: name

    if (! buf)
    {
        printf("%s","Can't allocate kernel buffer in CreateCondition\n");
        return -1;
    }

    if( copyin(vaddr, len, buf) == -1 )
    {
        printf("%s","Bad pointer passed to CreateCondition\n");
        delete[] buf;
        return -1;
    }

    buf[len]='\0';
    
    printf(buf, param1, param2, param3);
}
