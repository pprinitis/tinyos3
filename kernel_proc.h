#ifndef __KERNEL_PROC_H
#define __KERNEL_PROC_H

/**
  @file kernel_proc.h
  @brief The process table and process management.

  @defgroup proc Processes
  @ingroup kernel
  @brief The process table and process management.

  This file defines the PCB structure and basic helpers for
  process access.

  @{
*/ 

#include "tinyos.h"
#include "kernel_sched.h"

/**
  @brief PID state

  A PID can be either free (no process is using it), ALIVE (some running process is
  using it), or ZOMBIE (a zombie process is using it).
  */
typedef enum pid_state_e {
  FREE,   /**< @brief The PID is free and available */
  ALIVE,  /**< @brief The PID is given to a process */
  ZOMBIE  /**< @brief The PID is held by a zombie */
} pid_state;



typedef struct thread_snapshot_s {
  TCB* tcb;
  void* data;
  void* sp;
  rlnode snapnode;
  cpu_context_t context;
} thread_snapshot;


/**
  @brief Process Control Block.

  This structure holds all information pertaining to a process.
 */
typedef struct process_control_block {
  pid_state  pstate;      /**< @brief The pid state for this PCB */
  PCB* parent;            /**< @brief Parent's pcb. */


  int exitval;            /**< @brief The exit value of the process */
  TCB* main_thread;       /**< @brief The main thread */
  thread_snapshot* snapshot; /**< used by fork **/

  Task main_task;         /**< @brief The main thread's function */
  int argl;               /**< @brief The main thread's argument length */
  void* args;             /**< @brief The main thread's argument string */

  volatile int sigkill;   /**< @brief Mark a process for termination */

  rlnode children_list;     /**< @brief List of children */
  rlnode exited_list;       /**< @brief List of exited children */

  rlnode children_node;     /**< @brief Intrusive node for @c children_list */
  rlnode exited_node;       /**< @brief Intrusive node for @c exited_list */

  volatile int errcode;     /**< @brief The last error code returned to the process */

  wait_queue child_exit;     /**< @brief Wait queue for @c WaitChild. 

                             This queue is  broadcast each time a child
                             process terminates. It is used in the implementation of
                             @c WaitChild() */

  FCB* FIDT[MAX_FILEID];  /**< @brief The fileid table of the process */

  Inode* cur_dir;         /**< @brief Current file system directory */
  Inode* root_dir;        /**< @brief Root directory for this process */

} PCB;


/**
  @brief Initialize the process table.

  This function is called during kernel initialization, to initialize
  any data structures related to process creation.
*/
void initialize_processes();

/**
  @brief Get the PCB for a PID.

  This function will return a pointer to the PCB of 
  the process with a given PID. If the PID does not
  correspond to a process, the function returns @c NULL.

  @param pid the pid of the process 
  @returns A pointer to the PCB of the process, or NULL.
*/
PCB* get_pcb(Pid_t pid);

/**
  @brief Get the PID of a PCB.

  This function will return the PID of the process 
  whose PCB is pointed at by @c pcb. If the pcb does not
  correspond to a process, the function returns @c NOPROC.

  @param pcb the pcb of the process 
  @returns the PID of the process, or NOPROC.
*/
Pid_t get_pid(PCB* pcb);


/**
  @brief Set the error code in the current process.

  @param e the new value of the error code.
  */
static inline void set_errcode(int e)
{
  CURPROC->errcode = e;
}


/** @} */

#endif
