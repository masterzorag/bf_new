#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "parser.h"


static ctx *p = NULL;

static void dump_v1()
{
  u8 *d = NULL;
  for(u8 i = 0; i < p->wlen; i++) // d scan each charset
  {
    d = p->idx[i];
    scan(&d[1], &d[0], MARK_ALL, &p->word[i]);
    puts("");
  }
}


static void sig_handler(int signo) // use p to access data
{
  if(signo == SIGUSR1) // uses kill -USR1
  {
    DPRINTF("received SIGUSR1\n");
    //dump_v1();
    p->done = DUMP; // dump the ctx, marking current items in charsets
  }

  if(signo == SIGINT) // grab Ctrl-c
  {
    DPRINTF("\nreceived SIGINT\n"); puts("");
    p->done = DONE;
  }
}


void setup_signals(ctx *ctx)
{
  if(ctx->out_m == QUIET) printf("send 'kill -USR1 %d' from another terminal to dump\n", getpid());

  p = ctx; // address p

  if(signal(SIGUSR1, sig_handler) == SIG_ERR) printf("\ncan't catch SIGUSR1\n");

  if(signal(SIGINT,  sig_handler) == SIG_ERR) printf("\ncan't catch SIGINT\n");
}
