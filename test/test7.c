/*
 * test7.c
 *
 *  Created on: Oct 8, 2010
 *      Author: Abhi
 */
#include "syscall.h"
/*lock and cv number is greater than max locks space..i.e 100*/
void thread1(void);
void thread2(void);

int a,b,count;
void main()
{
	a = CreateLock("lock1",5);
	b = CreateCV("cond1",5);

	Fork(thread1);
}

void thread1()
{


    Acquire(a+101);

	Release(a+101);
	Exit();
}




