#include "syscall.h"

int a,b,count;
void thread1(void);
void thread2(void);
void main()
{
	count =0;

	a = CreateLock("lock1",5);
	b = CreateCV("cond1",5);

	Fork(thread1);
	Fork(thread2);
	Yield();
	Yield();
	Yield();
}

void thread1()
{
    Acquire(a);

    if(count ==0)
	{
		count++;
		Wait(b,a);
		Release(a);
	}

	else
	{
		count--;
		Signal(b,a);
		Release(a);
    }
	Exit();
}


void thread2()
{
	Acquire(a);

	if(count > 0)
	{
		count--;
		Signal(b,a);
		Release(a);
	}
	else
	{
		count++;
		Wait(b,a);
        Release(a);
	}
	Exit();
}
