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
#include <wchan.h>

#define NMATING 10

static struct cv *male_cv;
static struct cv *female_cv;
static struct cv *matchmaker_cv;

static struct lock *lk_lock;

static struct semaphore *finish_sem;

static volatile int male_count = 0;
static volatile int female_count = 0;
static volatile int matchmaker_count = 0;

static
void
initlocks(void)
{
	//Init lock for male threads
		lk_lock = lock_create("lk_lock");
		if (lk_lock == NULL) {
			panic("lk_lock create failed\n");
		}


	//Init cv for the incoming male
		male_cv = cv_create("male_cv");
		if (male_cv == NULL) {
			panic("male_cv create failed\n");
		}
	//Init cv for the incoming female
		female_cv = cv_create("female_cv");
		if (female_cv == NULL) {
			panic("female_cv create failed\n");
		}
	//Init cv for the incoming matchmaker
		matchmaker_cv = cv_create("matchmaker_cv");
		if (matchmaker_cv == NULL) {
			panic("matchmaker_cv create failed\n");
		}

	//Init sem for finish (Record number of thread that already finished)
		finish_sem = sem_create("finish_sem", 0);
		if (finish_sem == NULL) {
			panic("finish_sem create failed\n");
		}

}

static
void
male(void *p, unsigned long which)
{
	(void)p;
	kprintf("male whale #%ld starting\n", which);

	lock_acquire(lk_lock);
			if(female_count == 0 || matchmaker_count == 0)
			{//check if not all 3 are ready, then wait in waitchannel
				male_count++;
				cv_wait(male_cv, lk_lock);
			}
			else
			{//all 3 are ready, pull out a group 
				cv_signal(female_cv, lk_lock);
				female_count--;
				cv_signal(matchmaker_cv, lk_lock);
				matchmaker_count--;
			}

	lock_release(lk_lock);
	kprintf("male whale #%ld finished mating!!!!!\n", which);
	V(finish_sem);
}

static
void
female(void *p, unsigned long which)
{
	(void)p;
	kprintf("female whale #%ld starting\n", which);

	lock_acquire(lk_lock);
				if(male_count == 0 || matchmaker_count == 0){//check if not all 3 are ready, then wait in waitchannel
				female_count++;
				cv_wait(female_cv, lk_lock);
			}
			else{//all 3 are ready, pull out a group 
				cv_signal(male_cv, lk_lock);
				male_count--;
				cv_signal(matchmaker_cv, lk_lock);
				matchmaker_count--;
			}

	lock_release(lk_lock);
	kprintf("female whale #%ld finished mating!!!!!\n", which);
	V(finish_sem);
}

static
void
matchmaker(void *p, unsigned long which)
{
	(void)p;
	kprintf("matchmaker whale #%ld starting\n", which);

	lock_acquire(lk_lock);
			if(male_count == 0 || female_count == 0){//check if not all 3 are ready, then wait in waitchannel
				matchmaker_count++;
				cv_wait(matchmaker_cv, lk_lock);
			}
			else{//all 3 are ready, pull out a group 
				cv_signal(male_cv, lk_lock);
				male_count--;
				cv_signal(female_cv, lk_lock);
				female_count--;
			}

	lock_release(lk_lock);
	kprintf("match maker whale #%ld finished helping\n", which);
	V(finish_sem);
}


// Change this function as necessary
int
whalemating(int nargs, char **args)
{

	int i, j, err=0;

	(void)nargs;
	(void)args;

	initlocks();

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


	//waiting for all thread finish.
	for (i=0; i<3*NMATING; i++) {
		P(finish_sem);
	}

	kprintf("All whales have finished mating!!!\n");


	return 0;
}
