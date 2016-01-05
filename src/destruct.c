/**
 * \file destruct.c
 *    \brief Destructors.
 * \author k.edeline
 * \version 0.1
 */

/**
 * \fn static void destruct()
 * \brief Kill processes, threads, close fds and free memory.
 */

#include "destruct.h"
#include "sock.h"
#include "debug.h"

static void destruct();

static volatile pthread_t *ctid;
static volatile pid_t *cpid;
static volatile int *fd;
static volatile unsigned int ctid_index;
static volatile unsigned int cpid_index;
static volatile unsigned int fd_index;
pthread_mutex_t lock;

void set_pthread(pthread_t t) {
   if (pthread_mutex_lock(&lock) != 0)
      die("mutex lock");
   ctid[ctid_index++] = t;
   if (pthread_mutex_unlock(&lock) != 0)
      die("mutex unlock");
}

void set_cpid(pid_t c) {
   if (pthread_mutex_lock(&lock) != 0)
      die("mutex lock");
   cpid[cpid_index++] = c;
   if (pthread_mutex_unlock(&lock) != 0)
      die("mutex unlock");
}

void set_fd(int fds){
   if (pthread_mutex_lock(&lock) != 0)
      die("mutex lock");
   fd[fd_index++] = fds;
   if (pthread_mutex_unlock(&lock) != 0)
      die("mutex unlock");
}

void destruct() {

   if (pthread_mutex_lock(&lock) != 0)
      die("mutex lock");
   debug_print("exiting ...\n");
   // Ensure child process is killed and socket is closed   
   /* thread ids */
   for (int i=0; i<ctid_index; i++) {
      if (!pthread_cancel(ctid[i])) {
         debug_print("thread %d canceled\n", (int)ctid[i]);
         continue;
      }
      pthread_kill(ctid[i], SIGKILL);
      pthread_kill(ctid[i], SIGTERM);
   }
   /* process ids */
   for (int i=0; i<cpid_index; i++) {
      kill(cpid[i], SIGKILL);
      kill(cpid[i], SIGTERM);
   }
   /* file descriptors */
   for (int i=0; i<fd_index; i++) close(fd[i]);

   /* mallocs */
   free((void*)ctid);free((void*)cpid);
   free((void*)fd);

   if (pthread_mutex_unlock(&lock) != 0)
      die("mutex unlock");
   pthread_mutex_destroy(&lock);
}

void init_destructors(struct tun_state *state) {
   /* init & lock mutex */
   if (pthread_mutex_init(&lock, NULL) != 0) 
      die("mutex init");
   if (pthread_mutex_lock(&lock) != 0)
      die("mutex lock");
   
   /* Set Processes, Threads and File descriptors destructors*/
   ctid = calloc(state->fd_lim, sizeof(pthread_t));
   cpid = calloc(state->fd_lim, sizeof(pid_t));
   fd   = calloc(state->fd_lim, sizeof(int));

   ctid_index = 0;
   cpid_index = 0;
   fd_index   = 0;

   atexit(destruct);

   if (pthread_mutex_unlock(&lock) != 0)
      die("mutex unlock");
}
