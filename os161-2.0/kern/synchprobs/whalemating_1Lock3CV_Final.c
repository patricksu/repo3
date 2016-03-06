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

//Peng 2.23.2016

static volatile unsigned int nmale=0;
static volatile unsigned int nfemale=0;
static volatile unsigned int nmatchmaker=0;

static struct lock *mateLock;

static struct cv *male_go;
static struct cv *female_go;
static struct cv *matchmaker_go;

static struct semaphore *doneSem;
//Peng

static
void
initiateMating()
{
	if (mateLock==NULL) {
		mateLock = lock_create("mateLock");
		if (mateLock == NULL) {
			panic("mateLock: lock_create failed\n");
		}
	}
	if (male_go==NULL) {
		male_go = cv_create("male_go");
		if (male_go == NULL) {
			panic("male_go: cv_create failed\n");
		}
	}
	if (female_go==NULL) {
		female_go = cv_create("female_go");
		if (female_go == NULL) {
			panic("female_go: cv_create failed\n");
		}
	}
	if (matchmaker_go==NULL) {
		matchmaker_go = cv_create("matchmaker_go");
		if (matchmaker_go == NULL) {
			panic("matchmaker_go: cv_create failed\n");
		}
	}
	if (doneSem==NULL) {
		doneSem = sem_create("doneSem", 0);
		if (doneSem == NULL) {
			panic("doneSem: sem_create failed\n");
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
	lock_acquire(mateLock);	
	if(nfemale<1 || nmatchmaker<1)
	{
		nmale++;
		cv_wait(male_go,mateLock);
	}
	else
	{
		cv_signal(female_go,mateLock);
		nfemale--;
		cv_signal(matchmaker_go,mateLock);
		nmatchmaker--;
	}
	
	lock_release(mateLock);
	kprintf("male %lu finished mating\n",which);
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
	lock_acquire(mateLock);	
	if(nmale<1 || nmatchmaker<1)
	{
		nfemale++;
		cv_wait(female_go,mateLock);
	}
	else
	{
		cv_signal(male_go,mateLock);
		nmale--;
		cv_signal(matchmaker_go,mateLock);
		nmatchmaker--;
	}
	
	lock_release(mateLock);
	kprintf("female %lu finished mating\n",which);
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
	lock_acquire(mateLock);	
	if(nmale<1 || nfemale<1)
	{
		nmatchmaker++;
		cv_wait(matchmaker_go,mateLock);
	}
	else
	{
		cv_signal(male_go,mateLock);
		nmale--;
		cv_signal(female_go,mateLock);
		nfemale--;
	}
	
	lock_release(mateLock);
	kprintf("matchmaker %lu finished mating\n",which);
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

	nmale=0;
	nfemale=0;
	nmatchmaker=0;
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
	for (i=0;i<3*NMATING;i++) {
		P(doneSem);
	}

	return 0;
}
