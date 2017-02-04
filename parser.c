#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
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
    fprintf(stderr, "data interrupted"); getchar();
  }
}


// actually we resume on parse_file()
static u8 resume(ctx *p)
{
  u8 res = 0;
  u8 *t = calloc(p->wlen, sizeof(u8));

  FILE *fp = fopen(FILESAVE, "r");
  if(!t || !fp) return 0;

  fseek(fp, 0L, SEEK_END); // check on file size
  long r = ftell(fp);
  rewind(fp);
  if(r != p->wlen) goto END;

  size_t n = fread(t, sizeof(char), (size_t)p->wlen, fp); // read

  if(n != p->wlen) goto END; // check on readed size

  if(!memcmp(t, p->word, (size_t)p->wlen)) goto END; // check on readed content

  for(u8 i = 0; i < p->wlen; i++) // validate t against indexes
  {
    if(scan(&p->idx[i][1], &p->idx[i][0], FIND, &t[i]) < 1) goto END;
  }

  memcpy(p->word, t, (size_t)p->wlen); // passed, resume from saved
  res = n;

END:
  fclose(fp), fp = NULL;
  free(t), t = NULL;
  return res;
}


// actually we save on cleanup()
static size_t save(ctx *p)
{
  FILE *fp = fopen(FILESAVE, "w");
  if(!fp) return 0;

  size_t n = fwrite(p->word, sizeof(unsigned char), p->wlen, fp); // dump
  fclose(fp), fp = NULL;

  DPRINTF("Written %zub from %p to '%s'\n", n, p->word, FILESAVE);
  return n;
}


// wrapper to release allocated ctx memory
void cleanup(ctx *p)
{
  /* save last generated, to resume on next run
     note: in a completed run, word is turned into the first one by change()! */
  if(p->work == DONE) save(p);

  /* clean free()s */
  if(p->word) free(p->word), p->word = NULL;

  if(p->idx)
  {
    DPRINTF("**idx @%p\n", p->idx);
    for(u8 i = 0; i < p->wlen; i++)
    {
      if(p->idx[i])
      {
        DPRINTF("%.2d-@%p -> @%p %hhu\n", i, &p->idx[i], p->idx[i], *p->idx[i]);
        free(p->idx[i]), p->idx[i] = NULL;
      }
    }
    DPRINTF("**idx @%p %p\n", p->idx, *p->idx);
    free(p->idx), p->idx = NULL;
  }
}


static void help()
{
  printf("\n\
  bruteforge, a selective data combinator.\n\
  -----------\n\
  \n\
  -c  pass a valid config file\n\
  -l  set word lenght (default = max possible)\n\
  -n  generate just n words\n\
  -x  use HEX mode    (default = CHAR)\n\
  \n\
  Output:\n\
  -b  binary output\n\
  -w  print out wordlist\n\
  -q  quiet run\n\
  \n\
  v%s, 2017, masterzorag@gmail.com\n\
  \n\
  run test:\n\
  $ ./bf -c test/test_2\n\
  $ ./bf -c test/test_3 -x\n\n", VERSION);
}


/*
  unset LANG LC_ALL; LC_CTYPE=en_US.iso88591 export LC_CTYPE
*/
/* Locale initiator, -1 ok, 0 on error */
static s8 setup_locale(void)
{
  char *res;

  /* Get the name of the current locale. */
  res = setlocale (LC_ALL, "");   // set user default
  res = setlocale (LC_ALL, NULL); // query result
  DPRINTF("LC_ALL: %s\n", res);
  if(!res) { printf("error"); return 0; }

  res = setlocale (LC_CTYPE, "en_US.iso88591"); // try to set codepage
  if(!res) { printf("[E] error setting locale!"); return 0; }

  res = setlocale (LC_CTYPE, NULL); // to check, now query
  DPRINTF("LC_CTYPE: %s\n", res);
  if(!res) { printf("[E] error setting locale!"); return 0; }

  return -1;
}


/* Option parser, 0 ok, -1 on error */
s8 parse_opt(int argc, char **argv, ctx *ctx)
{
  if(argc == 1){ help(); return -1;}

  if(!setup_locale()) return -1;

  int idx, c, flag_err = 0;
  opterr = 0;
  opmode = CHAR;

  while((c = getopt(argc, argv, "c:l:n:xbwqh")) != -1)
    switch(c)
    {
      case 'h': help(); return -1;
      case 'c': ctx->word = (u8*)strdup(optarg); break;
      case 'x': ctx->mode = opmode = HEX; break;

      case 'l':
      case 'n':
      {
        signed int t = atoi(optarg);
        if(t < 1) { optopt = c; goto ERR; }

        if(c == 'l') ctx->wlen = t;
        else         ctx->numw = t;
        break;
      }
      case 'b': flag_err++, ctx->out_m = BIN;      break;
      case 'w': flag_err++, ctx->out_m = WORDLIST; break;
      case 'q': flag_err++, ctx->out_m = QUIET;    break;

      case '?':
ERR:
        if(optopt == 'c'
        || optopt == 'l'
        || optopt == 'n')
          fprintf(stderr, "Option -%c requires an argument.\n", optopt);
        else if(isprint(optopt))
          fprintf(stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
        return -1;

      default:
        abort();
    }

  /* accept just one output flag! */
  if(flag_err > 1) { fprintf(stderr, "[E] flags -b, -w, -q, are mutually exclusive\n"); return -1; }

  DPRINTF("wlen = %d, mode = %d, filename = %s, bin = %u, n = %d\n",
    ctx->wlen, ctx->mode, ctx->word, ctx->out_m, ctx->numw);

  for(idx = optind; idx < argc; idx++)
    fprintf(stderr, "Non-option argument %s\n", argv[idx]);

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
        if(opmode == CHAR) printf("%c",   *p);
        else               printf("%.2x", *p);
        break;

      case IS_HEX: if(!isxdigit(*p)) return i;
        break;

      case FIND:   if(*p == *dst) return i +1;
        break;

      case COUNT:  if(*p == *dst) ret++;
        break;

      case HEXDUMP:
        printf("%.2d/%.2d  ", i, *l);
        if(isprint(*p)) printf("%c\t", *p);
        else            printf(".\t");
        printf("0x%.2x %.3d\n", *p, *p);
        break;

      case MARK_ONE: {
        if(p == dst) MARKER_ON
        if(opmode == CHAR) printf("%c",   *p);
        else               printf("%.2x", *p);
        if(p == dst) MARKER_OFF
        break; }

      case MARK_ALL: {
        if(*p == *dst) MARKER_ON
        if(opmode == CHAR) printf("%c",   *p);
        else               printf("%.2x", *p);
        if(*p == *dst) MARKER_OFF
        break; }

      default: break;
    }
    p++;
  }

  return ret;
}


/* Marked matrix dumper
   used just on DRY_RUN, or triggered by -USR1 signal */
void dump_matrix(ctx *p)
{
  scan(p->word, &p->wlen, PRINT, NULL); puts(""); // report current

  // setup a bounder line
  size_t max = sizeof(char) * p->wlen *3;
  char *t = calloc(max +1,  sizeof(char)); // bounder line
  u8  *t2 = calloc(p->wlen, sizeof(u8));   // to store last one

  if(!t || !t2) fprintf(stderr, "error");

  memset(t, '-', max);
  fprintf(stderr, "%s\n", t);

  max = 1;
  u8 row = 0, *d = NULL;
  while(row < max)
  {
  //DPRINTF("row %d, max %zu\n", row, max);
    for(u8 i = 0; i < p->wlen; i++) // d scan each charset
    {
      d = p->idx[i];
      if(d[0] > max) max = d[0]; // update max if needed

      if(row < d[0]) // we have an item
      {
        if(d[row +1] == p->word[i]) MARKER_ON;

        if(p->mode == CHAR) fprintf(stderr, " %c ",  d[row +1]);
        else                fprintf(stderr, "%.2x ", d[row +1]);

        if(d[row +1] == p->word[i]) MARKER_OFF;
      }
      else fprintf(stderr, "   ");

      if(p->out_m == DRY_RUN) t2[i] = *(d + d[0]); // compose last possible
    }
    fprintf(stderr, "\n"); row++;
  }
  fprintf(stderr, "%s\n", t); // close with another bounder line
  free(t), t = NULL;

  if(p->out_m == DRY_RUN) // report max possible
  {
  //DPRINTF("%p %p %p\n", p, p->word, &p->word);
    free(p->word), p->word = t2;
    scan(p->word, &p->wlen, PRINT, NULL); puts(""); // report last
  }
  else free(t2), t2 = NULL;


  if(p->work == DUMP) p->work = 0; // revert flag back to working
}


s8 parse_file(ctx *ctx)
{
  FILE *fp = fopen((char *)ctx->word, "r");
  DPRINTF("fopen(%s) @%p\n", ctx->word, fp);
  if(!fp)
  {
    fprintf(stderr, "Can't open file %s\n", ctx->word); return -1;
  }

  u8 max = MAX_ELEM; // setup on target length
  if(ctx->wlen)
  {
    max = ctx->wlen; DPRINTF("Reading max %d lines\n", max);
  }

  // alloc for data indexes
  size_t size = sizeof(u8*) * max;
  ctx->idx = malloc(size);
  if(!ctx->idx) return -1;
  DPRINTF("malloc @%p %zub for max %u idx\n", ctx->idx, size, max);

  /* Step 1: first check and store */
  u8 i = 0, len, *p = NULL;
  char *line = NULL;
  ssize_t read;

  while((read = getline(&line, &size, fp)) != -1 // read one line at once
  && (i < max)
  )
  {
    if(line[0] == '#') continue; // skip commented (#) lines

    DPRINTF("%2d Retrieved line of length %zu: ", i, read);
    line[read -1] = '\0'; // trim newline unconditionally
    len = strlen(line);
    DPRINTF("[%s] %u-%zu\n", line, len, read);

    if(!len) break; // stop read, on empty line

    switch(ctx->mode) /* check and store, on requested mode */
    {
      case CHAR:
        p = (u8*)strndup(line, len); // store
        break;

      case HEX: {
        if(len %2 || len < 2) // minimal check on lenght
        {
          fprintf(stderr, "[!] Line %u: lenght must be even! (is:%u)\n%s\n", i +1, len, line);
          break;
        }

        s8 r = scan((u8*)line, &len, IS_HEX, NULL); // check for hex digits
        if(r != -1)
        {
          fprintf(stderr, "[!] Line %u: must contain hexadecimal digits only for HEX mode!\n", i +1);
          scan((u8*)line, &len, MARK_ONE, (u8*)&line[(u8)r]); puts("");
          break;
        }

        len /= 2;
        p = _x_to_u8_buffer(line); // store
        break; }
    }
    if(!p) return(-1);

    // alloc for single index
    size = sizeof(u8) * (len +1);
    ctx->idx[i] = malloc(size);
    if(!ctx->idx[i]) return -1;

    memcpy(&ctx->idx[i][1], p, len); // store data
    free(p), p = NULL;
    ctx->idx[i][0] = len;            // store lenght

    DPRINTF("idx %2d/%.2d @%p %zub: %d items\n", i, max, ctx->idx[i], size, len);
    #ifdef DEBUG
      scan(&ctx->idx[i][1], &ctx->idx[i][0], HEXDUMP, NULL); // report
    #endif

    i++;
  }
  free(line), line = NULL;
  fclose(fp), fp = NULL;
  free(p), p = NULL;

  /* Step 2 */
  ctx->wlen = i; // 1. setup tergets lenght

  if(1) // 2. realloc buffers
  {
    size = sizeof(u8) * (i +1);
    p = malloc(size);
    if(!p) return -1;

    memcpy(p, ctx->word, size);
    free(ctx->word), ctx->word = p; // swap buffers, for word
    DPRINTF("Realloc word\t@%p %zub, for %u items\n", ctx->word, size, i);

    if(i != max)
    {
      size = sizeof(u8*) * i;
      u8 **t = malloc(size);
      if(!t) return -1;

      memcpy(t, ctx->idx, size);
      free(ctx->idx), ctx->idx = t; // swap buffers, for data indexes
      DPRINTF("Realloc %d idx\t@%p %zub\n", i, ctx->idx, size);
    }
  }

  if(1) /* Step 3 */
  {
    u32 estimated = 1;
    for(i = 0; i < ctx->wlen; i++)
    {
      p = ctx->idx[i]; // address each charset

      *(ctx->word + i) = p[1]; // 1. fill the initial word
      DPRINTF("idx %2d/%.2d @%p:\t%d items\n", i, ctx->wlen, p, p[0]);
      u8 *d = NULL;
      s8 count = 0;
      for(u8 j = 1; j < p[0] +1; j++) // d scan each charset, from 1
      {
        d = ctx->idx[i] + j;
        count = scan(&p[1], &p[0], COUNT, d) +1;
        //DPRINTF(" %.2d/%.2d\t%#.2x\tcount=%d\n", j, p[0], *d, count);

        if(count > 1) // 2. check if stored charset is unique
        {
          fprintf(stderr, "[!] Line %u: must be unique, found %u repetitions!\n", i +1, count);
          scan(&p[1], &p[0], MARK_ALL, d); puts("");
          return -1;
        }
      }
      estimated *= p[0]; // 3. count combinations
      d = NULL;
    }

    if(ctx->out_m == DRY_RUN) printf("[I] Estimated %u combinations\n", estimated);

    if(estimated < ctx->numw) { fprintf(stderr, "[E] Requested %u combinations can't be reached!\n", ctx->numw); return -1; }
  }

  /* Step 4. check if there is interrupted work to resume... */
  if(resume(ctx) && ctx->out_m == DRY_RUN) printf("[I] Filesave detected! Resuming last generated from: '%s'\n", FILESAVE);

  return 0;
}

