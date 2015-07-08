// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "../threads/system.h"
#include "addrspace.h"
#include "noff.h"
// #include "table.h"
// #include "../threads/synch.h"
#include <stdio.h>      /* printf, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */


// forward declarations
int getFreePage();
int getPPN(int vpn);

extern "C" { int bzero(char *, int); };
TranslationEntryIPT* ipt;
Lock* iptLock;
BitMap* swapFileMap;
int currentTLB;
int evictionPolicy;
OpenFile* swapFile;
OpenFile* debugFile;

Table::Table(int s) : map(s), table(0), lock(0), size(s) {
    table = new void *[size];
    lock = new Lock("TableLock");
    count = 0;
}

Table::~Table() {
    if (table) {
	delete table;
	table = 0;
    }
    if (lock) {
	delete lock;
	lock = 0;
    }
}

void *Table::Get(int i) {
    // Return the element associated with the given i, or 0 if
    // there is none.

    return (i >=0 && i < size && map.Test(i)) ? table[i] : 0;
}

int Table::Put(void *f) {
    // Put the element in the table and return the slot it used.  Use a
    // lock so 2 files don't get the same space.
    int i;	// to find the next slot

    lock->Acquire();
    i = map.Find();
    lock->Release();
    if ( i != -1){
    	table[i] = f;
        count++;
        maxCount++;
    }
    return i;
}

void *Table::Remove(int i) {
    // Remove the element associated with identifier i from the table,
    // and return it.

    void *f =0;

    if ( i >= 0 && i < size ) {
	lock->Acquire();
	if ( map.Test(i) ) {
	    map.Clear(i);
	    f = table[i];
	    table[i] = 0;
        count--;
	}
	lock->Release();
    }
    return f;
}

void Table::lockAcquire() {
    // Acquire table's lock
	lock->Acquire();
}
void Table::lockRelease() {
    // Acquire table's lock
	lock->Release();
}

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	"executable" is the file containing the object code to load into memory
//
//      It's possible to fail to fully construct the address space for
//      several reasons, including being unable to allocate memory,
//      and being unable to read key parts of the executable.
//      Incompletely consrtucted address spaces have the member
//      constructed set to false.
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable) : fileTable(MaxOpenFiles) {
    
    srand(time(NULL));
    
    exec = executable;
    NoffHeader noffH;
    int i;
    unsigned int size;
    int maxNumThreads = 128;
    id = processTable->Put(this);           //adds a process to the process table
    FIFOEvictionQueue = new List();
    srand(time(NULL));
    //Keep track of all of the threads that belong to the process
    threadTable = new Table(maxNumThreads);
    currentThread->setThreadTableLocation(threadTable->Put(currentThread));        //adds the thread to the thread table

    // Don't allocate the input or output to disk files
    fileTable.Put(0);
    fileTable.Put(0);

    exec->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size ;
    DEBUG('a', "Initializing address space... size %d\n", size);
    //size of the code
    execSize = divRoundUp(size, PageSize);
    
    numPages = divRoundUp(size, PageSize) + divRoundUp(UserStackSize,PageSize);
                        // we need to increase the size
						// to leave room for the stack

    size = numPages * PageSize;     //page size is 128 bytes

    //ASSERT(numPages <= NumPhysPages);	// check we're not trying
                                        // to run anything too big --
                                        // at least until we have
                                        // virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
					numPages, size);
    
    pageTable = new TranslationEntryExec[numPages];

    for (i = 0; i < (int)numPages; i++)
    {
        DEBUG('z', "AddrSpace: setting page %d invalid\n", i);
        
    	pageTable[i].valid = FALSE;
        pageTable[i].inSwapFile = FALSE;
        if (i < execSize)
        {
            pageTable[i].inExec = EXEC;
            pageTable[i].byteOffset = noffH.code.inFileAddr + i*PageSize;
        }
        else pageTable[i].inExec = NEITHER;
    }
}   

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//
// 	Dealloate an address space.  release pages, page tables, files
// 	and file tables
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    delete pageTable;
    delete threadTable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::setNewPageTable(){
    //unsigned int tempNumPages = numPages;
    //unsigned int size = noffH.code.size + noffH.initData.size + noffH.uninitData.size ;
    unsigned int previousNumPages = numPages;
    numPages += divRoundUp(UserStackSize,PageSize); //decrement on exit
    TranslationEntryExec *tempTable = new TranslationEntryExec[numPages];

    int ppn = 0;
    //copy over old Page Table data
    for (unsigned int i = 0; i < previousNumPages; i++)
    {
        DEBUG('z', "setNewPageTable: copying page %i\n", i);
        
        tempTable[i].virtualPage = pageTable[i].virtualPage;   
        tempTable[i].physicalPage = pageTable[i].physicalPage;
        tempTable[i].valid = pageTable[i].valid;
        tempTable[i].use = pageTable[i].use;
        tempTable[i].dirty = pageTable[i].dirty;
        tempTable[i].readOnly = pageTable[i].readOnly;
        tempTable[i].inExec = pageTable[i].inExec;
        tempTable[i].byteOffset = pageTable[i].byteOffset;
        tempTable[i].inSwapFile = pageTable[i].inSwapFile;
    }

    for (unsigned int i = previousNumPages; i < numPages; ++i)
    {
        DEBUG('z', "setNewPageTable: setting page %d invalid\n", ppn);
        
        tempTable[i].valid = FALSE;
        pageTable[i].inExec = NEITHER;
        pageTable[i].inSwapFile = FALSE;
    }

    delete pageTable; 
    pageTable = tempTable;

    RestoreState();
}
//----------------------------------------------------------------------------
// The following two functions are the different eviction policies used to replace
// pages in memory. The 2 policies are Random and FIFO.
// Random Eviction will remove a random page from the IPT seeded by the runtime of the 
// process. FIFO Eviction will evict the page that has been in the IPT the longest.
//----------------------------------------------------------------------------
//Nachos -x ../test/matmult > debugFile

int AddrSpace::HandleMemoryFull(){
    int pageIndex = 0;

    iptLock->Acquire();
    
   //Random Eviction
    if(evictionPolicy == 1){             //default set in system.cc
        do
        {
            pageIndex = rand() % NumPhysPages;
        } while (ipt[pageIndex].use);
        ipt[pageIndex].use = true;
        DEBUG('z', "HandleMemoryFull: Randomly evicted page %d from the IPT\n", pageIndex);
    }
    
    //FIFO Eviction
    else{
        do
        {
            pageIndex = (int) FIFOEvictionQueue->Remove();
        } while (ipt[pageIndex].use);
        ipt[pageIndex].use = true;
        DEBUG('z', "HandleMemoryFull: Evicted page %d stored in the FIFO from the IPT\n", pageIndex);
    } 
    //DEBUG('p', "ipt[pageIndex].processID = %d, my id is %d\n", ipt[pageIndex].processID, id);

    iptLock->Release();
    
    if (ipt[pageIndex].processID == id){
        //look for ipt.vpn in the tlb
        //if a match, check to see if it's valid
        //if valid, propagate and set the tlb invalid
        for(int i = 0;i < TLBSize; i++){
            if(machine->tlb[i].virtualPage == ipt[pageIndex].virtualPage){
                if(machine->tlb[i].valid){
                    DEBUG('z', "My page %d will propagate the dirty bit\n", pageIndex);
                    ipt[pageIndex].dirty = machine->tlb[i].dirty;
                    machine->tlb[i].valid = FALSE;
                    break;
                }
            }
        }
    }

    AddrSpace* AddrSPtemp =  (AddrSpace*)processTable->Get(ipt[pageIndex].processID);
    //If dirty is true, move to swap
    if(ipt[pageIndex].dirty){
        DEBUG('p', "HandleMemoryFull: Accessing swapfile\n");
        //write to swapfile
        if(AddrSPtemp->pageTable[ipt[pageIndex].virtualPage].inSwapFile){
            DEBUG('p',  "Page from page table is already in the swapfile\n");
            swapFile->WriteAt(&(machine->mainMemory[pageIndex * PageSize]), PageSize, AddrSPtemp->pageTable[ipt[pageIndex].virtualPage].byteOffset);
            
            DEBUG('a', "Page written to same swap: vpn is %d, ppn is %d, byteOffset is %d, from swapfile\n", 
                ipt[pageIndex].virtualPage, ipt[pageIndex].physicalPage, AddrSPtemp->pageTable[ipt[pageIndex].virtualPage].byteOffset);
            cout << "Page written to same swap: vpn is: " << ipt[pageIndex].virtualPage << ", ppn is: "  << ipt[pageIndex].physicalPage << ", byteOffset is: " << AddrSPtemp->pageTable[ipt[pageIndex].virtualPage].byteOffset <<" from swapfile" << endl;
        }

        else{
            int sf = swapFileMap->Find();
            if(sf != -1){
                
                swapFile->WriteAt(&(machine->mainMemory[pageIndex * PageSize]), PageSize, PageSize*sf);
                AddrSPtemp->pageTable[ipt[pageIndex].virtualPage].inSwapFile = TRUE;
                AddrSPtemp->pageTable[ipt[pageIndex].virtualPage].byteOffset = PageSize*sf;
               
                DEBUG('p', "HandleMemoryFull: Writing page %d of ipt to swapfile pos (byte offset) %d.\n",
                 pageIndex, AddrSPtemp->pageTable[ipt[pageIndex].virtualPage].byteOffset);
                DEBUG('a', "Page written to new swap pos: vpn is %d, ppn is %d, byteOffset is %d, from swapfile\n", 
                 ipt[pageIndex].virtualPage, ipt[pageIndex].physicalPage, 
                 AddrSPtemp->pageTable[ipt[pageIndex].virtualPage].byteOffset);
                cout << "Page written to new swap: vpn is: " << ipt[pageIndex].virtualPage << ", ppn is: "  << ipt[pageIndex].physicalPage << ", byteOffset is: " << AddrSPtemp->pageTable[ipt[pageIndex].virtualPage].byteOffset <<" from swapfile" << endl;

            }
            else
                printf("HandleMemoryFull: swapfile full!!\n");
        }
    }

    //ipt[pageIndex].valid = FALSE;

    return pageIndex;
} 

//cout << nachos > test_output_file;

int AddrSpace::HandleIPTMiss(int vpn)
{
    int ppn = getFreePage();
    
    // if ppn = -1, kick a page
    if(ppn == -1)
        ppn = HandleMemoryFull();

    // if not -1, then add the ppn the the FIFO queue
    
    // if vpn is not stack
    //  copy from executable
    if (!pageTable[vpn].valid && (vpn < execSize))
    {
        DEBUG('z', "HandleIPTMiss: copying code from executable at offset %d\n", pageTable[vpn].byteOffset);
        
        exec->ReadAt(&(machine->mainMemory[ppn * PageSize]), PageSize, pageTable[vpn].byteOffset);
        
        pageTable[vpn].inExec = EXEC; // should already be set, but just in case
        DEBUG('a', "Reading from executable file, ppn is %d and byteOffset is %d\n", ppn, pageTable[vpn].byteOffset);
        cout << "Reading from executable file: ppn is: " << ppn << " and byteOffset is: "  << pageTable[vpn].byteOffset << endl;

    }

    if(pageTable[vpn].inSwapFile){
        swapFile->ReadAt(&(machine->mainMemory[ppn * PageSize]), PageSize, pageTable[vpn].byteOffset);
        DEBUG('a', "Reading from swap file, ppn is %d and byteOffset is %d\n", ppn, pageTable[vpn].byteOffset);
        cout << "Reading from swap file: ppn is: " << ppn << " and byteOffset is: "  << pageTable[vpn].byteOffset << endl;
    }
    //when you populate the ipt
    

    DEBUG('p', "HandleIPTMiss: setting virtual page %d valid\n", vpn);
    
    pageTable[vpn].virtualPage = vpn;   
    pageTable[vpn].physicalPage = ppn;
    pageTable[vpn].valid = TRUE;
    pageTable[vpn].use = FALSE;
    pageTable[vpn].dirty = FALSE;
    pageTable[vpn].readOnly = FALSE;
    
    DEBUG('p', "HandleIPTMiss: setting physical page %d valid\n", ppn);
    
    ipt[ppn].virtualPage = vpn;
    ipt[ppn].physicalPage = ppn;
    ipt[ppn].space = this;
    ipt[ppn].processID = id;
    ipt[ppn].valid = TRUE;
    ipt[ppn].use = FALSE;
    ipt[ppn].dirty = FALSE;
    ipt[ppn].readOnly = FALSE;

    DEBUG('a', "Page loaded to ipt: vpn is %d, ppn is %d, byteOffset is %d\n", 
                ipt[ppn].virtualPage, ipt[ppn].physicalPage, pageTable[vpn].byteOffset);
    cout << "Page loaded to iptp: vpn is: " << ipt[ppn].virtualPage << ", ppn is: "  << ipt[ppn].physicalPage << ", byteOffset is: " << pageTable[vpn].byteOffset << endl;


    // add ppn to the FIFO queue
    FIFOEvictionQueue->Append((void*)ppn);
    //look through the page table and if it is in the swap file then read it back in


    
    return ppn;
}
//Copy page table info to the tlb
void AddrSpace::PageFault(){
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    //page table index
    int PTIndex = getPPN((int)machine->ReadRegister(39)/PageSize); // will return -1 if not found
    //PTIndex = machine->ReadRegister(39)/PageSize;
    DEBUG('z', "PageFault: reg = %d, vpn = %d, ppn = %d\n", (int)machine->ReadRegister(39), (int)machine->ReadRegister(39)/PageSize, PTIndex);

    if (PTIndex == -1)
    {
        PTIndex = HandleIPTMiss((int)machine->ReadRegister(39)/PageSize);
    }
    
    DEBUG('z', "PageFault: copying ppn %d to tlb %d\n", PTIndex, currentTLB);
    

    machine->tlb[currentTLB].virtualPage = ipt[PTIndex].virtualPage;
    machine->tlb[currentTLB].physicalPage = ipt[PTIndex].physicalPage;
    machine->tlb[currentTLB].valid = ipt[PTIndex].valid;
    machine->tlb[currentTLB].use = ipt[PTIndex].use;
    machine->tlb[currentTLB].dirty = ipt[PTIndex].dirty; 
    machine->tlb[currentTLB].readOnly = ipt[PTIndex].readOnly;
    
    if(currentTLB >= TLBSize - 1)
        currentTLB = 0;
    else
        currentTLB++;

    //works like a circular queue
    //currentTLB = (currentTLB++) % TLBSize;            //doesn't work :(
    (void) interrupt->SetLevel(oldLevel);  //reenable interrupts  
}

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %x\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() {}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//  This will also invalitdate the TLB
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    //DEBUG('z', "RestoreState has been called\n");
    // machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
    //invalidate the TLB
    for(int i = 0; i <TLBSize; i++)
    {
        DEBUG('z', "RestoreState: setting tlb page %d invalid\n", i);

        if(machine->tlb[i].virtualPage == ipt[machine->tlb[i].physicalPage].virtualPage){
            if(machine->tlb[i].valid){
                DEBUG('p', "ipt index on context switch is %d\n", machine->tlb[i].physicalPage);
                ipt[machine->tlb[i].physicalPage].dirty = machine->tlb[i].dirty;
                break;
            }
        }
        machine->tlb[i].valid = false;
    }
    (void) interrupt->SetLevel(oldLevel);  //reenable interrupts     
}

int AddrSpace::getPPN(int vpn)
{
    for (int i = 0; i < NumPhysPages; i++)
    {
        TranslationEntryIPT t = ipt[i];
        //printf("i = %d, valid = %d, processID = %d, id = %d, virtualPage = %d, vpn = %d, physicalPage = %d\n", i, (int)t.valid, t.processID, id, t.virtualPage, vpn, t.physicalPage);
        if (t.valid && t.processID == id && t.virtualPage == vpn)
        {
            return t.physicalPage;
        }
    }
    return -1;
}

int AddrSpace::getFreePage()
{
    for (int i = 0; i < NumPhysPages; i++)
    {
        TranslationEntryIPT t = ipt[i];
        if (! t.valid)
        {
            return t.physicalPage;
        }
    }
    return -1;
}
