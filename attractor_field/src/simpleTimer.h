//
//  simpleTimer.h
//  attractor_field9
//
//  Created by Peter Werner on 10/01/13.
//
//

#ifndef attractor_field9_simpleTimer_h
#define attractor_field9_simpleTimer_h

#include <sys/time.h>

class simpleTimer {
    
public:
    simpleTimer() {
        started = false;
        stopped = false;
    }
    void start() {
        started = true;
        gettimeofday(&t1, NULL);
    }
    
    void stop() {
        gettimeofday(&t2, NULL);
        stopped = true;
    }
    
    long getMillis(struct timeval *t)
    {
        return((t->tv_sec) * 1000 + (t->tv_usec/1000.0));
    }
    
    double diff() {
        if (!started)
            return 0;
        
        if (!stopped)
            gettimeofday(&t2, NULL);
        
        double el = (t2.tv_sec - t1.tv_sec) * 1000.0;
        el += (t2.tv_usec - t1.tv_usec) / 1000.0;
        return(el);
    }
    
    void reset() {
        started = false;
        stopped = false;
        bzero(&t1, sizeof(struct timeval));
        bzero(&t2, sizeof(struct timeval));
    }
    
    bool isRunning() {
        return(started == true && stopped == false);
    }
    
private:
    timeval t1;
    timeval t2;
    
    bool started;
    bool stopped;
    
};

#endif
