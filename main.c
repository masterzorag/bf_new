/*
  bf_new
  -------
  2016, masterzorag@gmail.com
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"


#define COUNT  (1000000 *5) // enables timing info

#ifdef COUNT
  #include <time.h>
  clock_t startm, stopm;
  #define START if((startm = clock()) == -1){ printf("Error calling clock"); exit(1); }
  #define STOP  if((stopm  = clock()) == -1){ printf("Error calling clock"); exit(1); }
  #define PRINTTIME printf( "%6.3f seconds", ((double)stopm - startm) /CLOCKS_PER_SEC);
#endif

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

    if(*p == *(d->data + d->len -1)) // if p is the last in charset
    {
      *p = *d->data;                 // change to first one
      *i -= 1;
    }
    else
    {
      s8 r = scan(d->data, &d->len, FIND, p); // find p in charset
      *p = *(d->data + r + 1);                // change to next one
      break;
    }
  }
  p = NULL, d = NULL;
}


int main(int argc, char **argv)
{
  DPRINTF("[I] DEBUG build\n");

  u8 out    = 0;  // 0/1 enables wordlist
  u8 marked = 0;  // 0/1 enables highligh

  ctx job;        // working context
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


  set *p = NULL;
  if(1) // for verbose
  {
    #ifdef DEBUG
    DPRINTF("report from main, mode %u\n", job.mode);
    for(u8 i = 0; i < job.word.len; i++)
    {
      p = job.cset + i;
      DPRINTF("set %2d/%.2d @%p %zub\n", i, job.word.len, p->data, sizeof(u8) * p->len);
      scan(p->data, &p->len, DUMP, NULL); puts(""); // report
    }
    #endif

    /* report the very first word composed, our starting point */
    p = &job.word;
    scan(p->data, &p->len, job.mode, NULL); puts("");
  }
  DPRINTF("%zub %zub\n", sizeof(set), sizeof(u8));
  getchar();


  if(0) // example
  {
    printf("%s %u\n", p->data, p->len);
    scan(p->data, &p->len, CHAR, NULL);
    scan(p->data, &p->len, DUMP, NULL);
    scan(p->data, &p->len, HEX,  NULL);
  }

  /* main process here */
  s8 n = p->len -1;
  u32 c = 1;

  while(1) // break it to exit(COMPLETED)
  {
    //if(memcmp(job.word.data, "acqua", job.word.len) == 0) break;

    change(&job, &n);

    if(n < 0) break; // after that, we start increase word lenght!

    if(1) // main output
    {
      #ifdef COUNT
      if(c %COUNT == 0) //output only every COUNT attempt
      #endif
      {
        if(marked) // MARKed output
        {
          if(job.mode == CHAR)
            /* marked output, for CHAR mode */
            scan(p->data, &p->len, MARK_CHAR, &job.word.data[(u8)n]);
          else
            /* marked output, for HEX mode */
            scan(p->data, &p->len, MARK_HEX, &job.word.data[(u8)n]);
        }
        else
        { /* standard output, mode based */
          scan(p->data, &p->len, job.mode, NULL);
        }


        #ifdef COUNT
        if(1) // print timing info
        {
          STOP;
          PRINTTIME;
          printf(" [%.2f/sec]", COUNT /(((double)stopm - startm) /CLOCKS_PER_SEC));
          START;
        }
        #endif

        if(1) // output type
        {
          if(out == 0) printf("\r");  /* on-the-same-line output */
          else         printf("\n");  /* one-per-line output */
        }
      }
    } // end main output

    n = p->len -1;       // reset n to rightmost one
    c++;                 // and keep count
  }

  printf("\n[%u]\n", c); // computed items
  cleanup(&job);
  p = NULL;
  return 0;
}
