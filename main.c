#define _GNU_SOURCE

#include <sys/syscall.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>

#include <unistd.h>
#include <sched.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define Round_rubin_time 500
typedef struct Process{
	char name[10];
	int ready_time;
	int real_start_time;
	int execution_time;
	pid_t pid; 
}Process;

Process P[1005];
int atime = 0;


int compare_function(const void *a, const void *b){
	Process *first = (Process *)a;
	Process *second = (Process *)b;
	return first -> ready_time > second -> ready_time;
}

void a_second(){
	volatile unsigned long i; for(i=0;i<1000000UL;i++);
}

void scheduler_mask_initialize(cpu_set_t* sheduler_mask){
	CPU_ZERO(sheduler_mask);
	CPU_SET(0, sheduler_mask);
	return;
}

void process_mask_initialize(cpu_set_t* process_mask){
	CPU_ZERO(process_mask);
	CPU_SET(1, process_mask);
	return;
}
int queue[1000];
int first = 0, last = 0;
void queue_pop(){
	queue[first] = 0;
	first++;
	return;
}
void queue_push(int a){
	queue[last] = a;
	last++;
}
int next(int now_process, int N, int method){
	if(method == 0){
		// FIFO
		if(now_process != -1) // The now_process hasnʼt finished yet.
			return now_process;
		static int i = 0;
		while(i<N){
			if(P[i].ready_time > atime)
				break;
			if(P[i].execution_time > 0)
				return i;
			i++;
#ifdef DEBUG
			fprintf(stderr, "i = %d\n", i);
#endif
		}
		return -1;
	}

	else if(method == 1){
		//RR
		if(now_process != -1){
			if(atime - P[now_process].real_start_time >= Round_rubin_time){
				
				if(first != last){
					int a = queue[first];
					P[a].real_start_time = atime;
					queue_pop();
					if(P[now_process].execution_time > 0)
						queue_push(now_process);
					return a;
				}
				else{
					return now_process;
				}
				







			}

			return now_process;
		}
		if(first != last){
			int j = queue[first];
			queue_pop();
			return j;
		}

		return -1;

	}

	else if(method == 2){
		
		if(now_process != -1) // The now_process hasnʼt finished yet.
			return now_process;
		
		int the_shortest_process = -1;
		int the_shortest_time = INT_MAX; 
		
		for(int i = 0; i < N; i++){
			if(P[i].ready_time > atime){
				break;
			}
			if(P[i].execution_time > 0 && P[i].execution_time < the_shortest_time){
				the_shortest_process = i;
				the_shortest_time = P[i].execution_time;
			}
		}

		if(the_shortest_process != -1)
			return the_shortest_process;

		return -1;
	}

	else if(method == 3){
		int the_shortest_process = -1;
		int the_shortest_time = INT_MAX; 
		
		for(int i = 0; i < N; i++){
			if(P[i].ready_time > atime){
				break;
			}
			if(P[i].execution_time > 0 && P[i].execution_time < the_shortest_time){
				the_shortest_process = i;
				the_shortest_time = P[i].execution_time;
			}
		}

		if(the_shortest_process != -1)
			return the_shortest_process;

		return -1;
	}
}

void process_exec(int p_number){
	
	int ex_time = P[p_number].execution_time;
	for(int i = 0; i < ex_time; i++){
		a_second();
		atime++;
#ifdef DEBUG
		fprintf(stderr,"Process %d execute a second!\n", p_number);
#endif
	

	}
	
	return;

}

void process_block(int p_number){
	struct sched_param param;
	param.sched_priority = 0;
	int i = sched_setscheduler(P[p_number].pid, SCHED_IDLE, &param);

	return;
}
int p = 0;
void process_wake(int p_number){
	struct sched_param param;
	param.sched_priority = 99;
	sched_setscheduler(P[p_number].pid, SCHED_FIFO, &param);
	
	return;
}


int main(){
	char S[8];
	int N;
	//printf("HI\n");
	scanf("%s%d", S, &N);
	for(int i = 0; i < N; i++){
		scanf("%s%d%d", P[i].name, &P[i].ready_time, &P[i].execution_time);
		P[i].pid = -1;
		P[i].real_start_time = -1;
	}
	qsort(P, N, sizeof(P[0]), compare_function);
	int method = -1;
	if(strcmp(S, "FIFO") == 0)
		method = 0;
	if(strcmp(S, "RR") == 0)
		method = 1;
	if(strcmp(S, "SJF") == 0)
		method = 2;
	if(strcmp(S, "PSJF") == 0)
		method = 3;

	//initialize
	cpu_set_t sched_mask;
	cpu_set_t pro_mask;
	scheduler_mask_initialize(&sched_mask);
	process_mask_initialize(&pro_mask);
	int now_process = -1;
	int run_process_number = 0;

	if(sched_setaffinity(getpid(), sizeof(sched_mask), &sched_mask) < -1)
		fprintf(stderr, "scheduler: set affinity failure!\n");

#ifdef DEBUG
	fprintf(stderr, "set affinity success!\n");
#endif

	struct sched_param param;
	param.sched_priority = 99;
	sched_setscheduler(getpid(), SCHED_FIFO, &param);

#ifdef DEBUG
	fprintf(stderr, "initialize success\n");
#endif
	int i = 0;
	//run
	while(1){

		if(now_process != -1 && P[now_process].execution_time == 0){
			run_process_number ++;
			waitpid(P[now_process].pid, NULL ,0);
			fprintf(stdout, "%s %d\n", P[now_process].name, P[now_process].pid);
#ifdef DEBUG
			fprintf(stderr,"I waiting for you~\n");
#endif
			now_process = -1;
		}

		if(run_process_number == N)
			break;
		
		while(i < N){
			if(P[i].ready_time == atime){
				int ppid = fork();
				
				if(ppid > 0){
					process_block(i);
					P[i].pid = ppid;
					if(sched_setaffinity(ppid, sizeof(pro_mask), &pro_mask) == -1)
						fprintf(stderr, "Process: set affinity failure!\n");
					if(method == 1)
						queue_push(i);



					

#ifdef DEBUG
					fprintf(stderr,"pid = %d\n", P[i].pid);
#endif
				}
				else if(ppid == 0){
					P[i].pid = getpid();

					
					//while(P[i].pid == -1);
#ifdef DEBUG
					fprintf(stderr,"I am child Process %d!\n", P[i].pid);
#endif

					
					
					int a = P[i].execution_time;
					P[i].real_start_time = atime;

					
					
					unsigned long time1, time2;
					syscall(333, time1 , time2, getpid(), 0);
					//fprintf(stderr, "P[%d], start time = %lu.%lu\n", i, *time1, *time2);
					
					process_exec(i);
				
					syscall(333, time1 , time2, &P[i].pid, 1);
					//fprintf(stderr, "P[%d], terminal time = %lu.%lu\n", i, *time1, *time2);
					exit(0);
				}
			}
			else if(P[i].ready_time > atime) break;
			i++;
		}

		int next_process = next(now_process, N, method);
#ifdef DEBUG
		fprintf(stderr, "%d %d %d %d\n", next_process, run_process_number, atime, P[now_process].execution_time);
		for(int k = first; k < last; k++){
			fprintf(stderr, "%d ", queue[k]);
		}
		fprintf(stderr, "\n");
#endif		
		if(next_process != now_process){

#ifdef DEBUG
			fprintf(stderr, "wake %d Process~\n", next_process);
#endif
			if(next_process != -1)
				process_wake(next_process);
			if(now_process != -1)
				process_block(now_process);
			
			
#ifdef DEBUG
			fprintf(stderr, "block %d Process~\n", now_process);
#endif				
		}

		now_process = next_process;
		if(now_process != -1)
			P[now_process].execution_time--;
		a_second();
		atime ++;
	}

}