/*
 * test9.c
 *
 *  Created on: Oct 8, 2010
 *      Author: Abhi
 */

/*2 execs..and it should call 2 process and in which 2 threads tries to acquire each other's lock...(this is first process)*/
#include "syscall.h"
int a,count;

void thread1(void);

void main()
 {

	a = CreateLock("lock1",5);

	count =0;

	Fork(thread1);
	Yield();
	Yield();
	Yield();

}

void thread1()
{
    Acquire(0);

	Yield();


	Release(a);
	Exit();
}
