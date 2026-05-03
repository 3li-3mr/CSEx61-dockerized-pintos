#include "userprog/syscall.h"
#include "userprog/process.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "filesys/filesys.h"
#include "filesys/file.h"

static void syscall_handler (struct intr_frame *);

/* Helper function to read arguments safely from the stack.
   Person A MUST add pointer validation inside this function in Phase 2.2
   before it dereferences f->esp. */
static uint32_t *
get_arg (struct intr_frame *f, int offset)
{
  return (uint32_t *) ((uint8_t *) f->esp + offset);
}

static void
set_exit_status (int status)
{
#ifdef USERPROG
  thread_current ()->exit_status = status;
#else
  (void) status;
#endif
}

static struct lock filesys_lock;


void
syscall_init (void) 
{
    lock_init (&filesys_lock);

  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}
void validate_user_pointer (const void *ptr)
{
  if (ptr == NULL || !is_user_vaddr (ptr))
    {
      thread_current ()->exit_status = -1;
      thread_exit ();
    }
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
              
              struct file *file = thread_get_file (fd);
              if(file != NULL)
                {
                  lock_acquire (&filesys_lock);
                  f->eax = file_write (file, buffer, size);
                  lock_release (&filesys_lock);
                }
              else
                f->eax = -1;
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
          const char *file = (const char *)*(uint32_t *)get_arg (f, 4);
          unsigned initial_size = *(unsigned *)get_arg (f, 8);

          validate_user_pointer (file);
          lock_acquire (&filesys_lock);
          f->eax = filesys_create (file, initial_size);
          lock_release (&filesys_lock);
          break;
        }

      case SYS_REMOVE:
        {
          /* Read 1 argument: const char *file */
          const char *file = (const char *)*(uint32_t *)get_arg (f, 4);
          validate_user_pointer (file);
          lock_acquire (&filesys_lock);
          f->eax = filesys_remove (file);
          lock_release (&filesys_lock);
          break;
        }

      case SYS_OPEN:
        {
          /* Read 1 argument: const char *file */
          const char *file = (const char *)*(uint32_t *)get_arg (f, 4);
          validate_user_pointer (file);
          lock_acquire (&filesys_lock);
          struct file *opened_file = filesys_open (file);
          lock_release (&filesys_lock);
          if(opened_file != NULL)
            f->eax = thread_add_file (opened_file);
          else
            f->eax = -1;
          break;
        }

      case SYS_FILESIZE:
        {
          /* Read 1 argument: int fd */
          int fd = *(int *)get_arg (f, 4);
          /* Phase 2.2 (Person A): Get file size from FD table */
          struct file *file = thread_get_file (fd);
          if(file != NULL)
            {
              lock_acquire (&filesys_lock);
              f->eax = file_length (file);
              lock_release (&filesys_lock);
            }
          else
            f->eax = -1;
          break;
      }

      case SYS_READ:
        {
          /* Read 3 arguments: int fd, void *buffer, unsigned size */
          int fd= *(int *)get_arg (f, 4);
          void *buffer = (void *)*(uint32_t *)get_arg (f, 8);
          unsigned size = *(unsigned *)get_arg (f, 12);
          validate_user_pointer (buffer);
          if(fd == 0)
            {
              /* Hardcoded console input for testing framework */
              size_t i;
              for (i = 0; i < size; i++)
                ((char *)buffer)[i] = input_getc ();
              f->eax = size;
            }
            else if (fd == 1)
            {
              /* Writing to stdout is not allowed, return error */
              f->eax = -1; //negative area
            }
          else
            {
              /* Phase 2.2 (Person A): Read from actual files via FD table */
              struct file *file = thread_get_file (fd);
              if(file != NULL)
                {
                  lock_acquire (&filesys_lock);
                  f->eax = file_read (file, buffer, size);
                  lock_release (&filesys_lock);
                }
              else
                f->eax = -1;
            }
          
          break;
        }

      case SYS_SEEK:
        {
          /* Read 2 arguments: int fd, unsigned position */
          int fd = *(int *)get_arg (f, 4);
          unsigned position = *(unsigned *)get_arg (f, 8);
            /* Phase 2.2 (Person A): Seek in actual files via FD table */
            struct file *file = thread_get_file (fd);
            if(file != NULL)
              {
                lock_acquire (&filesys_lock);
                file_seek (file, position);
                lock_release (&filesys_lock);
              }
              else
                f->eax = -1;


          
          break;
        }

      case SYS_TELL:
        {
          /* Read 1 argument: int fd */
          int fd = *(int *)get_arg (f, 4);
            /* Phase 2.2 (Person A): Tell position in actual files via FD table */
            struct file *file = thread_get_file (fd);
            if(file != NULL)
              {
                lock_acquire (&filesys_lock);
                f->eax = file_tell (file);
                lock_release (&filesys_lock);
              }
              else
                f->eax = -1;
          
          break;
        }

      case SYS_CLOSE:
        {
          /* Read 1 argument: int fd */
          int fd = *(int *)get_arg (f, 4);
          /* Phase 2.2 (Person A): Close actual files via FD table */
          struct file *file = thread_get_file (fd);
          if(file != NULL)
            {
              lock_acquire (&filesys_lock);
              file_close (file);
              lock_release (&filesys_lock);
              thread_remove_file (fd);
            }
          else
            f->eax = -1;
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