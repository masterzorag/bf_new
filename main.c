#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tools.h"

typedef struct {
  u8 *data;
  u8 len;
} set;

typedef struct {
  set word;   // word
  set *cset;  // many charset
  u8 *R;      // Repetitions, max use
} ctx;


enum mode
{
  CHAR,
  DUMP,
  HEX,
//STORE_AS_CHAR,
  FIND
};


void cleanup(ctx *item)
{
  for(u8 i = 0; i < item->word.len; i++)
  {
    free(item->cset[i].data);
  }
  free(item->cset);
  free(item->word.data);
}

s8 scan(const u8 *item, const u8 *l, u8 mode, u8 *dst)
{
  u8 *p = (u8*)item;
  s8 ret = -1;

  for(u8 i = 0; i < *l; i++)
  {
    switch(mode)
    {
       case CHAR:
          printf("%c", *p);
          break;

       case DUMP:
          printf("%.2d/%.2d  %c  0x%.2x %.3d\n", i, *l, *p, *p, *p);
          break;

       case HEX:
          printf("%.2x ", *p);
          break;

/*       case STORE_AS_CHAR: {
          char t[2];
          strncpy(t, (char*)p, 1), t[1] = '\0';
          *(dst + i) = atoi(t);
          printf("%.2d/%.2d  %c  0x%.2x -> %.2d\n", i, *l, *p, atoi(t), *(dst + i));
          break; }
*/
       case FIND:
          if(*p == *dst)
          {
          //printf("%.2d/%.2d  %c  %c ret:%d\n", i, *l, *p, *dst, i);
            return i;
          }
          break;

       default :
          break;
    }
    p++;
  }
  puts("");

  return ret;
}

void change(ctx *ctx, u8 *i)
{
  u8  *p = NULL;
  set *d = NULL;

  while(*i >= 0)
  {
    p = ctx->word.data + *i;
    d = ctx->cset + *i;

    if(*p == *(d->data + d->len -1)) // last one
    {//printf("[%2x]", *p);
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
  p = NULL;
}

u8 parse_file(char *filename, ctx *ctx)
{
  FILE *fp;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  fp = fopen(filename, "r");
  if(!fp) exit(EXIT_FAILURE);

  u8 max = ctx->word.len;
  printf("reading max %d lines\n", max);

  ctx->cset = malloc(sizeof(set) * max);
  if(!ctx->cset) exit(EXIT_FAILURE);
  printf("malloc for %zu @%p\n", sizeof(set*) * max, ctx->cset);

  u8 idx = 0;
  set *dst = NULL;

  while ((read = getline(&line, &len, fp)) != -1
  && (idx < max))
  {
    if(line[0] == '#') continue; // skip commented (#) lines

//    printf("Retrieved line of length %zu\n", read);
//    printf("%s %zu\n", line, strlen(line));

    dst = ctx->cset + idx;

    dst->len = read -1;
    dst->data = (u8*)strndup(line, dst->len);

    printf("set %.2d/%.2d @%p:%d items\n", idx, max -1, dst, dst->len);
    scan(dst->data, &dst->len, DUMP, NULL);          // report

    idx++;
  }

  dst = NULL;
  fclose(fp);

  return idx;
}


int main(int argc, char **argv)
{
  ctx job;  // working context

  if(0)
  {
    u8 *p = NULL;
    p = _x_to_u8_buffer("c1c627e1638fdc8e24299bb041e4e23af4bb5427"); // store
    if(!p) exit(EXIT_FAILURE);
    u8 p_len = 20;
    scan(p, &p_len, HEX, NULL);          // report
    free(p);
  }

  if(argv[1])
  {
    job.word.len = strlen(argv[1]);
    job.word.data  = malloc(job.word.len);
    strncpy((char*)job.word.data, argv[1], job.word.len);


    if(!job.word.data) exit(-1);

  } else exit(EXIT_FAILURE);


  if(argv[2])
  {
    parse_file(argv[2], &job);
  } else {
    printf("pass a config file\n");
    exit(EXIT_FAILURE);
  }


  puts("report from main\n");
  for(u8 i = 0; i < job.word.len; i++)
  {
    scan(job.cset[i].data, &job.cset[i].len, DUMP, NULL);          // report
  }

/*
  cleanup(&job);
  exit(0);
*/

  printf("%s %u\n", job.word.data, job.word.len);

  scan(job.word.data, &job.word.len, CHAR, NULL);
  scan(job.word.data, &job.word.len, DUMP, NULL);
  scan(job.word.data, &job.word.len, HEX, NULL);

  u8 n = job.word.len -1;
  u32 c = 0;
  while(1)
  {

 //   scan(job.word, &job.word.len, CHAR, NULL);
    printf("%s %d\n", job.word.data, job.word.len);

    if(memcmp(job.word.data, "acqua", job.word.len) == 0) break;

    change(&job, &n);

    n = job.word.len -1;
    c++;
  }

  printf("%d\n", c);
  cleanup(&job);
  return 0;
}
