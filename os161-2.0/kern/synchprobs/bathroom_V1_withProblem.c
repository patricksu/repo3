/*
	无法避免男孩不停进入的情况
 * Copyright (c) 2001, 2002, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <types.h>
#include <lib.h>
#include <clock.h>
#include <thread.h>
#include <test.h>
#include <synch.h>

#define NPEOPLE 20


static volatile unsigned int boy_count;
static volatile unsigned int girl_count;
static volatile unsigned int mode;

static struct semaphore *BrSem;
static struct semaphore *doneSem;
static struct lock      *BrLock;
static struct cv        *boyCV;
static struct cv        *girlCV;

static
void
inititems(void)
{
	if (BrSem==NULL) {
		BrSem = sem_create("BrSem", 3);
		if (BrSem == NULL) {
			panic("BrSem: sem_create failed\n");
		}
	}
	if (doneSem==NULL) {
		doneSem = sem_create("doneSem", 0);
		if (doneSem == NULL) {
			panic("synchtest: sem_create failed\n");
		}
	}
	if (BrLock==NULL) {
		BrLock = lock_create("BrLock");
		if (BrLock == NULL) {
			panic("BrLock: lock_create failed\n");
		}
	}
	if (boyCV==NULL) {
		boyCV = cv_create("boyCV");
		if (boyCV == NULL) {
			panic("boyCV: cv_create failed\n");
		}
	}
	if (girlCV==NULL) {
		girlCV = cv_create("girlCV");
		if (girlCV == NULL) {
			panic("girlCV: cv_create failed\n");
		}
	}
}

static
void
shower()
{
	clocksleep(1);
	// The thread enjoys a refreshing shower!
    //clocksleep(1);
}


static
void
boy(void *p, unsigned long which)
{
	(void)p;
	kprintf("boy #%ld starting\n", which);
	// Implement this function
	boy_count++;
	lock_acquire(BrLock);
	if(mode==0)
	{
		if(boy_count>girl_count)
			mode=1;
		else
			mode=2;
	}
	while(mode==2 || BrSem->sem_count==0)
		cv_wait(boyCV,BrLock);
	P(BrSem);
	boy_count--;
	lock_release(BrLock);
	// use bathroom
	kprintf("boy #%ld entering bathroom...\n", which);	
	shower();
	kprintf("boy #%ld leaving bathroom\n", which);

	lock_acquire(BrLock);
	V(BrSem);
	if(BrSem->sem_count==3)
	{
		mode=0;
		if(girl_count!=0)
		{
			mode=2;
			cv_broadcast(girlCV,BrLock);
		}
		else
		{
			if(boy_count==0)
				mode=0;
			else
			{
				mode=1;
				cv_broadcast(boyCV,BrLock);
			}
		}
	}
	lock_release(BrLock);
	V(doneSem);
}

static
void
girl(void *p, unsigned long which)
{
	(void)p;
	kprintf("girl #%ld starting\n", which);

	// Implement this function
	girl_count++;
	lock_acquire(BrLock);
	if(mode==0)
	{
		if(boy_count>girl_count)
			mode=1;
		else
			mode=2;
	}
	while(mode==1 || BrSem->sem_count==0)
		cv_wait(girlCV,BrLock);
	P(BrSem);
	girl_count--;
	lock_release(BrLock);
	// use bathroom
	kprintf("girl #%ld entering bathroom...\n", which);	
	shower();
	kprintf("girl #%ld leaving bathroom\n", which);

	lock_acquire(BrLock);
	V(BrSem);
	if(BrSem->sem_count==3)
	{
		mode=0;
		if(boy_count!=0)
		{
			mode=1;
			cv_broadcast(boyCV,BrLock);
		}
		else
		{
			if(girl_count==0)
				mode=0;
			else
			{
				mode=2;
				cv_broadcast(girlCV,BrLock);
			}
		}
	}
	lock_release(BrLock);
	V(doneSem);
}

// Change this function as necessary
int
bathroom(int nargs, char **args)
{

	int i, err=0;

	(void)nargs;
	(void)args;

	inititems();
	for (i = 0; i < NPEOPLE; i++) {
		/*
		err = thread_fork("Boy Thread", NULL,
		  boy, NULL, i);
		  */
		
		switch(i % 2) {
			case 0:
			err = thread_fork("Boy Thread", NULL,
					  boy, NULL, i);
			break;
			case 1:
			err = thread_fork("Girl Thread", NULL,
					  girl, NULL, i);
			break;
			}
			
		if (err) {
			panic("bathroom: thread_fork failed: %s)\n",
				strerror(err));
		}
	}

	for(i=0;i<NPEOPLE;i++)
		P(doneSem);

	return 0;
}

