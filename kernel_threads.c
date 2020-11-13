
#include "tinyos.h"
#include "kernel_streams.h"
#include "kernel_sched.h"
#include "kernel_proc.h"
#include "kernel_threads.h"
#include "util.h"

/** 
  @brief Create a new thread in the current process.
  */

Tid_t sys_CreateThread(Task task, int argl, void* args)
{ 
  PTCB* ptcb = makePTCB(CURPROC,task,argl,args);
  TCB* tcb = spawn_thread(CURPROC,start_common_thread);
  ptcb->tcb = tcb;
  tcb->owner_ptcb = ptcb;
  wakeup(ptcb->tcb);
	return (Tid_t)ptcb;
}

PTCB* makePTCB(PCB* pcb,Task task, int argl, void* args)
{
  PTCB *ptcb =xmalloc(sizeof(PTCB));
  
  ptcb->argl=argl;
  ptcb->args=args;
  ptcb->task=task;
  ptcb->exitval=0;
  ptcb->exited = 0;
  ptcb->detached = 0;
  ptcb->exit_cv = COND_INIT;
  ptcb->refcount = 1;

  rlnode_init(&ptcb->ptcb_list_node, ptcb);
  rlnode_init(&pcb->ptcb_list,&pcb);
  rlist_push_front(&pcb->ptcb_list,&ptcb->ptcb_list_node);
  pcb->tread_count+=1;

  return ptcb;
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
  PTCB* ptcb = (PTCB*) tid;
  if(rlist_find(&CURPROC->ptcb_list,ptcb,NULL)==NULL){
    return -1;
  }
  if(ptcb->detached==1){
    return -1;
  }
  if(sys_ThreadSelf()==tid){
    return -1;
  }
  PTCB* waitptcb = (PTCB*) sys_ThreadSelf();

  waitptcb ->refcount++;

  while(ptcb->exited==0 && ptcb->detached==0){
    kernel_wait(&ptcb->exit_cv,SCHED_USER);
  }
  if(exitval != NULL){
    *exitval = ptcb->exitval;
  }
  ptcb->refcount --;

  return 0;
 
}



/**
  @brief Detach the given thread.
  */
int sys_ThreadDetach(Tid_t tid)
{
  
  PTCB* ptcb = (PTCB*) tid;
  if(sys_ThreadSelf()==tid &&ptcb->exited!=1){
    ptcb->detached=1;
    return 0;
  }
  
  if(rlist_find(&CURPROC->ptcb_list,ptcb,NULL)==NULL){
    return -1;
  }
  if(ptcb->exited==1){
    return -1;
  }
  ptcb->detached=1;
  return 0;
  
}

/**
  @brief Terminate the current thread.
  */
void sys_ThreadExit(int exitval)
{
  PCB *pcb =CURPROC;
  PTCB *ptcb = CURPT; 
  pcb->tread_count--;
  ptcb->exited=1;
  ptcb->exitval=exitval;
  ptcb->tcb = NULL;
  if(ptcb->refcount==1)
  rlist_remove(&ptcb->ptcb_list_node);
  //kernel_broadcast(&ptcb->exit_cv);

  if(pcb->tread_count==1){
    sys_Exit(exitval);
  }
  kernel_sleep(EXITED,SCHED_USER);
  }





