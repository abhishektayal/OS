/*
 * test1.cc
 *
 *  Created on: Oct 8, 2010
 *      Author: Abhi
 */


#include "syscall.h"

int a,b;
int count =0;
void thread1(void);
void thread2(void);
void thread3(void);
void thread4(void);
void main()
{




	int count =0;

	int a = CreateLock("lock1",5);
	int b = CreateCV("cond1",5);
	    Fork(thread1);
	    Fork(thread2);
	    Fork(thread3);
	    Fork(thread4);
Exit();
}

	void thread1()
	{
	    Acquire(a);
		Wait(b,a);
		Release(a);
		Exit();
	}

	void thread2()
	{
		Acquire(a);
		Wait(b,a);
		DestroyLock(a);
		Release(a);
		DestroyLock(a);
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


