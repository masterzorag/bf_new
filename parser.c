#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "parser.h"


static u8 opmode; // CHAR | HEX, for scan()

static u64 _x_to_u64(const char *hex)
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


static u8 *_x_to_u8_buffer(const char *hex)
{
  u32 len = strlen(hex); // printf("%s %u\n", hex, len);
//  if(len % 2 != 0) return NULL; // (add sanity check in caller)

  char xtmp[3] = {0, 0, 0};
  u8 *res = (u8*)malloc(sizeof(u8) * len);
  u8 *ptr = res;

  while(len--){
    xtmp[0] = *hex++;
    xtmp[1] = *hex++;
    *ptr++  = (u8) _x_to_u64(xtmp);
  }
  return res;
}


// wrapper to bin write to STDOUT
void bin2stdout(ctx *p)
{
  ssize_t n = write(STDOUT_FILENO, p->word, p->wlen);

  if(n != p->wlen) // trap exceptions on interruption
  {
    printf("data interrupted"); getchar();
  }
}


// actually we save on cleanup()
static size_t save(ctx *p)
{
  FILE *fp = fopen(".bf.save", "w");
  if(!fp) return 0;

  size_t n = fwrite(p->word, sizeof(char), p->wlen, fp); // dump
  fclose(fp); fp = NULL;

  DPRINTF("written %zub, @%p\n", n, p->word);
  return n;
}


// wrapper to release allocated ctx memory
void cleanup(ctx *p)
{
  /* save to resume on next run
     note: if(job.done), word is turned in the first one by change()! */
  if(p->wlen) save(p);

  /* clean free()s */
  if(p->word) free(p->word);
  if(p->idx)
  {
    for(u8 i = 0; i < p->wlen; i++)
      if(p->idx[i]) free(p->idx[i]);

    if(p->idx) free(p->idx);
  }
}


static void help()
{
  printf("\n\
  bruteforge %s, an advanced data generator\n\
  -----------\n\n\
  -c  pass a valid config file\n\
  -l  set word lenght (default = max possible)\n\
  -x  use HEX mode    (default = CHAR)\n\
  -b  binary output   (default = no)\n\n\
                   2017, masterzorag@gmail.com\n\
  run test:\n\
  $ ./bf -c test/test_2\n\
  $ ./bf -c test/test_3 -x\n\n", VERSION);
}


s8 parse_opt(int argc, char **argv, ctx *ctx)
{
  if(argc == 1){ help(); return -1;}

  int idx, c;
  opterr = 0;
  opmode = CHAR;

  while((c = getopt(argc, argv, "c:l:bhx")) != -1)
    switch(c)
    {
      case 'c': ctx->word = (u8*)strdup(optarg); break;
      case 'l': ctx->wlen = atoi(optarg); break;

      case 'x': ctx->mode = opmode = HEX; break;
      case 'b': ctx->bin  = 1; break;
      case 'h': help(); return -1;

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

  DPRINTF("wlen = %d, xflag = %d, filename = %s, bin = %u\n", ctx->wlen, ctx->mode, ctx->word, ctx->bin);

  for(idx = optind; idx < argc; idx++)
    printf("Non-option argument %s\n", argv[idx]);

  return 0;
}


s8 scan(const u8 *item, const u8 *l, const u8 smode, const u8 *dst)
{
  u8 *p = (u8*)item;
  s8 ret = -1;

  for(u8 i = 0; i < *l; i++)
  {
    switch(smode)
    {
      case PRINT:
        if(opmode == CHAR)
          printf("%c", *p);
        else
          printf("%.2x", *p);
        break;

      case IS_HEX: // DPRINTF("%.2d/%.2d  %2x %d\n", i, *l, *p, isxdigit(*p));
        if(!isxdigit(*p)) return i;
        break;

      case HEXDUMP:
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

      case MARK_ONE: {
        if(p == dst) MARKER_ON
        if(opmode == CHAR)
          printf("%c", *p);
        else
          printf("%.2x", *p);
        if(p == dst) MARKER_OFF
        break; }

      case MARK_ALL: {
        if(*p == *dst) MARKER_ON
        if(opmode == CHAR)
          printf("%c", *p);
        else
          printf("%.2x", *p);
        if(*p == *dst) MARKER_OFF
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
  char *line  = NULL;
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

  size = sizeof(u8*) * max;
  ctx->idx = malloc(size); // new index data type
  if(!ctx->idx) return -1;
  DPRINTF("malloc @%p %zub for %u idx\n", ctx->idx, size, max);

  /* Step 1
    - read one line at once
    - first check and store
  */
  u8 *p = NULL;
  u8  i = 0, len;
  while((read = getline(&line, &size, fp)) != -1
  && (i < max)
  )
  {
    if(line[0] == '#') continue; // skip commented (#) lines

    DPRINTF("%2d Retrieved line of length %zu: ", i, read);
    line[read -1] = '\0'; // trim newline unconditionally
    len = strlen(line);
    DPRINTF("%s %u-%zu\n", line, len, read);

    if(!len) break; // stop read, on empty line

    switch(ctx->mode) /* check and store, on requested mode */
    {
      case CHAR:
        p = (u8*)strndup(line, len); // store
        break;

      case HEX: {
        if(len %2 || len < 2) // check against lenght
        {
          printf("[!] Line %u: lenght must be even! (is:%u)\n%s\n", i +1, len, line);
          break;
        }

        s8 r = scan((u8*)line, &len, IS_HEX, NULL); // check for hex digits
        if(r != -1)
        {
          printf("[!] Line %u: must contain hexadecimal digits only for HEX mode!\n", i +1);
          scan((u8*)line, &len, MARK_ONE, (u8*)&line[(u8)r]); puts("");
          break;
        }

        len /= 2;
        p = _x_to_u8_buffer(line); // store
        break; }
    }
    if(!p) return(-1);

    // alloc each new index type:
    size = sizeof(u8) * (len +1);
    ctx->idx[i] = malloc(size);
    if(!ctx->idx[i]) return -1;

    memcpy(&ctx->idx[i][1], p, len); // store data
    ctx->idx[i][0] = len;            // store lenght

    DPRINTF("idx %2d/%.2d @%p %zub: %d items\n", i, max, ctx->idx[i], size, len);
    //scan(&ctx->idx[i][1], &ctx->idx[i][0], HEXDUMP, NULL);          // report

    i++;
  }
  free(line); line = NULL;
  fclose(fp); fp = NULL;
  free(p); p = NULL;

  /* Step 2 */
  ctx->wlen = i; // 1. setup tergets lenght

  if(1) // 2. realloc buffers
  {
    size = sizeof(u8) * (i +1);
    if(!realloc(ctx->word, size)) return -1; // reuse ctx->word
    *(ctx->word + i) = '\0';
    DPRINTF("realloc word\t@%p %zub: %u items\n", ctx->word, size, i);

    if(i != max)
    {
      size = sizeof(u8*) * i;
      u8 **t = malloc(size);
      if(!t) return -1;
      DPRINTF("realloc idx\t@%p %zub\n", t, size);
      memcpy(t, ctx->idx, size);
      free(ctx->idx);
      ctx->idx = t; // swap buffers, for indexes
    }
  }

  if(1) /* Step 3 */
  {
    u32 estimated = 1;
    for(i = 0; i < ctx->wlen; i++)
    {
      p = ctx->idx[i]; // address each charset

      *(ctx->word + i) = p[1]; // 1. fill the initial word
      DPRINTF("idx %2d/%.2d @%p:%d items\n", i, ctx->wlen, p, p[0]);
      u8 *d = NULL;
      s8 count = 0;
      for(u8 j = 1; j < p[0] +1; j++) // d scan each charset, from 1
      {
        d = ctx->idx[i] + j;
        count = scan(&p[1], &p[0], COUNT, d) +1;
        //DPRINTF(" %.2d/%.2d\t%#.2x\tcount=%d\n", j, p[0], *d, count);

        if(count > 1) // 2. check if stored charset is unique
        {
          printf("[!] Line %u: must be unique, found %u repetitions!\n", i +1, count);
          scan(&p[1], &p[0], MARK_ALL, d); puts("");
          return -1;
        }
      }
      estimated *= p[0]; // 3. count combinations
      d = NULL;
    }
    DPRINTF("[I] Estimated %u combinations\n", estimated);
  }

  return max;
}

