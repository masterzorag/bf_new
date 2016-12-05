/*
  bf_new
  -------
  2016, masterzorag@gmail.com
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"


void cleanup(ctx *item)
{
  if(item->cset)
  {
    for(u8 i = 0; i < item->word.len; i++)
      if(item->cset[i].data) free(item->cset[i].data);

    free(item->cset);
  }
  if(item->word.data) free(item->word.data);
}


void change(ctx *ctx, s8 *i)
{
  u8  *p = NULL;
  set *d = NULL;

  while(*i >= 0)
  {
    p = ctx->word.data + *i;
    d = ctx->cset + *i;

    if(*p == *(d->data + d->len -1)) // last one
    {
      *p = *d->data;     // change to first one
      *i -= 1;
    }
    else
    {
      s8 r = scan(d->data, &d->len, FIND, p); // find in charset
      *p = *(d->data + r + 1);                // change to next one
      break;
    }
  }
  p = NULL, d = NULL;
}


int main(int argc, char **argv)
{
  DPRINTF("[I] DEBUG build\n");

  u8 out = 1;     // 0/1 enables wordlist
  u8 marked = 1;  // 0/1 enables highligh

  ctx job;                     // working context
  job.cset = NULL;
  job.mode = CHAR;
  job.word.data = malloc(MAX_ELEM);
  if(!job.word.data) exit(EXIT_FAILURE);

  s8 ret = parse_opt(argc, argv, &job);
  DPRINTF("parse_opt() ret:%d\n", ret);
  if(ret)
  {
    cleanup(&job);
    exit(EXIT_FAILURE);
  }

  ret = parse_file(&job);
  DPRINTF("parse_file() ret:%d\n", ret);
  if(ret < 0)
  {
    printf("[E] Please recheck and pass a valid config file with -c\n");
    cleanup(&job);
    exit(EXIT_FAILURE);
  }


  if(1) // for verbose
  {
    DPRINTF("report from main, mode %u\n", job.mode);
    for(u8 i = 0; i < job.word.len; i++)
    {
      DPRINTF("set %2d/%.2d @%p:%d items\n", i, job.word.len, job.cset[i].data, job.cset[i].len);
      scan(job.cset[i].data, &job.cset[i].len, DUMP, NULL); puts(""); // report
    }
    scan(job.word.data, &job.word.len, job.mode, NULL); puts("");
  }
  getchar();


  if(0) // example
  {
    printf("%s %u\n", job.word.data, job.word.len);
    scan(job.word.data, &job.word.len, CHAR, NULL);
    scan(job.word.data, &job.word.len, DUMP, NULL);
    scan(job.word.data, &job.word.len, HEX,  NULL);
  }

  /* main process here */
  s8 n = job.word.len -1;
  u32 c = 1;
  while(1)  // break it to exit(COMPLETED)
  {
    //if(memcmp(job.word.data, "acqua", job.word.len) == 0) break;

    change(&job, &n);

    if(n < 0) break;

    if(1) // main output
    {
    //if(c %(1000000 *20) == 0)
      {
        if(marked) // MARKed output
        {
          if(job.mode == CHAR)
            /* marked output, for CHAR */
            scan(job.word.data, &job.word.len, MARK_CHAR, &job.word.data[(u8)n]);
          else
            /* marked output, for HEX */
            scan(job.word.data, &job.word.len, MARK_HEX, &job.word.data[(u8)n]);
        }
        else
        { /* standard output, mode based */
          scan(job.word.data, &job.word.len, job.mode, NULL);
        }

        if(1) // output type
        {
          if(out == 0) printf("\r");  /* on-the-same-line output */
          else         printf("\n");  /* one-per-line output */
        }
      }
    } // main output ends

    n = job.word.len -1; // reset n to rightmost one
    c++;                 // and keep count
  }

  printf("\n[%u]\n", c); // computed items
  cleanup(&job);
  return 0;
}
