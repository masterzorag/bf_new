#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "parser.h"


static ctx *p = NULL;


void sig_handler(int signo) // use p to access data
{
  if(signo == SIGUSR1)
    printf("received SIGUSR1\n");
    // dump the ctx, marking current in charsets

  if (signo == SIGINT) // get Ctrl-c
  {
    DPRINTF("\nreceived SIGINT\n");
    FILE *fp = fopen(".bf.save", "w");
    if(!fp) exit(-1);

    size_t n = fwrite(p->word, sizeof(char), p->wlen, fp); // dump
    fclose(fp); fp = NULL;

    printf("written %zub, @%p\n", n, p->word);
    p = NULL;
    exit(1);
  }
}

void setup_signals(ctx *ctx)
{
  printf("send 'kill -USR1 %d' from another term\n", getpid());

  p = ctx; // address p

  if(signal(SIGUSR1, sig_handler) == SIG_ERR) printf("\ncan't catch SIGUSR1\n");

  if(signal(SIGINT,  sig_handler) == SIG_ERR) printf("\ncan't catch SIGINT\n");
}