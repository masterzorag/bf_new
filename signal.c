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

static void dump_v2()
{
  u8 row = 0, max = 1, *d = NULL;
  while(row < max)
  {
    for(u8 i = 0; i < p->wlen; i++) // d scan each charset
    {
      d = p->idx[i];
      if(d[0] > max) max = d[0]; // update max if needed

      if(row < d[0])
      {
        //if(d[row +1] == p->word[i]) MARKER_ON;

        if(p->mode == CHAR)
          printf("%c ", d[row +1]);
        else
          printf("%.2x", d[row +1]);

        //if(d[row +1] == p->word[i]) MARKER_OFF;
      }
      else
        printf("  ");
    }
    puts(""); row++;
  }
  sleep(2);
}


static void sig_handler(int signo) // use p to access data
{
  if(signo == SIGUSR1) // uses kill -USR
  {
    DPRINTF("received SIGUSR1\n");
    // dump the ctx, marking current item in charsets
    dump_v2();
  }

  if(signo == SIGINT) // grab Ctrl-c
  {
    DPRINTF("\nreceived SIGINT\n");
    FILE *fp = fopen(".bf.save", "w");
    if(!fp) exit(-1);

    size_t n = fwrite(p->word, sizeof(char), p->wlen, fp); // dump
    fclose(fp); fp = NULL;

    DPRINTF("written %zub, @%p\n", n, p->word);
    p->done = 1;
  }
}


void setup_signals(ctx *ctx)
{
  DPRINTF("send 'kill -USR1 %d' from another terminal to dump\n", getpid());

  p = ctx; // address p

  if(signal(SIGUSR1, sig_handler) == SIG_ERR) printf("\ncan't catch SIGUSR1\n");

  if(signal(SIGINT,  sig_handler) == SIG_ERR) printf("\ncan't catch SIGINT\n");
}