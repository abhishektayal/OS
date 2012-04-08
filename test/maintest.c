/*
 * maintest.c
 *
 *  Created on: Oct 8, 2010
 *      Author: Abhi
 */

#include "syscall.h"
void main()
{
 int input;
 int q,c,i;

Printf("\n1 general interaction between two threads\n");
 Printf("\n2 threads trying to work on lock that has not been created\n");
 Printf("\n3 Broadcast all the waiting threads\n");
 Printf("\n4 signal done by thread but no one waiting..\n");
 Printf("\n5 many threads waiting.....but only one signal\n");
 Printf("\n6 one thread waiting on some lock and other thread signals it on some other lock..\n");
 Printf("\n7 lock number is greater than max locks spacei.e 100\n");
 Printf("\n8 forking more than 100 threads in 1 proc..which is beyond the set limit ... so error\n");
 Printf("\n9 Two execs..and it should call 2 process and in which 2 threads tries to acquire each other's lock..\n");
 Printf("\n10 lock acquired by one thread so all other should wait for the thread to release the lock.. \n");
 Printf("\n11 testing destroy lock and destroy CV system call	 \n");
 Printf("\n12 running 2 carl jr proccesses with  \nCUSTOMERS:30 \n OT:5\n COOKS:3\nWAITER:5\nMANAGER:1\n");
 input= Scanf();

 switch ( input)
 {
 case 1:
   q=  Exec("../test/test1",13);
   break;
 case 2:
	  q=  Exec("../test/test2",13);
   break;
 case 3:
	  q=  Exec("../test/test3",13);
   break;
 case 4:
	  q=  Exec("../test/test4",13);
   break;
 case 5:
	  q=  Exec("../test/test5",13);
   break;
 case 6:
	  q=   Exec("../test/test6",13);
   break;
 case 7:
	  q=  Exec("../test/test7",13);
   break;
 case 8:
	  q= Exec("../test/test8",13);
   break;
 case 9:
	  q= Exec("../test/test9",13);
	  c=  Exec("../test/test10",14);
	  Yield();
   break;
 case 10:
	  q= Exec("../test/test11",14);
   break;

 case 11:

	 q= Exec("../test/test12",14);

	 break;
 case 12:

	 q= Exec("../test/carljr",14);
	 c= Exec("../test/carljr",14);
	 break;
}
for(i=0;i<10000;i++)
{
 Yield();
}

}
