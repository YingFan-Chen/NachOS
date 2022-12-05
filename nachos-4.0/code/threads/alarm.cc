// alarm.cc
//	Routines to use a hardware timer device to provide a
//	software alarm clock.  For now, we just provide time-slicing.
//
//	Not completely implemented.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "alarm.h"
#include "main.h"
#include "thread.h"

//----------------------------------------------------------------------
// Alarm::Alarm
//      Initialize a software alarm clock.  Start up a timer device
//
//      "doRandom" -- if true, arrange for the hardware interrupts to 
//		occur at random, instead of fixed, intervals.
//----------------------------------------------------------------------

Alarm::Alarm(bool doRandom) : current(0)
{  
	timer = new Timer(doRandom, this);
}

//----------------------------------------------------------------------
// Alarm::CallBack
//	Software interrupt handler for the timer device. The timer device is
//	set up to interrupt the CPU periodically (once every TimerTicks).
//	This routine is called each time there is a timer interrupt,
//	with interrupts disabled.
//
//	Note that instead of calling Yield() directly (which would
//	suspend the interrupt handler, not the interrupted thread
//	which is what we wanted to context switch), we set a flag
//	so that once the interrupt handler is done, it will appear as 
//	if the interrupted thread called Yield at the point it is 
//	was interrupted.
//
//	For now, just provide time-slicing.  Only need to time slice 
//      if we're currently running something (in other words, not idle).
//	Also, to keep from looping forever, we check if there's
//	nothing on the ready list, and there are no other pending
//	interrupts.  In this case, we can safely halt.
//----------------------------------------------------------------------

void 
Alarm::CallBack() 
{
    Interrupt *interrupt = kernel->interrupt;
    MachineStatus status = interrupt->getStatus();
    current ++;
	
    // wake up the thread which is time to wake up (r11921111 edit)
    bool flag = true;
    while(sleeping_list.size()){
	if(sleeping_list.top().wakeup_time <= current){
		// Put the thread back to ready queue (r11921111 edit)
		kernel->scheduler->ReadyToRun(sleeping_list.top().t);
		sleeping_list.pop();
		flag = false;
	}else break;
    }    

    if (status == IdleMode && sleeping_list.empty() && flag) {	// is it time to quit?
        if (!interrupt->AnyFutureInterrupts()) {
	    timer->Disable();	// turn off the timer
	}
    } else {			// there's someone to preempt
	// Only RR will preempt (r11921111 edit)	
	if (kernel->scheduler->get_schedulerType() == RR){
	    interrupt->YieldOnReturn();	
	    printf("========== RR Yielding ==========\n");
	}	
    }
}

void 
Alarm::WaitUntil(int x){
	// Enter Critical Section (r11921111 edit)
	IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
	Thread* thread = kernel->currentThread;
	sleeping_list.push(sleeping_thread{current + x, thread});	
	thread->Sleep(false);	// Block the thread (r11921111 edit)
	kernel->interrupt->SetLevel(oldLevel);
	// Exit Critical Section (r11921111 edit)
}

