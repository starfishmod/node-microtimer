#include <iostream>
#include <chrono>
#include <random>
#include <thread>
#include "streaming-worker.h"
//#include "json.hpp"  //https://github.com/nlohmann/json

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/timerfd.h>

using namespace std;
//using json = nlohmann::json;

struct periodic_info {
	int timer_fd;
	//unsigned long long wakeups_missed;
};

static int make_periodic(unsigned int period, struct periodic_info *info)
{
	int ret;
	unsigned int ns;
	unsigned int sec;
	int fd;
	struct itimerspec itval;

	/* Create the timer */
	fd = timerfd_create(CLOCK_MONOTONIC, 0);
	//info->wakeups_missed = 0;
	info->timer_fd = fd;
	if (fd == -1)
		return fd;

	/* Make the timer periodic */
	sec = period / 1000000;
	ns = (period - (sec * 1000000)) * 1000;
	itval.it_interval.tv_sec = sec;
	itval.it_interval.tv_nsec = ns;
	itval.it_value.tv_sec = sec;
	itval.it_value.tv_nsec = ns;
	ret = timerfd_settime(fd, 0, &itval, NULL);
	return ret;
}

static void wait_period(struct periodic_info *info)
{
	unsigned long long missed;
	int ret;

	/* Wait for the next timer event. If we have missed any the
	   number is written to "missed" */
	ret = read(info->timer_fd, &missed, sizeof(missed));
	if (ret == -1) {
		//perror("read timer");
		return;
	}

	//info->wakeups_missed += missed;
}


class MicroTimer : public StreamingWorker {
  public:
    MicroTimer(Callback *data, Callback *complete, Callback *error_callback,  v8::Local<v8::Object> & options) 
          : StreamingWorker(data, complete, error_callback){

      period = 0;
        if (options->IsObject() ) {
          v8::Local<v8::Value> n_ = options->Get(New<v8::String>("period").ToLocalChecked());
          if ( n_->IsNumber() ) {
            period = n_->NumberValue();
          }
        }

        if ( period == 0 ) {
          SetErrorMessage("Period must be an int > 0");
          return;
        }

		make_periodic(period, &info);
    }
     
    void send_sample(const AsyncProgressWorker::ExecutionProgress& progress) {
        Message tosend("period", std::to_string(period));
        writeToNode(progress, tosend);
    }

    void Execute (const AsyncProgressWorker::ExecutionProgress& progress) {
      
      while (!closed()) {
		wait_period(&info); 
        send_sample(progress);
      }
    }
  private:
    unsigned int period;
    struct periodic_info info;
};

StreamingWorker * create_worker(Callback *data
    , Callback *complete
    , Callback *error_callback, v8::Local<v8::Object> & options) {
 return new MicroTimer(data, complete, error_callback, options);
}

NODE_MODULE(micro_timer, StreamWorkerWrapper::Init)
