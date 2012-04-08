/*
 * test10.c
 *
 *  Created on: Oct 8, 2010
 *      Author: Abhi
 */
#include "syscall.h"
/*2 execs..and it should call 2 process and in which 2 threads tries to acquire each other's lock...(this is second process)*/


void thread2(void);
void main()
{
	Fork(thread2);
	Yield();
	Yield();
	Yield();
	Yield();
	Exit();
}

void thread2()
{
    Acquire(0);

	Yield();

	Release(0);
	Exit();
}
