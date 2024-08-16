#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structures.h"


struct commands* create_commands(int id){
    
    struct commands* cmds = (struct commands*)malloc(sizeof(struct commands));
    
    bzero(cmds->rm_out,sizeof cmds->rm_out);
    sprintf(cmds->rm_out,"rm -r output%d.txt 2>/dev/null",id);

    bzero(cmds->rm_diff,sizeof cmds->rm_diff);
    sprintf(cmds->rm_diff,"rm -r diff%d.txt 2>/dev/null",id);
   
    bzero(cmds->rm_comp_err,sizeof cmds->rm_comp_err);
    sprintf(cmds->rm_comp_err,"rm -r comp_err%d.txt 2>/dev/null",id);

    bzero(cmds->rm_run_err,sizeof cmds->rm_run_err);
    sprintf(cmds->rm_run_err,"rm -r run_err%d.txt 2>/dev/null",id);

    bzero(cmds->rm_test,sizeof cmds->rm_test);
    sprintf(cmds->rm_test,"rm -r testing%d.c 2>/dev/null",id);

    bzero(cmds->rm_binaries,sizeof cmds->rm_binaries);
    sprintf(cmds->rm_binaries,"rm -r testing%d.o 2>/dev/null",id);

    bzero(cmds->open_test,sizeof cmds->open_test);
    sprintf(cmds->open_test,"testing%d.c",id);
    printf("Thread Created with id %s\n",cmds->open_test);
        
    bzero(cmds->open_comp_err,sizeof cmds->open_comp_err);
    sprintf(cmds->open_comp_err,"comp_err%d.txt",id);

    bzero(cmds->open_run_err,sizeof cmds->open_run_err);
    sprintf(cmds->open_run_err,"run_err%d.txt",id);

    bzero(cmds->open_diff, sizeof cmds->open_diff);
    sprintf(cmds->open_diff,"diff%d.txt",id);

    bzero(cmds->compile,sizeof cmds->compile);
    sprintf(cmds->compile,"gcc testing%d.c -o testing%d.o 2>comp_err%d.txt",id,id,id);

    bzero(cmds->run,sizeof cmds->run);
    sprintf(cmds->run,"./testing%d.o 1>output%d.txt 2>run_err%d.txt",id,id,id);

    bzero(cmds->diff,sizeof cmds->diff);
    sprintf(cmds->diff,"diff output%d.txt exp_out.txt > diff%d.txt",id,id);

    bzero(cmds->create_queue_size,sizeof cmds->create_queue_size);
    sprintf(cmds->create_queue_size,"touch queueSize.txt");

    bzero(cmds->rm_queue_size, sizeof cmds->rm_queue_size);
    sprintf(cmds->rm_queue_size,"rm -r queueSize.txt 2>/dev/null");

    bzero(cmds->append_queue_size, sizeof cmds->append_queue_size);
    sprintf(cmds->append_queue_size,"queueSize.txt");

    return cmds;
}