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
      *i -= 1;
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
  ctx job;                     // working context
  job.cset = NULL;
  job.mode = CHAR;
  job.word.data = malloc(256);       //strncpy((char*)job.word.data, argv[1], job.word.len);
  if(!job.word.data) exit(EXIT_FAILURE);

  s8 ret = parse_opt(argc, argv, &job); // printf("%d\n", ret);
  if(ret)
  {
    cleanup(&job);
    exit(EXIT_FAILURE);
  }
  getchar();

  ret = parse_file(&job);
  if(ret < 0) // parse file
  {
    printf("[E] Please recheck and pass a valid config file with -c\n");
    cleanup(&job);
    exit(EXIT_FAILURE);
  }

  printf("report from main, mode %u\n", job.mode);
  for(u8 i = 0; i < job.word.len; i++)
  {
    scan(job.cset[i].data, &job.cset[i].len, DUMP, NULL); // report
  }
  scan(job.word.data, &job.word.len, job.mode, NULL);
  getchar();

  if(0) // example
  {
    printf("%s %u\n", job.word.data, job.word.len);
    scan(job.word.data, &job.word.len, CHAR, NULL);
    scan(job.word.data, &job.word.len, DUMP, NULL);
    scan(job.word.data, &job.word.len, HEX,  NULL);
  }

  s8 n = job.word.len -1;
  u32 c = 1;
  while(1)
  {
    if(memcmp(job.word.data, "acqua", job.word.len) == 0) break;

    change(&job, &n);

    if(n < 0) break; // exit(COMPLETED)

    if(1) // main output
    {
//    scan(job.word.data, &job.word.len, CHAR, NULL);
      scan(job.word.data, &job.word.len, MARK, &job.word.data[(u8)n]);
//      printf("%s %d\n", job.word.data, job.word.len);
    }

    n = job.word.len -1; // reset n to rightmost one
    c++;
  }

  printf("\n[%d]\n", c);
  cleanup(&job);
  return 0;
}
