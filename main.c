#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

typedef struct {
  u8 w_len;
  u8 c_len;
  u8 *word;
  u8 *cset;
} ctx;

void scan(const u8 *item, const u8 *l, u8 mode)
{
  u8 *d = (u8*)item;

  for(u8 i=0; i<*l; i++)
  {
    switch(mode)
    {
       case 0:            // quiet
          printf("%c", *d);
          break;

       case 1:
          printf("%d/%d %c %2x\n", i, *l, *d, *d);
          break;

       case 2:
          printf("%2x:", *d);
          break;

       default :
          break;
    }
    d++;
  }
  puts("");
}

void change(u8 *item, u8 *i)
{
  u8 *p = NULL;

  while(*i >= 0)
  {
    p = item + *i;

    if(*p == 0x7a) {  //last
      //printf("[%2x]", *p);

      *p = 0x61;   // first

      // 0x21-0x7f A-z
      // 0x61-0x3a a-9

      *i -= 1;
      if(*i < 0) exit(-1);

    } else {
      *p += 1;
      return;
    }

  }

  p = NULL;
}


int main(int argc, char **argv)
{
  ctx job;

  if(argv[1]) {

    job.w_len = strlen(argv[1]);
    job.word  = malloc(job.w_len);
    strncpy((char*)job.word, argv[1], job.w_len);

    if(!job.word) exit(-1);

  } else {
    exit(-1);
  }

  //printf("%lu %lu\n", sizeof(job), sizeof(u8));

  printf("%s %u\n", job.word, job.w_len);

  scan(job.word, &job.w_len, 0);
  scan(job.word, &job.w_len, 1);
  scan(job.word, &job.w_len, 2);

  u8 n = job.w_len -1;
  u32 c = 0;
  while(1) {
    change(job.word, &n);

//    scan(job.word, &job.w_len, 2);

    printf("%s %d\r", job.word, job.w_len);

    if(memcmp(job.word, "acqua", job.w_len) == 0) break;   

    n = job.w_len -1;
    c++;
  }

  printf("%d\n", c);

  free(job.word);
}
