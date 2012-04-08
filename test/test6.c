/*
 * test6.c
 *
 *  Created on: Oct 8, 2010
 *      Author: Abhi
 */

/*one thread waiting on some lock and other thread signals it on some other lock..*/

#include "syscall.h"
void thread1(void);
void thread2(void);


int a,b,c,count;

void main()
 {

	a = CreateLock("lock1",5);
	b = CreateCV("cond1",5);
	c = CreateLock("lock2",5);

	count =0;

	Fork(thread1);
	Fork(thread2);

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
	Yield();
	Yield();

	Acquire(a);
	Signal(b,c);
	Release(a);
	Exit();
}
