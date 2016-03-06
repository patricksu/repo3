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

#include <types.h>
#include <lib.h>
#include <clock.h>
#include <thread.h>
#include <test.h>
#include <synch.h>
#include <wchan.h>

#define NPEOPLE 20

#define BOYS 1
#define GIRLS 2
#define FREE 0


static struct lock *bath_lock;

static struct cv *boy_cv;
static struct cv *girl_cv;

static struct semaphore *bath_sem;
static struct semaphore *finish_sem;

static volatile int bath_state = FREE;
static volatile int last_state = FREE;

static volatile int boy_count = 0;
static volatile int girl_count = 0;

static
void
initlocks(void)
{
	//Init lock for both boy and gril threads
		bath_lock = lock_create("bath_lock");
		if (bath_lock == NULL) {
			panic("bath_lock create failed\n");
		}

	//Init lock for boy threads
		boy_cv = cv_create("boy_cv");
		if (boy_cv == NULL) {
			panic("boy_cv reate failed\n");
		}
	//Init lock for girl threads
		girl_cv = cv_create("girl_cv");
		if (girl_cv == NULL) {
			panic("boy_cv create failed\n");
		}

	//Init sem for mate (wait for all three whales(male, female, matchmaker) ready to mate)
		bath_sem = sem_create("bath_sem", 3);
		if (bath_sem == NULL) {
			panic("bath_sem create failed\n");
		}
	//Init sem for finish (Record number of thread that already finished)
		finish_sem = sem_create("finish_sem", 0);
		if (finish_sem == NULL) {
			panic("finish_sem create failed\n");
		}

}


static
void
shower()
{
	// The thread enjoys a refreshing shower!
        clocksleep(1);
}

static
void
boy(void *p, unsigned long which)
{
	(void)p;
	kprintf("boy #%ld starting\n", which);

	lock_acquire(bath_lock);
		//if there's grils in bath or no room for bath, wait in boy_cv wait channel
		while(bath_state == GIRLS || bath_sem->sem_count == 0 || last_state == BOYS){
			boy_count++;
			cv_wait(boy_cv, bath_lock);
			boy_count--;
		}
		P(bath_sem);
		bath_state = BOYS;
	lock_release(bath_lock);

	// use bathroom
	kprintf("boy #%ld entering bathroom...\n", which);
	shower();
	kprintf("boy #%ld leaving bathroom\n", which);
   if(girl_count > 0)last_state = BOYS;
	V(bath_sem);

	lock_acquire(bath_lock);
      if(girl_count == 0)cv_broadcast(boy_cv,bath_lock);
		else if(bath_sem->sem_count == 3){
			bath_state = GIRLS;
			cv_broadcast(girl_cv,bath_lock);
		}
	lock_release(bath_lock);

	V(finish_sem);
}

static
void
girl(void *p, unsigned long which)
{
	(void)p;
	kprintf("girl #%ld starting\n", which);

	// Implement this function

	lock_acquire(bath_lock);

		while(bath_state == BOYS || bath_sem->sem_count == 0 || (last_state == GIRLS && boy_count > 0)){//if there's grils in bath or no room for bath, wait in boy_cv wait channel
			girl_count++;
			cv_wait(girl_cv, bath_lock);
			girl_count--;
		}
		P(bath_sem);
		bath_state = GIRLS;
	lock_release(bath_lock);


	// use bathroom
	kprintf("girl #%ld entering bathroom\n", which);
	shower();
	kprintf("girl #%ld leaving bathroom\n", which);
	if(boy_count > 0)last_state = GIRLS;
	V(bath_sem);

	lock_acquire(bath_lock);
      if(boy_count == 0)cv_broadcast(girl_cv,bath_lock);
		else if(bath_sem->sem_count == 3){
			bath_state = BOYS;
			cv_broadcast(boy_cv,bath_lock);
		}
	lock_release(bath_lock);


	V(finish_sem);
}

// Change this function as necessary
int
bathroom(int nargs, char **args)
{

	int i, err=0;

	(void)nargs;
	(void)args;
   
	initlocks();

	for (i = 0; i < NPEOPLE; i++) {
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

	//waiting for all thread finish.
	for (i=0; i<NPEOPLE; i++) {
		P(finish_sem);
	}

	kprintf("All boys and girls have finished bathing!!!\n");
   
	return 0;
}

