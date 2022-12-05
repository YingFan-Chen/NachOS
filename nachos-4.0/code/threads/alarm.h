// alarm.h 
//	Data structures for a software alarm clock.
//
//	We make use of a hardware timer device, that generates
//	an interrupt every X time ticks (on real systems, X is
//	usually between 0.25 - 10 milliseconds).
//
//	From this, we provide the ability for a thread to be
//	woken up after a delay; we also provide time-slicing.
//
//	NOTE: this abstraction is not completely implemented.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ALARM_H
#define ALARM_H

#include "copyright.h"
#include "utility.h"
#include "callback.h"
#include "timer.h"
#include "thread.h"
#include <queue>
#include <vector>

// The following class defines a software alarm clock. 
class Alarm : public CallBackObj {
  public:
    Alarm(bool doRandomYield);	// Initialize the timer, and callback 
				// to "toCall" every time slice.
    ~Alarm() { delete timer; }
    
    void WaitUntil(int x);	// suspend execution until time > now + x

  private:
    Timer *timer;		// the hardware timer device
    unsigned int current;       // current time stamp (increment when Callback) 				// (r11921111 edit)

    void CallBack();		// called when the hardware
				// timer generates an interrupt

    // structure for recording sleeping thread (r11921111 edit)
    class sleeping_thread{
	public:
		unsigned int wakeup_time;
		Thread* t;
		sleeping_thread(unsigned int x, Thread* thread):
			wakeup_time(x), t(thread) {}
    };
    
    // this is for priority_queue comparison (r11921111 edit)
    struct cmp{
	bool operator() (sleeping_thread &a, sleeping_thread &b){
		return a.wakeup_time > b.wakeup_time;	
	}
    };
    
    // use priority_queue to maintain the sleeping thread (r11921111 edit)
    std::priority_queue<sleeping_thread, std::vector<sleeping_thread>, cmp> 	  sleeping_list;
};

#endif // ALARM_H
