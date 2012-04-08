/*
 * test4.c
 *
 *  Created on: Oct 8, 2010
 *      Author: Abhi
 */

#include "syscall.h"
void thread1(void);
void thread2(void);


int a,b,count;

void main()
{
	a = CreateLock("lock1",5);
	b = CreateCV("cond1",5);

	Fork(thread1);
		Fork(thread2);

}

void thread1()
{
    Acquire(a);
	Signal(b,a);
	Release(a);
	Exit();
}

void thread2()
{
	Yield();
	Yield();
	Yield();

	Acquire(a);
	Wait(b,a);
    Release(a);
	Exit();
}
