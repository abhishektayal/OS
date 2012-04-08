/*
 * test8.c
 *
 *  Created on: Oct 8, 2010
 *      Author: Abhi
 */

/*fork 100 threads in 1 proc..*/
#include "syscall.h"

int a,b,count,j;
void thread1(void);

void main()
 {

	a = CreateLock("lock1",5);
	b = CreateCV("cond1",5);

	count =0;

	for( j=0;j<105;j++)
	{
		Fork(thread1);
	}
}

void thread1()
{
    Acquire(a);
	Wait(b,a);
	Release(a);
	Exit();
}
