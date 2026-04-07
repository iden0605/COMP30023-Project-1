#include "process.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INIT_PROC_CAP 16
#define INIT_BURST_CAP 4

/* qsort comparator: processes ordered by arrival time, breaking ties
 * by lexicographic PID order. */
static int cmp_proc(const void *a, const void *b) {
    const Proc *x = a;
    const Proc *y = b;

    if (x->arrival < y->arrival) {
        return -1;
    }
    else if (x->arrival > y->arrival) {
        return 1;
    }

    return strcmp(x->pid, y->pid);
}

Proc *procs_load(const char *fname, int *n) {
    FILE *f = fopen(fname, "r");
    
    // Return NULL on file error
    if (!f) {
        return NULL;
    }

    // Dynamic array of processes
    int nproc = 0;
    int cap = INIT_PROC_CAP;
    Proc *procs = malloc(cap * sizeof(Proc));

    // getline buffer
    char *line = NULL;
    size_t llen = 0;

    while (getline(&line, &llen, f) != -1) {
        // Double process array size if full
        if (nproc == cap) {
            cap *= 2;
            procs = realloc(procs, cap * sizeof(Proc));
        }

        Proc *p = &procs[nproc++];
        memset(p, 0, sizeof(*p));

        // Get process arrival time
        char *tok = strtok(line, " \n");
        p->arrival = (uint32_t)strtoul(tok, NULL, 10);

        // Get process PID
        tok = strtok(NULL, " \n");
        strncpy(p->pid, tok, PID_BUF - 1);

        // Get process bursts
        int bcap = INIT_BURST_CAP;
        p->bursts = malloc(bcap * sizeof(uint32_t));
        while ((tok = strtok(NULL, " \n"))) {
            // Double process bursts array size if full
            if (p->nbursts == bcap) {
                bcap *= 2;
                p->bursts = realloc(p->bursts, bcap * sizeof(uint32_t));
            }

            uint32_t v = (uint32_t)strtoul(tok, NULL, 10);
            p->bursts[p->nbursts++] = v;
            // Odd bursts are CPU, even bursts are I/O.
            if (p->nbursts % 2 == 1) {
                p->cpu_total += v;
            }
            else {
                p->io_total += v;
            }
        }

        // Initialise runtime field for first CPU burst
        p->cpu_left = p->bursts[0];
    }
    free(line);
    fclose(f);

    // Sort processes by (arrival, pid)
    if (procs) {
        qsort(procs, nproc, sizeof(Proc), cmp_proc);
    }
    *n = nproc;
    return procs;
}

void procs_free(Proc *procs, int n) {
    // Free process bursts arrays
    for (int i = 0; i < n; i++) {
        free(procs[i].bursts);
    }

    // Free process array
    free(procs);
}
