#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"

static void syscall_handler (struct intr_frame *);

/* Helper function to read arguments safely from the stack.
   Person A MUST add pointer validation inside this function in Phase 2.2
   before it dereferences f->esp. */
static uint32_t *
get_arg (struct intr_frame *f, int offset)
{
  return (uint32_t *) ((uint8_t *) f->esp + offset);
}

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
  /* 1. Read the syscall number from the stack pointer */
  int syscall_number = *(int *)(f->esp);

  /* 2. Dispatch the syscall */
  switch (syscall_number)
    {
      /* --- UNBLOCKING PHASE (Phase 2.1) --- */

      case SYS_HALT:
        {
          shutdown_power_off ();
          break;
        }

      case SYS_EXIT:
        {
          int status = *(int *)get_arg (f, 4);
          thread_current ()->exit_status = status;
          thread_exit ();
          break;
        }

      case SYS_WRITE:
        {
          int fd = *(int *)get_arg (f, 4);
          const void *buffer = (const void *)*(uint32_t *)get_arg (f, 8);
          unsigned size = *(unsigned *)get_arg (f, 12);

          /* Hardcoded console output for testing framework */
          if (fd == 1)
            {
              putbuf (buffer, size);
              f->eax = size;
            }
          else
            {
              /* Phase 2.2 (Person A): Write to actual files via FD table */
              f->eax = 0; 
            }
          break;
        }

      /* --- PROCESS & CONCURRENCY (Phase 2.2 - Person B) --- */

      case SYS_EXEC:
        {
          const char *cmd_line = (const char *)*(uint32_t *)get_arg (f, 4);
          f->eax = process_execute (cmd_line);
          break;
        }

      case SYS_WAIT:
        {
          tid_t child_tid = *(tid_t *)get_arg (f, 4);
          f->eax = process_wait (child_tid);
          break;
        }

      /* --- MEMORY & I/O (Phase 2.2 - Person A) --- */

      case SYS_CREATE:
        {
          /* Read 2 arguments: const char *file, unsigned initial_size */
          
          break;
        }

      case SYS_REMOVE:
        {
          /* Read 1 argument: const char *file */
          
          break;
        }

      case SYS_OPEN:
        {
          /* Read 1 argument: const char *file */
          
          break;
        }

      case SYS_FILESIZE:
        {
          /* Read 1 argument: int fd */
          
          break;
        }

      case SYS_READ:
        {
          /* Read 3 arguments: int fd, void *buffer, unsigned size */
          
          break;
        }

      case SYS_SEEK:
        {
          /* Read 2 arguments: int fd, unsigned position */
          
          break;
        }

      case SYS_TELL:
        {
          /* Read 1 argument: int fd */
          
          break;
        }

      case SYS_CLOSE:
        {
          /* Read 1 argument: int fd */
          
          break;
        }

      /* --- FALLBACK --- */

      default:
        {
          /* Unknown syscall: terminate the process safely */
          thread_current ()->exit_status = -1;
          thread_exit ();
          break;
        }
    }
}