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

typedef unsigned char u8;
typedef unsigned int u32;
typedef char s8;
typedef unsigned long long int u64;

typedef struct
__attribute__((packed, aligned(MIN_STRUCT_ALIGNMENT)))
{
  u8 *word;   // working word
  u8 **idx;   // many charset
  u8  wlen;
  u8  mode;   // requested mode
} ctx;

enum mode
{
  CHAR,
  HEX,
  IS_HEX,
  DUMP,
  FIND,
  COUNT,
  MARK_CHAR,
  MARK_HEX,
  MARK_ALL
};

s8 parse_opt (int argc, char **argv, ctx *ctx);
s8 scan(const u8 *item, const u8 *l, const u8 mode, const u8 *dst);
s8 parse_file(ctx *ctx);

#endif // PARSER_H__
