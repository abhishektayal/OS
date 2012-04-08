/* halt.c
 *	Simple program to test whether running a user program works.
 *	
 *	Just do a "syscall" that shuts down the OS.
 *
 * 	NOTE: for some reason, user programs with global data structures 
 *	sometimes haven't worked in the Nachos environment.  So be careful
 *	out there!  One option is to allocate data structures as 
 * 	automatics within a procedure, but if you do this, you have to
 *	be careful to allocate a big enough stack to hold the automatics!
 */

#include "syscall.h"

int a,b;
int count =0;
void thread1(void);
void thread2(void);
void thread3(void);
void thread4(void);
main()
{




	int count =0;

	Printf1((unsigned int)"testing printf",count);
	    Fork(thread1);
	    Fork(thread2);
	    Fork(thread3);
	    	    Fork(thread4);
	    Yield();
	    Yield();
	    Yield();
Exit();
}

	void thread1()
	{
	  Printf("inthread1");
	  Yield();
		Exit();
	}

	void thread2()
	{

		  Printf("inthread2");
		  Yield();
		Exit();
	}

	void thread3()
	{
		Acquire(a);
		Wait(b,a);
		Release(a);
		Exit();
	}

	void thread4()
	{
		Acquire(a);
		Yield();
		Yield();
		Broadcast(b,a);
		Release(a);
		DestroyLock(a);
		Exit();
	}


