#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "parser.h"


static ctx *p = NULL;


static void sig_handler(int signo) // use p to access data
{
  if(signo == SIGUSR1) // uses kill -USR
  {
    DPRINTF("received SIGUSR1\n");
    // dump the ctx, marking current item in charsets
    u8 *d = NULL;
    for(u8 i = 0; i < p->wlen; i++) // d scan each charset
    {
      d = p->idx[i];
      if(p->mode == CHAR)
        scan(&d[1], &d[0], MARK_ALL_CHAR, &p->word[i]);
      else
        scan(&d[1], &d[0], MARK_ALL_HEX, &p->word[i]);
      puts("");
    }
  }

  if(signo == SIGINT) // grab Ctrl-c
  {
    DPRINTF("\nreceived SIGINT\n");
    FILE *fp = fopen(".bf.save", "w");
    if(!fp) exit(-1);

    size_t n = fwrite(p->word, sizeof(char), p->wlen, fp); // dump
    fclose(fp); fp = NULL;

    DPRINTF("written %zub, @%p\n", n, p->word);
    cleanup(p);
    exit(0);
  }
}

void setup_signals(ctx *ctx)
{
  printf("send 'kill -USR1 %d' from another terminal to dump\n", getpid());

  p = ctx; // address p

  if(signal(SIGUSR1, sig_handler) == SIG_ERR) printf("\ncan't catch SIGUSR1\n");

  if(signal(SIGINT,  sig_handler) == SIG_ERR) printf("\ncan't catch SIGINT\n");
}