#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "parser.h"


static ctx *p = NULL;

/*static void dump_v1()
{
  u8 *d = NULL;
  for(u8 i = 0; i < p->wlen; i++) // d scan each charset
  {
    d = p->idx[i];
    scan(&d[1], &d[0], MARK_ALL, &p->word[i]);
    puts("");
  }
}*/


static void sig_handler(int signo) // use p to access data
{
  // uses -USR1
  if(signo == SIGUSR1) p->work = DUMP; // dump the ctx, marking current item in charsets

  // grab Ctrl-c
  if(signo == SIGINT ) p->work = INTR;
}


void setup_signals(ctx *ctx)
{
  p = ctx; // address p

  if(ctx->out_m == QUIET) fprintf(stderr, "send 'kill -USR1 %d' from another terminal to dump\n", getpid());

  if(signal(SIGUSR1, sig_handler) == SIG_ERR) fprintf(stderr, "\ncan't catch SIGUSR1\n");

  if(signal(SIGINT,  sig_handler) == SIG_ERR) fprintf(stderr, "\ncan't catch SIGINT\n");
}
