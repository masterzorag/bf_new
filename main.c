/*
  bf_new
  -------
  2016, masterzorag@gmail.com
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "signal.h"


//#define COUNT  (1000000 *5) // enables timing info


#ifdef COUNT
  #include <time.h>
  clock_t startm, stopm;
  #define START if((startm = clock()) == -1){ printf("Error calling clock"); exit(1); }
  #define STOP  if((stopm  = clock()) == -1){ printf("Error calling clock"); exit(1); }
  #define PRINTTIME printf( "%6.3f seconds", ((double)stopm - startm) /CLOCKS_PER_SEC);
#endif

static void change(ctx *ctx, s8 *i)
{
  u8 *p, *d;

  while(*i >= 0)
  {
    p = ctx->word + *i;
    d = ctx->idx[(u8)*i];

    //DPRINTF("change word[%d] : %c -> @%p : %d items\n", *i, *p, d, d[0]);
    //scan(&d[1], &d[0], DUMP, NULL);          // report

    if(*p == *(d + d[0])) // if p is the last in charset
    {
      *p = d[1];          // change to first one
      *i -= 1;
    }
    else
    {
      s8 r = scan(&d[1], &d[0], FIND, p); // find p in charset
      //DPRINTF("find %#2x: r=%d -> %c\n", *p, r, *(d + r + 1));
      *p = *(d + r + 1); // change to next one
      break;
    }
  }
  p = d = NULL;
}


int main(int argc, char **argv)
{
  DPRINTF("[I] DEBUG build\n");

  /* working context init */
  ctx job;
  job.mode = CHAR;
  job.idx  = NULL;
  job.done = 0;
  job.wlen = 0;
  job.word = malloc(MAX_ELEM);
  if(!job.word) exit(EXIT_FAILURE);

  job.out_m = DRY_RUN; // default

  s8 ret = parse_opt(argc, argv, &job);
  DPRINTF("parse_opt() ret:%d\n", ret);
  if(ret)
  {
    cleanup(&job); exit(EXIT_FAILURE);
  }

  ret = parse_file(&job);
  DPRINTF("parse_file() ret:%d\n", ret);
  if(ret < 0)
  {
    printf("[E] Please recheck and pass a valid config file with -c\n");
    cleanup(&job); exit(EXIT_FAILURE);
  }


  u8 *p = NULL;
  if(1) // for verbose
  {
    #ifdef DEBUG
    DPRINTF("report from main, mode %u\n", job.mode);
    for(u8 i = 0; i < job.wlen; i++)
    {
      p = job.idx[i];
      DPRINTF("idx %2d/%.2d @%p : %d items\n", i, job.wlen, p, p[0]);
      scan(&p[1], &p[0], HEXDUMP, NULL);
    }
    #endif
    DPRINTF("%zub %zub\n", sizeof(ctx), sizeof(void*));
  }

  p = job.word;
  switch(job.out_m) /* report the very first word composed, our starting point */
  {
    case BIN: bin2stdout(&job); break;
    default:  scan(p, &job.wlen, PRINT, NULL); puts(""); break;
  }

  if(job.out_m == DRY_RUN) // report data matrix
  {
    dump_matrix(&job);
    // in this case word is turned into the last one, report again
    scan(p, &job.wlen, PRINT, NULL); puts("");

    cleanup(&job); exit(0);
  }
  //else getchar(); // ready, user pause

  /* catch signals */
  setup_signals(&job);

  /* main process here */
  s8 n = job.wlen -1;
  u32 c = 1;

  while(job.done != DONE) // break it to exit(COMPLETED)
  {
    //if(memcmp(job.word, "acqua", job.wlen) == 0) break;

    change(&job, &n);

    if(n < 0) break; // after that, we start increase word lenght!

    /*
      compute which one have to change and eventually continue
      something like n = find(word);
    */

    if(1) // main output
    {
      #ifdef COUNT
      if(c %COUNT == 0) // output only every COUNT attempt
      #endif
      {
        switch(job.out_m)
        {
//        case MARKED: scan(p, &job.wlen, MARK_ONE, &p[(u8)n]); break;
          case BIN: bin2stdout(&job); break; /* bin to STDOUT, mode based */
          default:  scan(p, &job.wlen, PRINT, NULL); break; /* standard output, mode based */
        }

        #ifdef COUNT // print timing info
        {
          STOP;
          PRINTTIME;
          printf(" [%.2f/sec]", COUNT /(((double)stopm - startm) /CLOCKS_PER_SEC));
          START;
        }
        #endif

        switch(job.out_m)
        {
          case WORDLIST: printf("\n"); break; /* one-per-line output */
          case QUIET:    printf("\r"); break; /* on-the-same-line output */
          default: break;
        }
        if(job.done == DUMP) dump_matrix(&job); // SIGUSR1 output
      }
    } // end main output

    n = job.wlen -1; // reset n to rightmost one
    c++;             // and keep count
  }

  if(job.out_m == QUIET) printf("\nforged [%u] combinations\n", c);

  cleanup(&job);
  p = NULL;
  return 0;
}
