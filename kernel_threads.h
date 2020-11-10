#include "kernel_cc.h"
#include "kernel_proc.h"
#include "kernel_streams.h"

typedef struct process_thread_control_block
{
    TCB* tcb;

    Task task;
    int argl;
    void* args;
    int exited;
    int detached;

    CondVar exit_cv;
    int refcount;
    rlnode ptcb_list_node;
}PTCB;

PTCB* init_ptcb(Task task, int argl, void* args);
void start_common_thread();