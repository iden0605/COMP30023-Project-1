#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "process.h"
#include "scheduler.h"

int main(int argc, char **argv) {
    const char *fname = NULL;
    SchedConfig config = {0};
    char *q_arg = NULL;

    // Collect command line arguments
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-f")) {
            i++;
            fname = argv[i];
        }
        else if (!strcmp(argv[i], "-n")) {
            i++;
            config.n_levels = atoi(argv[i]);
        }
        else if (!strcmp(argv[i], "-q")) {
            i++;
            q_arg = argv[i];
        }
        else if (!strcmp(argv[i], "-b")) {
            config.has_boost = 1;
            config.boost_k = (uint32_t)strtoul(argv[++i], NULL, 10);
        }
    }

    // Allocate and tokenise -q using known n_levels
    config.quanta = malloc(config.n_levels * sizeof(uint32_t));
    int k = 0;
    char *tok = strtok(q_arg, ",");
    while (tok && k < config.n_levels) {
        config.quanta[k++] = (uint32_t)strtoul(tok, NULL, 10);
        tok = strtok(NULL, ",");
    }

    // Load processes and run simulation
    int nproc = 0;
    Proc *procs = procs_load(fname, &nproc);
    scheduler_run(procs, nproc, &config);

    // Cleanup
    procs_free(procs, nproc);
    free(config.quanta);

    return 0;
}
