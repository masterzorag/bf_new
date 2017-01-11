#ifndef PARSER_H__
#define PARSER_H__

/*
  bf_new definitions
*/

#define MAX_ELEM        (256 /8)
#define MIN_STRUCT_ALIGNMENT  8

#ifdef DEBUG
  #define DPRINTF printf
#else
  #define DPRINTF(...)
#endif

#define MARKER_ON   printf("%c[%d;%d;%dm", 0x1B, 2, 37, 40); // Set MARK on
#define MARKER_OFF  printf("%c[%d;%d;%dm", 0x1B, 0, 0, 0);   // Revert back

typedef unsigned char u8;
typedef unsigned int u32;
typedef signed char s8;
typedef unsigned long long int u64;

typedef struct
__attribute__((packed, aligned(MIN_STRUCT_ALIGNMENT)))
{
  u8 *word;   // working word
  u8 **idx;   // many charset
  u8  wlen;   // word length = num of charsets
  u8  mode;   // requested mode
  u8  done;   // lock/sync for signal
//u8  pad[5]; // useless, padding
} ctx;

enum mode
{
  CHAR,
  HEX,
  PRINT,
  IS_HEX,
  HEXDUMP,
  FIND,
  COUNT,
  MARK_ONE,
  MARK_ALL
};

void cleanup(ctx *p);
s8 parse_opt (int argc, char **argv, ctx *ctx);
s8 scan(const u8 *item, const u8 *l, const u8 smode, const u8 *dst);
s8 parse_file(ctx *ctx);

#endif // PARSER_H__
