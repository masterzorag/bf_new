#ifndef PARSER_H__
#define PARSER_H__

/*
  bf_new definitions
*/

#define VERSION      "0.2.6-dev"
#define FILESAVE      ".bf.save"

#define MAX_ELEM        (256 /8)
#define MIN_STRUCT_ALIGNMENT  8

#ifdef DEBUG
  #define DPRINTF printf
#else
  #define DPRINTF(...)
#endif

#define MARKER_ON   printf("%c[%d;%d;%dm", 0x1B, 2, 37, 40); // Set MARK on
#define MARKER_OFF  printf("%c[%d;%d;%dm", 0x1B, 0, 0, 0);   // Revert back

typedef signed   char s8;
typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long int u64;

typedef struct
__attribute__((packed, aligned(MIN_STRUCT_ALIGNMENT)))
{
  u8  *word;  // working word
  u8  **idx;  // many charset
  u8   wlen;  // word length = num of charsets
  u8   mode;  // requested mode (CHAR | HEX)
  s8   work;  // lock/sync for signals
  u8  out_m;  // output mode flag
  u32  numw;  // max num of words
} ctx;

enum flags
{
  // for main mode
  CHAR,
  HEX,
  // for scan() mode
  PRINT,
  IS_HEX,
  FIND,
  COUNT,
  HEXDUMP,
  MARK_ONE,
  MARK_ALL,
  // for output mode
  BIN,
  WORDLIST,
  DRY_RUN, // default
  QUIET,
  // for signals
  DONE,
  DUMP
};

void bin2stdout(ctx *p);
void dump_matrix(ctx *p);
void cleanup(ctx *p);
s8 parse_opt (int argc, char **argv, ctx *ctx);
s8 scan(const u8 *item, const u8 *l, const u8 smode, const u8 *dst);
s8 parse_file(ctx *ctx);

#endif // PARSER_H__
