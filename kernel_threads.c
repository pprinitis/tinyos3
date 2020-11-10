
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
  PTCB* ptcb = init_ptcb(task,argl,args);
  ptcb->tcb = spawn_cthread(CURPROC,ptcb,start_common_thread);
  wakeup(ptcb->tcb);
	return (Tid_t)ptcb;
}
PTCB* init_ptcb(Task task,int argl ,void* args)
{
  PTCB* ptcb = NULL;
  if(task==NULL){
    return 0;
  }
  ptcb->task = task;
  ptcb->argl = argl;

  if(args!=NULL){
    ptcb->args = malloc (argl);
    memcpy(ptcb->args, args, argl);
  }else
    ptcb->args = NULL;

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

  exitval = call(argl,args);
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
  
  PTCB* threadwait = rlist_find(&CURPROC->ptcb_list,(PTCB*)& tid,NULL);
  if(threadwait==NULL){
    return -1;
  }else{

    if(threadwait->detached==1){
     printf("Detached thread are not joinable");
     return -1;
    }else{
    return wait_for_specific_thread(threadwait,exitval);
    }
  }
}

int wait_for_specific_thread(PTCB* tid, int* exitval)
{
  PTCB* ct = CURPT;
  
  while(tid->tcb->state == RUNNING)
    kernel_wait(& ct->exit_cv, SCHED_USER);


}

/**
  @brief Detach the given thread.
  */
int sys_ThreadDetach(Tid_t tid)
{
  PTCB* threaddet = rlist_find(&CURPROC->ptcb_list, (PTCB*)&tid,NULL);
  if(threaddet!=NULL || threaddet->exited==1){
    threaddet->detached = 1;
    return 0;
  }else{
    return -1;
  }
  
}

/**
  @brief Terminate the current thread.
  */
void sys_ThreadExit(int exitval)
{
  PTCB* cpt = CURPT;
  PCB* cpr = CURPROC;
  TCB* tcb = cpt->tcb;

  tcb->state = ZOMBIE;
  tcb->phase = EXITED;
  cpt->exited = 1;
  rlist_remove(&cpt->ptcb_list_node);

  if(cpt->refcount==0){

  }

  if(is_rlist_empty(&cpr->ptcb_list)){
    sys_Exit(exitval);
  }



}

