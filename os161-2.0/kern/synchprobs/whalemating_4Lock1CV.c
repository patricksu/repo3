/*
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

/*
 * Driver code for whale mating problem
 */
#include <types.h>
#include <lib.h>
#include <thread.h>
#include <test.h>
 #include <synch.h>

#define NMATING 10
#define DefaultV 0
//Peng 2.23.2016
static volatile unsigned long male_ID=DefaultV;
static volatile unsigned long female_ID=DefaultV;
static volatile unsigned long mtchmkr_ID=DefaultV;

//static struct lock *whaleLock;
static struct lock *maleLock;
static struct lock *femaleLock;
static struct lock *mtchmkrLock;

static struct lock *mateLock;

static struct cv *mateCV;

static struct semaphore *mateSem;
static struct semaphore *doneSem;
//Peng

static
void
initiateMating()
{
	if (mateCV==NULL) {
		mateCV = cv_create("mateCV");
		if (mateCV == NULL) {
			panic("mateCV: cv_create failed\n");
		}
	}

	if (maleLock==NULL) {
		maleLock = lock_create("maleLock");
		if (maleLock == NULL) {
			panic("maleLock: lock_create failed\n");
		}
	}
	if (femaleLock==NULL) {
		femaleLock = lock_create("femaleLock");
		if (femaleLock == NULL) {
			panic("femaleLock: lock_create failed\n");
		}
	}
	if (mtchmkrLock==NULL) {
		mtchmkrLock = lock_create("mtchmkrLock");
		if (mtchmkrLock == NULL) {
			panic("mtchmkrLock: lock_create failed\n");
		}
	}
	if (mateLock==NULL) {
		mateLock = lock_create("mateLock");
		if (mateLock == NULL) {
			panic("mateLock: lock_create failed\n");
		}
	}
	if (doneSem==NULL) {
		doneSem = sem_create("doneSem", 0);
		if (doneSem == NULL) {
			panic("doneSem: sem_create failed\n");
		}
	}
	if (mateSem==NULL) {
		mateSem = sem_create("mateSem", 3);
		if (mateSem == NULL) {
			panic("mateSem: sem_create failed\n");
		}
	}
}

static
void
male(void *p, unsigned long which)
{
	(void)p;
	kprintf("male whale #%ld starting\n", which);
	// Implement this function
	//Peng 2.23.2016
	lock_acquire(maleLock);
	P(mateSem);
	male_ID=which;	

	lock_acquire(mateLock);
	if(mateSem->sem_count!=0)
	{
		//while(mateSem->sem_count!=0)
			cv_wait(mateCV,mateLock);
	}
	else
	{
		cv_broadcast(mateCV,mateLock);
	}
	lock_release(mateLock);

	kprintf("male %lu is mating with female %lu under mtchmkr %lu\n",male_ID,female_ID,mtchmkr_ID);

	V(mateSem);

	lock_acquire(mateLock);	
	if(mateSem->sem_count!=3)
	{
		//while(mateSem->sem_count!=3)
			cv_wait(mateCV,mateLock);
	}
	else
	{
		cv_broadcast(mateCV,mateLock);
	}
	lock_release(mateLock);

	lock_release(maleLock);
	V(doneSem);
	//Peng
}

static
void
female(void *p, unsigned long which)
{
	(void)p;
	kprintf("female whale #%ld starting\n", which);

	// Implement this function
	//Peng 2.23.2016
	lock_acquire(femaleLock);
	P(mateSem);
	female_ID=which;	
	
	lock_acquire(mateLock);
	if(mateSem->sem_count!=0)
	{
		//while(mateSem->sem_count!=0)
			cv_wait(mateCV,mateLock);
	}
	else
	{
		cv_broadcast(mateCV,mateLock);
	}
	lock_release(mateLock);

	kprintf("female %lu is mating with male %lu under mtchmkr %lu\n",female_ID,male_ID,mtchmkr_ID);

	V(mateSem);

	lock_acquire(mateLock);	
	if(mateSem->sem_count!=3)
	{
		//while(mateSem->sem_count!=3)
			cv_wait(mateCV,mateLock);
	}
	else
	{
		cv_broadcast(mateCV,mateLock);
	}
	lock_release(mateLock);

	lock_release(femaleLock);
	V(doneSem);
	//Peng
}

static
void
matchmaker(void *p, unsigned long which)
{
	(void)p;
	kprintf("matchmaker whale #%ld starting\n", which);

	// Implement this function
	//Peng 2.23.2016
	lock_acquire(mtchmkrLock);
	P(mateSem);
	mtchmkr_ID=which;	
	
	lock_acquire(mateLock);
	if(mateSem->sem_count!=0)
	{
		//while(mateSem->sem_count!=0)
			cv_wait(mateCV,mateLock);
	}
	else
	{
		cv_broadcast(mateCV,mateLock);
	}
	lock_release(mateLock);

	kprintf("matchmaker %lu is helping male %lu and female %lu to mate\n",mtchmkr_ID,male_ID,female_ID);

	V(mateSem);

	lock_acquire(mateLock);	
	if(mateSem->sem_count!=3)
	{
		//while(mateSem->sem_count!=3)
			cv_wait(mateCV,mateLock);
	}
	else
	{
		cv_broadcast(mateCV,mateLock);
	}
	lock_release(mateLock);

	lock_release(mtchmkrLock);
	V(doneSem);
	//Peng
}


// Change this function as necessary
int
whalemating(int nargs, char **args)
{

	int i, j, err=0;

	(void)nargs;
	(void)args;

	initiateMating();

	for (i = 0; i < 3; i++) {
		for (j = 0; j < NMATING; j++) {
			switch(i) {
			    case 0:
				err = thread_fork("Male Whale Thread",
						  NULL, male, NULL, j);
				break;
			    case 1:
				err = thread_fork("Female Whale Thread",
						  NULL, female, NULL, j);
				break;
			    case 2:
				err = thread_fork("Matchmaker Whale Thread",
						  NULL, matchmaker, NULL, j);
				break;
			}
			if (err) {
				panic("whalemating: thread_fork failed: %s)\n",
				      strerror(err));
			}
		}
	}

	for (i=0; i<3*NMATING; i++) {
		P(doneSem);
	}
	return 0;
}
