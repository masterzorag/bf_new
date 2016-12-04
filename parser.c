#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

u64 _x_to_u64(const s8 *hex){
        u64 t = 0, res = 0;
        u32 len = strlen(hex);
        char c;

        while(len--){
                c = *hex++;
                if(c >= '0' && c <= '9')        t = c - '0';
                else if(c >= 'a' && c <= 'f')   t = c - 'a' + 10;
                else if(c >= 'A' && c <= 'F')   t = c - 'A' + 10;
                else                            t = 0;
                res |= t << (len * 4);
        }
        return res;
}

u8 *_x_to_u8_buffer(const s8 *hex){
        u32 len = strlen(hex);
        if(len % 2 != 0) return NULL;   // (add sanity check in caller)

        s8 xtmp[3] = {0, 0, 0};
        u8 *res = (u8 *)malloc(sizeof(u8) * len);
        u8 *ptr = res;

        while(len--){
                xtmp[0] = *hex++;
                xtmp[1] = *hex++;
                *ptr++ = (u8) _x_to_u64(xtmp);
        }
        return res;
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

      case HEX:
        printf("%.2x ", *p);
        break;

      case DUMP:
        printf("%.2d/%.2d  %c\t0x%.2x %.3d\n", i, *l, *p, *p, *p);
        break;

      case FIND:
        if(*p == *dst) return i;
        break;

      case COUNT:
        if(*p == *dst)
        { //printf("%.2d/%.2d  %c  %c ret:%d\n", i, *l, *p, *dst, i);
          ret++;
        }
        break;

      case MARK: {
        if(p == dst) printf("%c[%d;%d;%dm", 0x1B, 2, 37, 40); // Mark dst
        printf("%c", *p);
        if(p == dst) printf("%c[%d;%d;%dm", 0x1B, 0, 0, 0);   // Revert back
        break; }

      case MARK_ALL: {
        if(*p == *dst) printf("%c[%d;%d;%dm", 0x1B, 2, 37, 40); // Mark dst
        printf("%c", *p);
        if(*p == *dst) printf("%c[%d;%d;%dm", 0x1B, 0, 0, 0);   // Revert back
        break; }

      default :
        break;
    }
    p++;
  }
  puts("");

  return ret;
}


s8 parse_file(char *filename, ctx *ctx)
{
  FILE *fp;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  fp = fopen(filename, "r");
  if(!fp) return -1;

  u8 max = ctx->word.len - 1;
  printf("reading max %d lines\n", max);

  ctx->cset = malloc(sizeof(set) * max);
  if(!ctx->cset) exit(EXIT_FAILURE);
  printf("malloc for %zu @%p\n", sizeof(set) * max, ctx->cset);

  u8 idx = 0;
  set *dst = NULL;

  while ((read = getline(&line, &len, fp)) != -1
  && (idx < max))
  {
    if(line[0] == '#') continue; // skip commented (#) lines

//  printf("Retrieved line of length %zu\n", read);
//  printf("%s %zu\n", line, strlen(line));

    dst = ctx->cset + idx;

    line[read] = '\n';
    dst->len = read -1;

    switch(ctx->mode)  // store
    {
      case CHAR:
        dst->data = (u8*)strndup(line, dst->len);
        break;

      case HEX:
        dst->len /= 2;
        dst->data = _x_to_u8_buffer(line);
        break;
    }
    if(!dst->data) return(-1);

    printf("set %2d/%.2d @%p:%d items\n", idx, max -1, dst, dst->len);
    scan(dst->data, &dst->len, DUMP, NULL);          // report

    idx++;
  }
  fclose(fp);

  /* Step 2
  - setup starting point
  - check for uniqueness
  */
  ctx->word.len = idx;
  *(ctx->word.data + idx) = '\n';

  dst = &ctx->word;
  set *d = NULL;
  for(u8 i = 0; i < idx; i++)
  {
    d = ctx->cset + i;
    *(dst->data + i) = *d->data; // fill the initial word

    // check if stored charset is unique
    printf("set %2d/%.2d @%p:%d items\n", i, idx, d, d->len);
    u8 *p = NULL;
    s8 count = 0;
    for(u8 j = 0; j < d->len; j++)  // scan each charset
    {
      p = (ctx->cset + i)->data + j;
      count = scan(d->data, &d->len, COUNT, p) +1;
//    printf(" %.2d/%.2d %.2x:%d items\n", j, d->len, *p, count);
      if(count > 1)
      {
        scan(d->data, &d->len, MARK_ALL, p);
        printf("[!] Charset %u must be unique, found %u repetitions!\n", i, count);
        return -1;
      }
    }
  }

  return idx;
}

