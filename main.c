#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"


typedef struct {
  u8 w_len;   // word lenght
  u8 c_len;   // charset lenght
  u8 *word;
  u8 *cset;
  u8 *R;      // Repetitions, max use
} ctx;


enum mode
{
  CHAR,
  DUMP,
  HEX,
  STORE_IN
};

void scan(const u8 *item, const u8 *l, u8 mode, u8 *dst)
{
  u8 *p = (u8*)item;

  for(u8 i=0; i<*l; i++)
  {
    switch(mode)
    {
       case 0:    // output as CHAR
          printf("%c", *p);
          break;

       case 1:    // DUMP values
          printf("%.2d/%.2d  %c  0x%.2x %.3d\n", i, *l, *p, *p, *p);
          break;

       case 2:    // output as HEX
          printf("%.2x ", *p);
          break;

       case 3: {  // STORE_IN dst[c_len]
          char t[2];
          strncpy(t, (char*)p, 1), t[1] = '\0';
          *(dst + i) = atoi(t);
          printf("%.2d/%.2d  %c  0x%.2x -> %.2d\n", i, *l, *p, atoi(t), *(dst + i));
          break; }

       default :
          break;
    }
    p++;
  }
  puts("");
}

void change(ctx *ctx, u8 *i)
{
  u8 *p = NULL;

  while(*i >= 0)
  {
    p = ctx->word + *i;

    if(*p == 0x7a) {  //last    //ctx->c_len
      //printf("[%2x]", *p);

      *p = 0x61;   // first     //ctx->cset

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

void parse_file(char *filename, ctx *ctx)
{
  FILE *fp;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  fp = fopen(filename, "r");
  if(!fp) exit(EXIT_FAILURE);

  u8 idx = 0;
  while ((read = getline(&line, &len, fp)) != -1)
  {
    if(line[0] == '#') continue;  // skip commented (#) lines

    switch(idx)
    {
      // setup charset
      case 0:

        ctx->c_len = strlen(line) /* trim newline unconditionally */ -1;
        ctx->cset  = malloc(ctx->c_len);
        if(!ctx->cset) exit(-1);

        strncpy((char*)ctx->cset, line, ctx->c_len); // store
        ctx->cset[ctx->c_len] = '\0';

        scan(ctx->cset, &ctx->c_len, CHAR, NULL);    // report

        idx++;
        break;

      // setup Repetitions, max use
      case 1:
        ctx->R  = malloc(ctx->c_len);
        if(!ctx->R) exit(-1);

        scan((u8*)line, &ctx->c_len, STORE_IN, ctx->R); // store

        scan(ctx->R, &ctx->c_len, DUMP, NULL);          // report

        idx++;
        break;

      case 2:
        break;

      default :
        break;

    }

    printf("Retrieved line of length %zu :\n", read);
    printf("%s %zu", line, strlen(line));

  }
  free(line);
  fclose(fp);
}



int main(int argc, char **argv)
{
  ctx job;

  if(argv[2])
  {
    parse_file(argv[2], &job);

    free(job.cset);
    free(job.R);
    exit(0);
  }


  if(argv[1])
  {
    job.w_len = strlen(argv[1]);
    job.word  = malloc(job.w_len);
    strncpy((char*)job.word, argv[1], job.w_len);

    if(!job.word) exit(-1);

  } else {
    exit(-1);
  }

  printf("%s %u\n", job.word, job.w_len);

  scan(job.word, &job.w_len, CHAR, NULL);
  scan(job.word, &job.w_len, DUMP, NULL);
  scan(job.word, &job.w_len, HEX, NULL);

  u8 n = job.w_len -1;
  u32 c = 0;
  while(1) {
    change(&job, &n);

//    scan(job.word, &job.w_len, CHAR);
    printf("%s %d\r", job.word, job.w_len);

    if(memcmp(job.word, "acqua", job.w_len) == 0) break;

    n = job.w_len -1;
    c++;
  }

  printf("%d\n", c);

  free(job.word);

  return 0;
}
