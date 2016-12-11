#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "parser.h"

static u64 _x_to_u64(const s8 *hex)
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


static u8 *_x_to_u8_buffer(const s8 *hex)
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
        ctx->wlen = atoi(optarg); break;

      case 'x':
        ctx->mode = HEX; break;

      case 'c':
        ctx->word = (u8*)strdup(optarg); break;

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

  DPRINTF("lvalue = %d, xflag = %d, filename = %s\n", ctx->wlen, ctx->mode, ctx->word);

  for(idx = optind; idx < argc; idx++)
    printf("Non-option argument %s\n", argv[idx]);

  return 0;
}

#define MARKER_ON   printf("%c[%d;%d;%dm", 0x1B, 2, 37, 40); // Set MARK on
#define MARKER_OFF  printf("%c[%d;%d;%dm", 0x1B, 0, 0, 0);   // Revert back
s8 scan(const u8 *item, const u8 *l, const u8 mode, const u8 *dst)
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

      case IS_HEX: // DPRINTF("%.2d/%.2d  %2x %d\n", i, *l, *p, isxdigit(*p));
        if(!isxdigit(*p)) return i;
        break;

      case DUMP:
        printf("%.2d/%.2d  ", i, *l);
        if(isprint(*p)) printf("%c\t", *p);
        else            printf(".\t");
        printf("0x%.2x %.3d\n", *p, *p);
        break;

      case FIND:
        if(*p == *dst) return i +1;
        break;

      case COUNT:
        if(*p == *dst) ret++;
        break;

      case MARK_CHAR: {
        if(p == dst) MARKER_ON
        printf("%c", *p);
        if(p == dst) MARKER_OFF
        break; }

      case MARK_ALL: {
        if(*p == *dst) MARKER_ON
        printf("%c", *p);
        if(*p == *dst) MARKER_OFF
        break; }

      case MARK_HEX: {
        if(p == dst) MARKER_ON
        printf("%.2x", *p);
        if(p == dst) MARKER_OFF
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
  size_t size = 0;
  ssize_t read;

  fp = fopen((char *)ctx->word, "r");
  DPRINTF("fopen(%s) @%p\n", ctx->word, fp);
  if(!fp)
  {
    printf("Can't open file %s\n", ctx->word); return -1;
  }

  u8 max = MAX_ELEM;
  if(ctx->wlen)
  {
    max = ctx->wlen; DPRINTF("reading max %d lines\n", max);
  }

  // new index
  size = sizeof(u8*) * max;
  ctx->idx = malloc(size);
  if(!ctx->idx) return -1;
  DPRINTF("malloc  @%p %zub for %u idx\n", ctx->idx, size, max);

  /* Step 1
    - read one line at once
    - first check and store
  */
  u8 *dst = NULL;
  u8 idx   = 0;
  u8 len;
  while((read = getline(&line, &size, fp)) != -1
  && (idx < max)
  )
  {
    len = 0;
    if(line[0] == '#') continue; // skip commented (#) lines

    DPRINTF("%2d Retrieved line of length %zu: ", idx, read);
    line[read -1] = '\0'; // trim newline unconditionally
    DPRINTF("%s %zu-%zu\n", line, strlen(line), read);

    len = strlen(line);
    if(!len) break; // stop on empty line

    /* check and store, on requested mode */
    switch(ctx->mode)
    {
      case CHAR:
        dst = (u8*)strndup(line, len); // store
        break;

      case HEX: {
        if(len %2 || len < 2) // check against lenght
        {
          printf("[!] Line %u: lenght must be even! (is:%u)\n", idx +1, len);
          scan((u8 *)line, &len, CHAR, NULL); puts("");
          break;
        }

        s8 r = scan((u8 *)line, &len, IS_HEX, NULL); // check for hex digits
        if(r != -1)
        {
          printf("[!] Line %u: must contain hexadecimal digits only!\n", idx +1);
          scan((u8 *)line, &len, MARK_CHAR, (u8 *)&line[(u8)r]); puts("");
          break;
        }

        len /= 2;
        dst = _x_to_u8_buffer(line); // store
        break; }
    }
    if(!dst) return(-1);

    // new index data type:
    size = sizeof(u8) * (len +1);
    ctx->idx[idx] = malloc(size);
    if(!ctx->idx[idx]) return -1;

    memcpy(&ctx->idx[idx][1], dst, len); // store data
    ctx->idx[idx][0] = len;              // store lenght

    DPRINTF("idx %2d/%.2d @%p %zub: %d items\n", idx, max, ctx->idx[idx], size, len);
    //scan(&ctx->idx[idx][1], &ctx->idx[idx][0], DUMP, NULL);          // report

    idx++;
  }
  free(dst); dst = NULL;
  fclose(fp); fp = NULL;

  /* Step 2
    - setup tergets lenght
    - setup starting point
    - check for uniqueness
    - realloc sets buffers
  */
  ctx->wlen = idx;

  /* realloc buffers */
  size = sizeof(u8) * (idx +1);
  if(!realloc(ctx->word, size)) return -1;
  *(ctx->word + idx) = '\0';
  DPRINTF("realloc word\t@%p %zub: %u items\n", ctx->word, size, idx);

  if(idx != max)
  {
    size = sizeof(u8*) * idx;
    u8 **tmp = malloc(size);
    if(!tmp) return -1;
    DPRINTF("realloc idx\t@%p %zub\n", tmp, size);

    // swap buffers
    memcpy(tmp, ctx->idx, size);
    free(ctx->idx);
    ctx->idx = tmp;
  }


  u32 estimated = 1;
  for(u8 i = 0; i < idx; i++)
  {
    dst = ctx->idx[i];
    *(ctx->word + i) = dst[1]; // fill the initial word

    // check if stored charset is unique
    DPRINTF("idx %2d/%.2d @%p:%d items\n", i, idx, dst, dst[0]);
    u8 *p = NULL;
    s8 count = 0;
    for(u8 j = 1; j < dst[0] +1; j++) // scan each charset, from 1
    {
      p = ctx->idx[i] + j;
      count = scan(&dst[1], &dst[0], COUNT, p) +1;
      DPRINTF(" %.2d/%.2d\t%#.2x\tcount=%d\n", j, dst[0], *p, count);
      if(count > 1)
      {
        printf("[!] Line %u: must be unique, found %u repetitions!\n", i, count);
        scan(&dst[1], &dst[0], MARK_ALL, p); puts("");
        return -1;
      }
    }

    estimated *= dst[0];
  }
  DPRINTF("[I] Estimated %u combinations\n", estimated);

  return idx;
}

