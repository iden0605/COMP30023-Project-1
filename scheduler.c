#include "scheduler.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "queue.h"

void scheduler_run(Proc *procs, int nproc, const SchedConfig *cfg) {
    if (nproc == 0) {
        return;
    }

    // Initialise one queue per priority level
    int n = cfg->n_levels;
    Queue *queues = malloc(n * sizeof(Queue));
    for (int i = 0; i < n; i++) {
        q_init(&queues[i]);
    }

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
                // Quantum exhausted, demote process by 1 level
                int old_level = p->level;
                printf("%u,PREEMPTED,process=%s,queue=%d,remaining-quantum=0,reason=quantum\n",
                       t, p->pid, old_level);

                if (p->level < n - 1) {
                    p->level++;
                }
                
                // Insert demoted process at the back of the next queue
                q_push(&queues[p->level], prev_running);
                curr_running = -1;
            }
        }

        // Add new arrived processes to highest priority queue
        while (next_arr < nproc && procs[next_arr].arrival == t) {
            procs[next_arr].level = 0;
            q_push(&queues[0], next_arr++);
        }

        // Priority preemption: a higher-priority queue became ready while a process was running
        if (curr_running >= 0) {
            Proc *p = &procs[curr_running];

            // Check queues with higher priority than this process's level
            int has_higher = 0;
            for (int i = 0; i < p->level; i++) {
                if (!q_empty(&queues[i])) {
                    has_higher = 1;
                    break;
                }
            }

            // Higher-priority process found, preempt the current one
            if (has_higher) {
                // No quantum_left reset, resumes with the same slice later
                printf("%u,PREEMPTED,process=%s,queue=%d,remaining-quantum=%u,reason=priority\n",
                       t, p->pid, p->level, p->quantum_left);
                q_push(&queues[p->level], curr_running);
                curr_running = -1;
            }
        }

        // Pick process from the highest priority non-empty queue
        if (curr_running < 0) {
            for (int i = 0; i < n; i++) {
                if (!q_empty(&queues[i])) {
                    curr_running = q_pop(&queues[i]);
                    Proc *p = &procs[curr_running];

                    // Reset quantum if used up
                    if (p->quantum_left == 0) {
                        p->quantum_left = cfg->quanta[p->level];
                    }
                    printf("%u,RUNNING,process=%s,queue=%d,remaining-cpu=%u,remaining-quantum=%u\n",
                           t, p->pid, p->level, p->cpu_left, p->quantum_left);
                    break;
                }
            }
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

    // Cleanup queues
    for (int i = 0; i < n; i++) {
        q_free(&queues[i]);
    }
    free(queues);
}

void print_statistics(const Proc *procs, int nproc) {
    uint64_t sum_turn = 0;
    uint64_t sum_wait = 0;
    uint64_t max_wait = 0;

    // Sum turnaround and waiting times of all processes
    for (int i = 0; i < nproc; i++) {
        uint64_t turn = (uint64_t)procs[i].finish_time - procs[i].arrival;
        uint64_t wait = turn - procs[i].cpu_total - procs[i].io_total;
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
