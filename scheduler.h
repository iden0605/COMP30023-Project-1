#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

#include "process.h"

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

/* MLFQ simulation, printing the transcript and statistics to stdout.
 *  procs - array of processes sorted by (arrival, pid)
 *  nproc - number of processes in procs
 *  cfg   - scheduler configuration
 */
void scheduler_run(Proc *procs, int nproc, const SchedConfig *cfg);

/* Compute and print statistics of the simulation. */
void print_statistics(const Proc *procs, int nproc);

#endif
