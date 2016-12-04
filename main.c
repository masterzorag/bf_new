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
    {//printf("[%2x] ", *p);
      *p = *d->data;     // first one

      if(*i != 0)
        *i -= 1;
      else
        *i = -1;  // exit(COMPLETED)

    }
    else
    {//d = ctx->cset + *i;

      s8 r = scan(d->data, &d->len, FIND, p); // find in charset
      if(r == -1) // not found !
      {
        r = d->len; puts("exception"); getchar(); // trap exceptions
      }

      *p = *(d->data + r + 1);      // change to next one
      break;
    }
  }
  p = NULL, d = NULL;
}


int main(int argc, char **argv)
{
  if(!argv[1] || !argv[2])
  {
    printf("pass a lenght and a config file\n");
    exit(EXIT_FAILURE);
  }

  ctx job;                          // working context
  job.cset = NULL;
  job.mode = (argv[3]) ? HEX: CHAR; // setup mode
  job.word.len = atoi(argv[1]) + 1;
  job.word.data = malloc(job.word.len);
  //strncpy((char*)job.word.data, argv[1], job.word.len);
  if(!job.word.data) exit(EXIT_FAILURE);


  if(0) // example
  {
    u8 *p = NULL;
    p = _x_to_u8_buffer("c1c627e1638fdc8e24299bb041e4e23af4bb5427"); // store
    if(!p) exit(EXIT_FAILURE);
    u8 p_len = 20;
    scan(p, &p_len, HEX, NULL); // report
    free(p);
  }


  if(parse_file(argv[2], &job) < 0) // parse file
  {
    printf("pass a valid config file, for appropriate mode\n");
    cleanup(&job);
    exit(EXIT_FAILURE);
  }

  printf("report from main, mode %u\n", job.mode);
  for(u8 i = 0; i < job.word.len; i++)
  {
    scan(job.cset[i].data, &job.cset[i].len, DUMP, NULL); // report
  }
  getchar();

  if(0) // example
  {
    printf("%s %u\n", job.word.data, job.word.len);

    scan(job.word.data, &job.word.len, CHAR, NULL);
    scan(job.word.data, &job.word.len, DUMP, NULL);
    scan(job.word.data, &job.word.len, HEX,  NULL);
  }

  s8 n = job.word.len -1;
  u32 c = 0;
  while(1)
  {
    if(memcmp(job.word.data, "acqua", job.word.len) == 0) break;

    change(&job, &n);

    if(n < 0) break;  // exit(COMPLETED)

    // main output
    if(job.mode)
      scan(job.word.data, &job.word.len, HEX, NULL);
    else
    {
//    scan(job.word.data, &job.word.len, CHAR, NULL);
      scan(job.word.data, &job.word.len, MARK, &job.word.data[(u8)n]);
//      printf("%s %d\n", job.word.data, job.word.len);
    }

    // reset n to rightmost one
    n = job.word.len -1;
    c++;
  }

  printf("%d\n", c);
  cleanup(&job);
  return 0;
}
