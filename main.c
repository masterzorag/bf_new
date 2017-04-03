/*
  bf_new
  -------
  2016, 2017, masterzorag@gmail.com
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
  #define PTIME printf( "%6.3f seconds", ((double)stopm - startm) /CLOCKS_PER_SEC);
#endif

static void change(ctx * const ctx, s8 *i)
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
  job.word = NULL;
  job.idx  = NULL;
  job.numw = 0;
  job.wlen = 0;
  job.out_m = DRY_RUN; // default

  job.work = parse_opt(argc, argv, &job);
  DPRINTF("parse_opt()\tret:%d\n", job.work);
  if(job.work)
  {
    cleanup(&job); exit(EXIT_FAILURE);
  }

  job.work = parse_file(&job);
  DPRINTF("parse_file()\tret:%d\n", job.work);
  if(job.work)
  {
    fprintf(stderr, "[E] Please recheck and pass a valid config file with -c\n");
    cleanup(&job); exit(EXIT_FAILURE);
  }

  u8 *p = NULL;
  if(1) // for verbose
  {
    #ifdef DEBUG
    DPRINTF("Report from main, mode %u\n", job.mode);
    for(u8 i = 0; i < job.wlen; i++)
    {
      p = job.idx[i];
      DPRINTF("idx %2d/%.2d @%p:\t%d items\n", i, job.wlen, p, p[0]);
      scan(&p[1], &p[0], HEXDUMP, NULL);
    }
    DPRINTF("%zub %zub\n", sizeof(ctx), sizeof(void*));
    DPRINTF("[I] Config passed, report data matrix:\n");
    #endif
  }

  /* report data matrix */
  if(job.out_m == DRY_RUN)
  {
    if(job.numw) fprintf(stderr, "[I] Requested %u words!\n", job.numw);

    dump_matrix(&job);
    // in this case word is moved into the last one!
    cleanup(&job); exit(0);
  }

  /* catch signals */
  setup_signals(&job);

  /* main process starts here */
  p = job.word;
  s8  n = job.wlen -1;
  u32 c = 1;

  while(1) // break it to exit(DONE)
  {
    #ifdef COUNT
    if(c == 1) START;
    if(c %COUNT == 0) // output only every COUNT attempt
    #endif
    {
      switch(job.out_m)
      {
        case BIN: bin2stdout(&job); break;                /* bin to STDOUT,   mode based */
        default:  scan(p, &job.wlen, PRINT, NULL); break; /* standard output, mode based */
      }

      #ifdef COUNT // print timing info
      {
        STOP;
        printf(" [%.2f/sec]", COUNT /(((double)stopm - startm) /CLOCKS_PER_SEC));
        START;
      }
      #endif

      switch(job.out_m)
      {
        case WORDLIST: printf("\n"); break; /* one-per-line output */
        case QUIET:    printf("\r"); break; /* on-same-line output */
      }
    }

    if(job.numw && job.numw == c) break;

    if(job.work == DUMP) {
      DPRINTF("\nReceived SIGUSR1\n"); dump_matrix(&job); // -USR1 output trigger
    }
    else if(job.work == INTR) {
      DPRINTF("\nReceived SIGINT\n"); break;
    }

    change(&job, &n);

    if(n < 0) break; // after that, we start increase word lenght!
    /*
      compute which one have to change and eventually continue
      something like n = find(word);
    */
    n = job.wlen -1; // reset n to rightmost one

    c++;             // and keep count
  }

  fflush(stdout), job.work = DONE;

  #ifndef DEBUG
  if(job.out_m == QUIET)
  #endif
    fprintf(stderr, "\nForged [%u] combinations\n", c);

  cleanup(&job);
  p = NULL;
  return 0;
}
