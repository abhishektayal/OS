// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue) {
	name = debugName;
	value = initialValue;
	queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore() {
	delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void Semaphore::P() {
	IntStatus oldLevel = interrupt->SetLevel(IntOff); // disable interrupts

	while (value == 0) { // semaphore not available
		queue->Append((void *) currentThread); // so go to sleep
		currentThread->Sleep();
	}
	value--; // semaphore available,
	// consume its value

	(void) interrupt->SetLevel(oldLevel); // re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void Semaphore::V() {
	Thread *thread;
	IntStatus oldLevel = interrupt->SetLevel(IntOff);

	thread = (Thread *) queue->Remove();
	if (thread != NULL) // make thread ready, consuming the V immediately
		scheduler->ReadyToRun(thread);
	value++;
	(void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!


//----------------------------------------------------------------------
// Lock::Lock
//      Initialize a lock, so that it can be used for synchronization.
//
//      waitingqueue stores threads waiting for the lock
//      "debugName" is an arbitrary name, useful for debugging.
//      stateoflock sets the current state of the lock
//----------------------------------------------------------------------



Lock::Lock(char* debugName)
{
	name = debugName;
	waitingqueue = new List;
	stateoflock = UNLOCKED;
	owneroflock = NULL;
}

Lock::~Lock()
{
	delete waitingqueue;
}

// isHeldByCurrentThread

bool Lock::isHeldByCurrentThread(void)
{
	if (currentThread== owneroflock)
			{
				return TRUE;
			}
	else
	{
		return FALSE;
	}
}



//acquire
void Lock::Acquire() {
	IntStatus oldLevel = interrupt->SetLevel(IntOff);  //Turns off the interrupts

	if (isHeldByCurrentThread())  //Checks if currentThread is the lockowner

	{
		DEBUG('t', "oops i am the lock owner !! \n");
		(void) interrupt->SetLevel(oldLevel);   //Restore interrupts
		return;
	}

	if (stateoflock == UNLOCKED) {
		stateoflock = LOCKED;		//Sets the lock to busy
		owneroflock = currentThread; //resets the lock owner

	} else {
		waitingqueue->Append((void *) currentThread);  //Add thread to waiting queue

		currentThread->Sleep();     //Put thread to sleep
	}

	(void) interrupt->SetLevel(oldLevel);
}

//release

void Lock::Release() {
	IntStatus oldLevel = interrupt->SetLevel(IntOff);
	Thread *nowactive;

	if (!isHeldByCurrentThread()) //Checks if currentThread is not the lockowner
	{
		DEBUG('t', "ERROR!! thread not authorized to release lock \n");
		(void) interrupt->SetLevel(oldLevel);   //restore interrupts
		return;
	}

	if (waitingqueue->IsEmpty() == FALSE)    //Checks to see if waitingQueue is empty
	{
		nowactive = (Thread *) waitingqueue->Remove();  //remove a thread from the waitingQueue
		scheduler->ReadyToRun(nowactive);                //Wake up thread

		owneroflock = nowactive;                         //sets thread to lock owner
	}

	else {
		stateoflock = UNLOCKED;   //make the lock available

		owneroflock = NULL;       //Clear the lock ownership

	}

	(void) interrupt->SetLevel(oldLevel);   //Restore interrupts

}

//----------------------------------------------------------------------
// Condition::Condition
//      Initialize condition
//
//      "debugName" is an arbitrary name, useful for debugging.
//       "conditionwaitqueue" stores threads on the condition waitQueue
//      "waitingLock" stores current lock pointer
// ---------------------------------------------------------------------


Condition::Condition(char* debugName)

{
		name = debugName;
		conditionwaitqueue = new List;
		waitinglock = NULL;
}
Condition::~Condition()
{
	delete conditionwaitqueue;
}

void Condition::Wait(Lock* conditionLock) {

	// ASSERT(FALSE);
	IntStatus oldLevel = interrupt->SetLevel(IntOff);   //Turns off the interrupts

		if (conditionLock == NULL)                      //If wait is called on a null pointer
		{
			DEBUG('t', "ACCESS DENIED: received a null pointer for lock");
			(void) interrupt->SetLevel(oldLevel);        //restore interrupts
			return;
		}

		if(waitinglock== NULL)
		{
			waitinglock=conditionLock;	// first thread calling wait, save the lock
			DEBUG('t', "this is the first thread accesing wait");
		}

		// make sure the input lock matches the saved lock
		if (waitinglock!= conditionLock)
		{
			DEBUG('t', "ERROR: trying to wait with a wrong lock");
			(void) interrupt->SetLevel(oldLevel);          //restore interrupts
			return;
		}

		// everything ok to be a waiter:

		conditionwaitqueue->Append(currentThread);			// adding thread to conditionwaitqueue
		conditionLock->Release();                           //Exit the monitor

		currentThread->Sleep();                             //Put thread to sleep

		conditionLock->Acquire();                           //re-enter monitor


	(void) interrupt->SetLevel(oldLevel);                   //restore interrupts

}

void Condition::Signal(Lock* conditionLock)
{
	Thread* T1;

	IntStatus oldLevel = interrupt->SetLevel(IntOff);    //disable interrupts

	if (conditionwaitqueue->IsEmpty())					// cheking if conditionwait queue is empty
	{
		(void) interrupt->SetLevel(oldLevel);            //restore interrupts
		return;
	}

	if(waitinglock!=conditionLock)
	{
		DEBUG('t',"ERROR: trying to signal with a wrong lock");
		(void) interrupt->SetLevel(oldLevel);             //restore interrupts
		return;

	}

	// now ok to wakup waiting thread from conditionwaitqueue

	T1 = (Thread*) conditionwaitqueue->Remove();     //remove a thread from the conditionwaitqueue

	scheduler->ReadyToRun(T1);                       //Wake up thread, puts them on the ready queue

	if(conditionwaitqueue->IsEmpty())
	{
		waitinglock = NULL;                          //if no other threads waiting set lock to NULL
	}


	(void) interrupt->SetLevel(oldLevel);             //restore interrupts

}

int Condition::Broadcast(Lock* conditionLock)
{
	int a=0;
	while (!conditionwaitqueue->IsEmpty() )          //while there are waiting threads
	{
		a++;
		Signal(conditionLock);                       //signal each of them
	}



}
