#include "scheduler.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

void scheduler_run(Proc *procs, int nproc, const SchedConfig *config) {
    if (nproc == 0) {
        return;
    }

    // Initialise one queue per priority level
    int n = config->n_levels;
    Queue *queues = malloc(n * sizeof(Queue));
    for (int i = 0; i < n; i++) {
        q_init(&queues[i]);
    }

    // Pack the simulation states into a struct
    SchedState s = {
        .procs = procs,
        .nproc = nproc,
        .config = config,
        .queues = queues,
        .n = n,
        .t = 0,
        .curr_running = -1,
        .prev_running = -1,
        .finished = 0,
        .next_arrival = 0,
    };

    // Run simulation until all processes are finished
    while (s.finished < s.nproc) {
        handle_prev_running(&s);
        handle_io_completions(&s);
        add_arrivals(&s);
        try_priority_preempt(&s);
        select_next_process(&s);
        update_tick(&s);

        s.prev_running = s.curr_running;
        s.t++;
    }

    print_statistics(procs, nproc);

    // Cleanup queues
    for (int i = 0; i < n; i++) {
        q_free(&queues[i]);
    }
    free(queues);
}

void handle_prev_running(SchedState *s) {
    if (s->prev_running < 0) {
        return;
    }

    Proc *p = &s->procs[s->prev_running];
    if (p->cpu_left == 0) {
        // CPU burst exhausted, advance to next burst
        p->burst_idx++;
        if (p->burst_idx >= p->nbursts) {
            // No more bursts, process is finished
            printf("%u,FINISHED,process=%s\n", s->t, p->pid);
            p->state = PROC_FINISHED;
            p->finish_time = s->t;
            s->finished++;
        }
        else {
            // Next burst is I/O, process becomes BLOCKED
            p->io_left = p->bursts[p->burst_idx];
            p->state = PROC_BLOCKED;
            printf("%u,BLOCKED,process=%s\n", s->t, p->pid);

            // Demotion if quantum finished this tick
            if (p->quantum_left == 0 && p->level < s->n - 1) {
                p->level++;
            }
        }
        s->curr_running = -1;
    }
    else if (p->quantum_left == 0) {
        // Quantum finished, demote process by 1 level
        int old_level = p->level;
        printf("%u,PREEMPTED,process=%s,queue=%d,remaining-quantum=0,reason=quantum\n",
               s->t, p->pid, old_level);

        if (p->level < s->n - 1) {
            p->level++;
        }

        // Insert demoted process at the back of the next queue
        p->state = PROC_READY;
        q_push(&s->queues[p->level], s->prev_running);
        s->curr_running = -1;
    }
}

void handle_io_completions(SchedState *s) {
    for (int lvl = 0; lvl < s->n; lvl++) {
        int header_printed = 0;

        // Selection sort, picks the smallest PID process each loop
        while (1) {
            int curr = -1;
            for (int i = 0; i < s->nproc; i++) {
                Proc *p = &s->procs[i];
                // Skip processes not just BLOCKED
                if (p->state != PROC_BLOCKED || p->io_left != 0 || p->level != lvl) {
                    continue;
                }
                if (curr < 0 || strcmp(p->pid, s->procs[curr].pid) < 0) {
                    curr = i;
                }
            }

            if (curr < 0) {
                break;
            }

            // Update process state for the next CPU burst, reset quantum
            Proc *p = &s->procs[curr];
            p->burst_idx++;
            p->cpu_left = p->bursts[p->burst_idx];
            p->quantum_left = 0;
            p->state = PROC_READY;

            if (!header_printed) {
                printf("%u,UNBLOCKED,queue=%d,processes=%s", s->t, lvl, p->pid);
                header_printed = 1;
            }
            else {
                printf(",%s", p->pid);
            }

            // Insert process at the back of this level queue
            q_push(&s->queues[lvl], curr);
        }

        if (header_printed) {
            printf("\n");
        }
    }
}

void add_arrivals(SchedState *s) {
    while (s->next_arrival < s->nproc && s->procs[s->next_arrival].arrival == s->t) {
        s->procs[s->next_arrival].level = 0;
        s->procs[s->next_arrival].state = PROC_READY;
        q_push(&s->queues[0], s->next_arrival++);
    }
}

void try_priority_preempt(SchedState *s) {
    if (s->curr_running < 0) {
        return;
    }

    Proc *p = &s->procs[s->curr_running];

    // Check queues with higher priority than this process's level
    int has_higher = 0;
    for (int i = 0; i < p->level; i++) {
        if (!q_empty(&s->queues[i])) {
            has_higher = 1;
            break;
        }
    }

    if (!has_higher) {
        return;
    }

    // Higher-priority process found, preempt the current process
    printf("%u,PREEMPTED,process=%s,queue=%d,remaining-quantum=%u,reason=priority\n",
           s->t, p->pid, p->level, p->quantum_left);
    p->state = PROC_READY;
    q_push(&s->queues[p->level], s->curr_running);
    s->curr_running = -1;
}

void select_next_process(SchedState *s) {
    if (s->curr_running >= 0) {
        return;
    }

    // Pick process from the highest priority non-empty queue
    for (int i = 0; i < s->n; i++) {
        if (q_empty(&s->queues[i])) {
            continue;
        }
        s->curr_running = q_pop(&s->queues[i]);
        Proc *p = &s->procs[s->curr_running];

        // Reset quantum if used up
        if (p->quantum_left == 0) {
            p->quantum_left = s->config->quanta[p->level];
        }
        p->state = PROC_RUNNING;
        printf("%u,RUNNING,process=%s,queue=%d,remaining-cpu=%u,remaining-quantum=%u\n",
               s->t, p->pid, p->level, p->cpu_left, p->quantum_left);
        return;
    }
}

void update_tick(SchedState *s) {
    // Update running process
    if (s->curr_running >= 0) {
        s->procs[s->curr_running].cpu_left--;
        s->procs[s->curr_running].quantum_left--;
    }
    // Decrement I/O for all blocked processes
    for (int i = 0; i < s->nproc; i++) {
        if (s->procs[i].state == PROC_BLOCKED) {
            s->procs[i].io_left--;
        }
    }
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
