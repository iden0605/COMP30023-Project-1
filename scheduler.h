#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

#include "process.h"
#include "queue.h"

/* Scheduler configuration structure, populated from command line arguments.
 *  n_levels  - number of priority queues
 *  quanta    - dynamically allocated array of size n_levels
 *  has_boost - 1 if priority boosting is enabled, else 0
 *  boost_k   - boost interval in ticks
 */
typedef struct {
    int n_levels;
    uint32_t *quanta;
    int has_boost;
    uint32_t boost_k;
} SchedConfig;

/* Mutable simulation state shared across scheduler steps.
 *  procs        - process array (mutated in place)
 *  nproc        - number of processes in procs
 *  config          - scheduler configuration
 *  queues       - one ready queue per priority level
 *  n            - number of priority levels (mirror of config->n_levels)
 *  t            - current simulation tick
 *  curr_running - index of the process currently on the CPU (-1 if idle)
 *  prev_running - index of the process that ran during the previous tick
 *  finished     - number of processes that have reached PROC_FINISHED
 *  next_arrival - index of the next process awaiting admission
 */
typedef struct {
    Proc *procs;
    int nproc;
    const SchedConfig *config;
    Queue *queues;
    int n;
    uint32_t t;
    int curr_running;
    int prev_running;
    int finished;
    int next_arrival;
} SchedState;

/* MLFQ simulation, printing the transcript and statistics to stdout.
 *  procs - array of processes sorted by (arrival, pid)
 *  nproc - number of processes in procs
 *  config   - scheduler configuration
 */
void scheduler_run(Proc *procs, int nproc, const SchedConfig *config);

/* Handles the previous tick's running process, applies demotion as needed. */
void handle_prev_running(SchedState *s);

/* Moves processes whose I/O burst just completed from BLOCKED to READY */
void handle_io_completions(SchedState *s);

/* Insert any new arrived processes at the back of the highest priority queue. */
void add_arrivals(SchedState *s);

/* Empties all queues, resets every process's level to 0, and
 * inserts READY processes into Q0 in increasing PID order. BLOCKED
 * processes stay BLOCKED, re-inserted after I/O completion. */
void apply_boost(SchedState *s);

/* If a higher priority queue became ready while a process is running,
 * the process is preempted. */
void try_priority_preempt(SchedState *s);

/* If the CPU is idle, pick the head of the highest priority non-empty
 * queue as the next process to run. */
void select_next_process(SchedState *s);

/* Updates the simulation by one tick, decrements the running process's
 * CPU and quantum, and the I/O burst of every blocked process. */
void update_tick(SchedState *s);

/* Computes and prints statistics of the simulation. */
void print_statistics(const Proc *procs, int nproc);

#endif
