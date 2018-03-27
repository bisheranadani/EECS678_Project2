/** @file libscheduler.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libscheduler.h"
#include "../libpriqueue/libpriqueue.h"


/**
  Stores information making up a job to be scheduled including any statistics.

  You may need to define some global variables or a struct to store your job queue elements.
*/
// typedef struct _core_t
// {
// 	bool is_busy;
// 	job_t* current_job;
//
// } core_t;

typedef struct _job_t
{
	int id;
	int arrival_time;
	int used_time;
	int remaining_time;
	int needed_time;
	int last_start_time;
	int time_to_schedule;
	int priority;
} job_t;

typedef struct _scheduler_t
{
	int total_jobs;
	int num_cores;
	float total_wait;
	float total_turnaround;
	float total_response;
	scheme_t scheme;
	priqueue_t* priqueue;
	job_t** core_array;

} scheduler_t;

scheduler_t* scheduler;
/**
  Initalizes the scheduler.

  Assumptions:
    - You may assume this will be the first scheduler function called.
    - You may assume this function will be called once once.
    - You may assume that cores is a positive, non-zero number.
    - You may assume that scheme is a valid scheduling scheme.

  @param cores the number of cores that is available by the scheduler. These cores will be known as core(id=0), core(id=1), ..., core(id=cores-1).
  @param scheme  the scheduling scheme that should be used. This value will be one of the six enum values of scheme_t
*/

int fcfs_compare(const void* x, const void* y){
	job_t* job1 = (job_t*) x;
	job_t* job2 = (job_t*) y;

	return job1->arrival_time - job2->arrival_time;
}

int rr_compare(const void* x, const void* y){
	return 1;
}

int sjf_compare(const void* x, const void* y){
	job_t* job1 = (job_t*) x;
	job_t* job2 = (job_t*) y;
	int to_return = job1->remaining_time - job2->remaining_time;

	if(to_return == 0){
		return job1->arrival_time - job2->arrival_time;
	}
	else{
		return to_return;
	}
}

int pri_compare(const void* x, const void* y){
	job_t* job1 = (job_t*) x;
	job_t* job2 = (job_t*) y;
	int to_return = job1->priority - job2->priority;

	if(to_return == 0){
		return job1->arrival_time - job2->arrival_time;
	}
	else{
		return to_return;
	}

}

void scheduler_start_up(int cores, scheme_t scheme)
{
	scheduler = malloc(sizeof(scheduler_t));
	scheduler->total_jobs = 0;
	scheduler->num_cores = cores;
	scheduler->total_wait = 0;
	scheduler->total_turnaround = 0;
	scheduler->total_response = 0;
	scheduler->scheme = scheme;
	scheduler->priqueue = malloc(sizeof(priqueue_t));
	scheduler->core_array = (job_t**) calloc(cores, sizeof(job_t*));

	for (int i = 0; i < scheduler->num_cores; i++)
	{
		scheduler->core_array[i] = malloc(sizeof(job_t));
		scheduler->core_array[i] = NULL;
	}

	if(scheme == FCFS){
		priqueue_init(scheduler->priqueue, fcfs_compare);
	}
	else if(scheme == RR){
		priqueue_init(scheduler->priqueue, rr_compare);
	}
	else if(scheme == SJF || scheme == PSJF){
		priqueue_init(scheduler->priqueue, sjf_compare);
	}
	else if(scheme == PRI || scheme || PPRI){
		priqueue_init(scheduler->priqueue, pri_compare);
	}
	else{
		priqueue_init(scheduler->priqueue, fcfs_compare);
	}

}


void updateRemainingTimes(int time){
	for(int i=0; i < scheduler->num_cores; i++){
		if(scheduler->core_array[i] != NULL){
			scheduler->core_array[i]->used_time = scheduler->core_array[i]->used_time + (time - scheduler->core_array[i]->last_start_time);
			scheduler->core_array[i]->remaining_time = scheduler->core_array[i]->needed_time - scheduler->core_array[i]->used_time;
			scheduler->core_array[i]->last_start_time = time;
		}
	}
}

/**
  Called when a new job arrives.

  If multiple cores are idle, the job should be assigned to the core with the
  lowest id.
  If the job arriving should be scheduled to run during the next
  time cycle, return the zero-based index of the core the job should be
  scheduled on. If another job is already running on the core specified,
  this will preempt the currently running job.
  Assumptions:
    - You may assume that every job wil have a unique arrival time.

  @param job_number a globally unique identification number of the job arriving.
  @param time the current time of the simulator.
  @param running_time the total number of time units this job will run before it will be finished.
  @param priority the priority of the job. (The lower the value, the higher the priority.)
  @return index of core job should be scheduled on
  @return -1 if no scheduling changes should be made.

 */



int scheduler_new_job(int job_number, int time, int running_time, int priority)
{
    job_t* new_job = malloc(sizeof(job_t));
    new_job->id = job_number;
    new_job->arrival_time = time;
    new_job->used_time = 0;
		new_job->remaining_time = running_time;
		new_job->needed_time = running_time;
    new_job->last_start_time = 0;
    new_job->time_to_schedule = 0;
		new_job->priority = priority;

		int core = -1;
		for(int i=0; i < scheduler->num_cores; i++){
			if(scheduler->core_array[i] == NULL){
				core = i;
				break;
			}
		}

		updateRemainingTimes(time);

		if(core != -1)
    {
				new_job->last_start_time = time;
				// new_job->time_to_schedule = 0;
        scheduler->core_array[core] = new_job;
        return core;
    }

		if(scheduler->scheme == PSJF){
			//find job with longest remaining time
			core = 0;
			for(int i=0; i<scheduler->num_cores; i++){
				if(scheduler->core_array[i] != NULL){
					if(scheduler->core_array[i]->remaining_time > scheduler->core_array[core]->remaining_time){
						core = i;
					}
					else if(scheduler->core_array[i]->remaining_time == scheduler->core_array[core]->remaining_time){
						if(scheduler->core_array[i]->arrival_time > scheduler->core_array[core]->remaining_time){
							core = i;
						}
					}
				}
			}

			if(scheduler->core_array[core]->remaining_time <= new_job->remaining_time){
				priqueue_offer(scheduler->priqueue, new_job);
				return -1;
			}

			new_job->last_start_time = time;
			job_t* old_job = scheduler->core_array[core];
			priqueue_offer(scheduler->priqueue, old_job);
			scheduler->core_array[core] = NULL;
			scheduler->core_array[core] = new_job;

			return core;
		}

		else if(scheduler->scheme == PPRI){
			core = 0;
			for(int i=0; i<scheduler->num_cores; i++){
				if(scheduler->core_array[i] != NULL){
					if(scheduler->core_array[i]->priority > scheduler->core_array[core]->priority){
						core = i;
					}
					else if(scheduler->core_array[i]->priority == scheduler->core_array[core]->priority){
						if(scheduler->core_array[i]->arrival_time > scheduler->core_array[core]->remaining_time){
							core = i;
						}
					}
				}
			}

			if(scheduler->core_array[core]->priority <= new_job->priority){
				priqueue_offer(scheduler->priqueue, new_job);
				return -1;
			}

			new_job->last_start_time = time;
			job_t* old_job = scheduler->core_array[core];
			priqueue_offer(scheduler->priqueue, old_job);
			scheduler->core_array[core] = NULL;
			scheduler->core_array[core] = new_job;
			return core;
		}
		else if(scheduler->scheme == FCFS || scheduler->scheme == RR || scheduler->scheme == PRI || scheduler->scheme == SJF ){
			priqueue_offer(scheduler->priqueue, new_job);
			return -1;
		}

		else{
			return -1;
		}

}


/**
  Called when a job has completed execution.

  The core_id, job_number and time parameters are provided for convenience. You may be able to calculate the values with your own data structure.
  If any job should be scheduled to run on the core free'd up by the
  finished job, return the job_number of the job that should be scheduled to
  run on core core_id.

  @param core_id the zero-based index of the core where the job was located.
  @param job_number a globally unique identification number of the job.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled to run on core core_id
  @return -1 if core should remain idle.
 */
int scheduler_job_finished(int core_id, int job_number, int time)
{
	job_t* old_job = scheduler->core_array[core_id];
	scheduler->total_jobs = scheduler->total_jobs +1;
	int temp = (time - old_job->arrival_time) - old_job->needed_time;
	scheduler->total_wait = scheduler->total_wait + temp;
	temp = time - old_job->arrival_time;
	scheduler->total_turnaround = scheduler->total_turnaround + temp;
	// temp = old_job->time_to_schedule;
	scheduler->total_response = scheduler->total_response + old_job->time_to_schedule;

	scheduler->core_array[core_id] = NULL;
	free(old_job);

	job_t* new_job = priqueue_poll(scheduler->priqueue);
	if(new_job == NULL){
		return -1;
	}
	else{
		if(new_job->used_time == 0){
			new_job->time_to_schedule = time - new_job->arrival_time;
		}
		scheduler->core_array[core_id] = new_job;
		new_job->last_start_time = time;

		return new_job->id;
	}

}


/**
  When the scheme is set to RR, called when the quantum timer has expired
  on a core.

  If any job should be scheduled to run on the core free'd up by
  the quantum expiration, return the job_number of the job that should be
  scheduled to run on core core_id.

  @param core_id the zero-based index of the core where the quantum has expired.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled on core cord_id
  @return -1 if core should remain idle
 */
int scheduler_quantum_expired(int core_id, int time)
{
	updateRemainingTimes(time);
	job_t* old_job = scheduler->core_array[core_id];
	priqueue_offer(scheduler->priqueue, old_job);
	scheduler->core_array[core_id] = NULL;

	job_t* new_job = priqueue_poll(scheduler->priqueue);
	if(new_job == NULL){
		return -1;
	}
	else{
		if(new_job->used_time == 0){
			new_job->time_to_schedule = time - new_job->arrival_time;
		}
		scheduler->core_array[core_id] = new_job;
		new_job->last_start_time = time;

		return new_job->id;
	}
}


/**
  Returns the average waiting time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average waiting time of all jobs scheduled.
 */
float scheduler_average_waiting_time()
{
	float to_return = 0;
	if(scheduler->total_jobs == 0){
		//to_return = 0;
	}
	else{
		to_return = scheduler->total_wait / scheduler->total_jobs;
	}

	return to_return;
}


/**
  Returns the average turnaround time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average turnaround time of all jobs scheduled.
 */
float scheduler_average_turnaround_time()
{
	float to_return = 0;
	if(scheduler->total_jobs == 0){
		//to_return = 0;
	}
	else{
		to_return = scheduler->total_turnaround / scheduler->total_jobs;
	}

	return to_return;
}


/**
  Returns the average response time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average response time of all jobs scheduled.
 */
float scheduler_average_response_time()
{
	float to_return = 0;
	if(scheduler->total_jobs == 0){
		//to_return = 0;
	}
	else{
		to_return = scheduler->total_response / scheduler->total_jobs;
	}

	return to_return;
}


/**
  Free any memory associated with your scheduler.

  Assumptions:
    - This function will be the last function called in your library.
*/
void scheduler_clean_up()
{
	priqueue_destroy(scheduler->priqueue);
	free(scheduler->core_array);
	free(scheduler->priqueue);
	free(scheduler);
}


/**
  This function may print out any debugging information you choose. This
  function will be called by the simulator after every call the simulator
  makes to your scheduler.
  In our provided output, we have implemented this function to list the jobs in the order they are to be scheduled. Furthermore, we have also listed the current state of the job (either running on a given core or idle). For example, if we have a non-preemptive algorithm and job(id=4) has began running, job(id=2) arrives with a higher priority, and job(id=1) arrives with a lower priority, the output in our sample output will be:

    2(-1) 4(0) 1(-1)

  This function is not required and will not be graded. You may leave it
  blank if you do not find it useful.
 */
void scheduler_show_queue()
{
	// int pri = -1;
	// for(int i=0; i<scheduler->num_cores; i++){
	// 	pri = -1;
	// 	if(scheduler->core_array[i] != NULL){
	// 		pri = scheduler->core_array[i]->priority;
	// 	}
	// 	printf("Core: %d - Job priority: %d \n", i, pri);
	// }
	job_t* temp = NULL;
	for(int i=0; i<priqueue_size(scheduler->priqueue); i++){
		temp = priqueue_at(scheduler->priqueue, i);
		printf(" (%d)%d ", temp->id, temp->priority);
	}


}
