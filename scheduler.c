#include "scheduler.h"

#include <stdio.h>
#include <stdint.h>

#include "queue.h"

void scheduler_run(Proc *procs, int nproc, const SchedConfig *cfg) {
    if (nproc == 0) {
        return;
    }

    // Initialise MLFQ with one queue and quantum
    Queue q0;
    q_init(&q0);
    uint32_t quantum0 = cfg->quanta[0];

    // Simulation states
    int curr_running = -1;
    int prev_running = -1;
    uint32_t t = 0;
    int finished = 0;
    int next_arr = 0;

    // Run simulation until all processes are finished
    while (finished < nproc) {
        // Resolve previous running process
        if (prev_running >= 0) {
            Proc *p = &procs[prev_running];
            if (p->cpu_left == 0) {
                // CPU burst exhausted, process is finished
                printf("%u,FINISHED,process=%s\n", t, p->pid);
                p->finish_time = t;
                finished++;
                curr_running = -1;
            }
            else if (p->quantum_left == 0) {
                // Quantum exhausted, preempt and reinsert at back of queue
                printf("%u,PREEMPTED,process=%s,queue=0,remaining-quantum=0,reason=quantum\n",
                       t, p->pid);
                q_push(&q0, prev_running);
                curr_running = -1;
            }
        }

        // Add new arrived processes
        while (next_arr < nproc && procs[next_arr].arrival == t) {
            q_push(&q0, next_arr++);
        }

        // Pick process from the head of the queue if CPU is idle
        if (curr_running < 0 && !q_empty(&q0)) {
            curr_running = q_pop(&q0);
            // Restart quantum for this process
            procs[curr_running].quantum_left = quantum0;
            printf("%u,RUNNING,process=%s,queue=0,remaining-cpu=%u,remaining-quantum=%u\n",
                   t, procs[curr_running].pid, procs[curr_running].cpu_left, procs[curr_running].quantum_left);
        }

        // Update running process
        if (curr_running >= 0) {
            procs[curr_running].cpu_left--;
            procs[curr_running].quantum_left--;
        }

        prev_running = curr_running;
        t++;
    }

    print_statistics(procs, nproc);

    q_free(&q0);
}

void print_statistics(const Proc *procs, int nproc) {
    uint64_t sum_turn = 0;
    uint64_t sum_wait = 0;
    uint64_t max_wait = 0;

    // Sum turnaround and waiting times of all processes
    for (int i = 0; i < nproc; i++) {
        uint64_t turn = (uint64_t)procs[i].finish_time - procs[i].arrival;
        uint64_t wait = turn - procs[i].cpu_total;
        sum_turn += turn;
        sum_wait += wait;

        // Update max waiting time
        if (wait > max_wait) {
            max_wait = wait;
        }

    }

    // Get averages and print statistics
    uint64_t avg_turn = (sum_turn + nproc - 1) / nproc;
    uint64_t avg_wait = (sum_wait + nproc - 1) / nproc;
    printf("Average turnaround time %llu\n", (unsigned long long)avg_turn);
    printf("Average waiting time %llu\n", (unsigned long long)avg_wait);
    printf("Maximum waiting time %llu\n", (unsigned long long)max_wait);
}