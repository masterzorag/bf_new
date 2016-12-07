#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "parser.h"

u64 _x_to_u64(const s8 *hex)
{
  u64 t = 0, res = 0;
  u32 len = strlen(hex);
  char c;

  while(len--){
    c = *hex++;
    if(c >= '0' && c <= '9')      t = c - '0';
    else if(c >= 'a' && c <= 'f') t = c - 'a' + 10;
    else if(c >= 'A' && c <= 'F') t = c - 'A' + 10;
    else                          t = 0;
    res |= t << (len * 4);
  }
  return res;
}


u8 *_x_to_u8_buffer(const s8 *hex)
{
  u32 len = strlen(hex); // printf("%s %u\n", hex, len);
//  if(len % 2 != 0) return NULL; // (add sanity check in caller)

  s8 xtmp[3] = {0, 0, 0};
  u8 *res = (u8 *)malloc(sizeof(u8) * len);
  u8 *ptr = res;

  while(len--){
    xtmp[0] = *hex++;
    xtmp[1] = *hex++;
    *ptr++  = (u8) _x_to_u64(xtmp);
  }
  return res;
}


s8 parse_opt(int argc, char **argv, ctx *ctx)
{
  int idx, c;
  opterr = 0;

  while((c = getopt(argc, argv, "xc:l:")) != -1)
    switch(c)
    {
      case 'l':
        ctx->word.len = atoi(optarg); break;

      case 'x':
        ctx->mode = HEX; break;

      case 'c':
        ctx->word.data = (u8*)strdup(optarg); break;

      case '?':
        if(optopt == 'c' || optopt == 'l')
          fprintf(stderr, "Option -%c requires an argument.\n", optopt);
        else if(isprint(optopt))
          fprintf(stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
        return -1;

      default:
        abort();
    }

  DPRINTF("lvalue = %d, xflag = %d, cvalue = %s\n", ctx->word.len, ctx->mode, ctx->word.data);

  for(idx = optind; idx < argc; idx++)
    printf("Non-option argument %s\n", argv[idx]);

  return 0;
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
        printf("%.2x", *p);
        break;

      case IS_HEX:
      //printf("%.2d/%.2d  %2x %d\n", i, *l, *p, isxdigit(*p));
        if(!isxdigit(*p)) return i;
        break;

      case DUMP:
        printf("%.2d/%.2d  ", i, *l);
        if(isprint(*p)) printf("%c\t", *p);
        else            printf(".\t");
        printf("0x%.2x %.3d\n", *p, *p);
        break;

      case FIND:
        if(*p == *dst) return i;
        break;

      case COUNT:
        if(*p == *dst) ret++;
        break;

      case MARK_CHAR: {
        if(p == dst) printf("%c[%d;%d;%dm", 0x1B, 2, 37, 40); // Marker ON
        printf("%c", *p);
        if(p == dst) printf("%c[%d;%d;%dm", 0x1B, 0, 0, 0);   // Revert back
        break; }

      case MARK_ALL: {
        if(*p == *dst) printf("%c[%d;%d;%dm", 0x1B, 2, 37, 40); // Marker ON
        printf("%c", *p);
        if(*p == *dst) printf("%c[%d;%d;%dm", 0x1B, 0, 0, 0);   // Revert back
        break; }

      case MARK_HEX: {
        if(p == dst) printf("%c[%d;%d;%dm", 0x1B, 2, 37, 40); // Marker ON
        printf("%.2x", *p);
        if(p == dst) printf("%c[%d;%d;%dm", 0x1B, 0, 0, 0);   // Revert back
        break; }

      default :
        break;
    }
    p++;
  }

  return ret;
}


s8 parse_file(ctx *ctx)
{
  FILE *fp;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  fp = fopen((char *)ctx->word.data, "r");
  DPRINTF("%s\n", ctx->word.data);
  if(!fp) return -1;


  u8 max = MAX_ELEM;
  if(ctx->word.len)
  {
    max = ctx->word.len;
    printf("reading max %d lines\n", max);
  }

  ctx->cset = malloc(sizeof(set) * max);
  if(!ctx->cset) return -1;
  //printf("malloc for %zu @%p\n", sizeof(set) * max, ctx->cset);

  /* Step 1
  - read one line at one
  - easy check and store
  */
  set *dst = NULL;
  u8 idx   = 0;

  while((read = getline(&line, &len, fp)) != -1
  && (idx < max)
  )
  {
    if(line[0] == '#') continue; // skip commented (#) lines

    DPRINTF("%2d Retrieved line of length %zu: ", idx, read);
    line[read -1] = '\n'; // trim newline unconditionally
    DPRINTF("%s", line);


    dst = ctx->cset + idx;
    dst->len = strlen(line) -1;
    dst->data = NULL;

    switch(ctx->mode) // check and store
    {
      case CHAR:
        dst->data = (u8*)strndup(line, dst->len);
        break;

      case HEX: {
        if(dst->len %2 || dst->len < 2)
        {
          printf("[!] Line %u: lenght must be even! (is:%u)\n", idx +1, dst->len);
          scan((u8 *)line, &dst->len, CHAR, NULL); puts("");
          break;
        }

        s8 r = scan((u8 *)line, &dst->len, IS_HEX, NULL);
        if(r != -1)
        {
          printf("[!] Line %u: must contain hexadecimal digits only!\n", idx +1);
          scan((u8 *)line, &dst->len, MARK_CHAR, (u8 *)&line[(u8)r]); puts("");
          break;
        }

        dst->len /= 2;
        dst->data = _x_to_u8_buffer(line);
        break; }
    }
    if(!dst->data) return(-1);

    //DPRINTF("set %2d/%.2d @%p: %d items\n", idx, max, dst, dst->len);
    //scan(dst->data, &dst->len, DUMP, NULL);          // report

    idx++;
  }
  fclose(fp);

  /* Step 2
  - setup starting point
  - check for uniqueness
  */
  ctx->word.len = idx;
  if(!realloc(ctx->word.data, sizeof(u8) * (idx +1))) return -1;
  *(ctx->word.data + idx) = '\0';

  dst = &ctx->word;
  set *d = NULL;
  for(u8 i = 0; i < idx; i++)
  {
    d = ctx->cset + i;
    *(dst->data + i) = *d->data; // fill the initial word

    // check if stored charset is unique
    //DPRINTF("set %2d/%.2d @%p:%d items\n", i, idx, d, d->len);
    u8 *p = NULL;
    s8 count = 0;
    for(u8 j = 0; j < d->len; j++)  // scan each charset
    {
      p = (ctx->cset + i)->data + j;
      count = scan(d->data, &d->len, COUNT, p) +1;
//    printf(" %.2d/%.2d %.2x:%d items\n", j, d->len, *p, count);
      if(count > 1)
      {
        printf("[!] Line %u: must be unique, found %u repetitions!\n", i, count);
        scan(d->data, &d->len, MARK_ALL, p); puts("");
        return -1;
      }
    }
  }

  return idx;
}

