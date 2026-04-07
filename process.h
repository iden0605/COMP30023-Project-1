#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

#define PID_BUF 16

/* One process in the simulation.
 * Static fields are populated at load time, runtime fields are mutated
 * during simulation.
 *  pid          - process identifier
 *  arrival      - tick when the process is ready
 *  nbursts      - total number of bursts
 *  bursts       - alternating CPU/IO burst lengths, starts with CPU
 *  cpu_left     - remaining time in the current CPU burst
 *  quantum_left - remaining quantum while running
 *  finish_time  - tick when the process is finished
 *  cpu_total    - sum of all CPU bursts
 *  io_total     - sum of all I/O bursts
 */
typedef struct {
    char pid[PID_BUF];
    uint32_t arrival;
    int nbursts;
    uint32_t *bursts;
    uint32_t cpu_left;
    uint32_t quantum_left;
    uint32_t finish_time;
    uint64_t cpu_total;
    uint64_t io_total;
} Proc;

/* Read processes from 'fname' into an array.
 * Returns the array and writes size into *n.
 * Array sorted by (arrival, pid). Returns NULL on file error. */
Proc *procs_load(const char *fname, int *n);

/* Free an array of processes returned by procs_load. */
void procs_free(Proc *procs, int n);

#endif
