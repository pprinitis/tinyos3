
#include "tinyos.h"
#include "kernel_sched.h"
#include "kernel_proc.h"
#include "kernel_threads.h"
#include "util.h"

/** 
  @brief Create a new thread in the current process.
  */
Tid_t sys_CreateThread(Task task, int argl, void* args)
{ 
  PTCB* ptcb = init_ptcb;
  ptcb->tcb = spawn_cthread(CURPROC,ptcb,start_common_thread);
  wakeup(ptcb->tcb);
	return (Tid_t)ptcb;
}
PTCB* init_ptcb(Task task,int argl ,void* args)
{
  PTCB* ptcb = NULL;
  ptcb->task = task;
  ptcb->argl = argl;
  ptcb->args = args;

  ptcb->exited = 0;
  ptcb->detached = 0;
  ptcb->exit_cv = COND_INIT;
  ptcb->refcount = 0;

  rlnode_init(& ptcb->ptcb_list_node, ptcb);
  rlist_push_front(& CURPROC->ptcb_list, &ptcb->ptcb_list_node);
  CURPROC->tread_count+=1;

  return ptcb;

}
void start_common_thread()
{
  int exitval;

  Task call = CURPT->task;
  int argl = CURPT->argl;
  void* args = CURPT ->args;

  sys_ThreadExit(exitval);
}


/**
  @brief Return the Tid of the current thread.
 */
Tid_t sys_ThreadSelf()
{
	return (Tid_t) cur_thread();
}

/**
  @brief Join the given thread.
  */
int sys_ThreadJoin(Tid_t tid, int* exitval)
{
  
  PTCB* threadwait = rlist_find(&CURPROC->ptcb_list, tid,NULL);
  if(threadwait==NULL){
    return -1;
  }
  if(threadwait->detached==1){
    return -1;
  }
  while(threadwait->tcb->state==ALIVE)
    kernel_wait(&CURPT->exit_cv,SCHED_USER);

    
	return 0;
}

/**
  @brief Detach the given thread.
  */
int sys_ThreadDetach(Tid_t tid)
{
	return -1;
}

/**
  @brief Terminate the current thread.
  */
void sys_ThreadExit(int exitval)
{

}

