#ifndef PARSER_H__
#define PARSER_H__

/*
  bf_new definitions
*/

#define MAX_ELEM (256 /8)

#ifdef DEBUG
  #define DPRINTF printf
#else
  #define DPRINTF(...)
#endif

typedef unsigned char u8;
typedef unsigned int u32;
typedef char s8;
typedef unsigned long long int u64;

typedef struct {
  u8 *data;
  u8 len;
} set;

typedef struct {
  set word;   // working word
  set *cset;  // many charset
  u8 mode;    // requested mode
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

u64 _x_to_u64(const s8 *hex);
u8 *_x_to_u8_buffer(const s8 *hex);

s8 parse_opt (int argc, char **argv, ctx *ctx);
s8 scan(const u8 *item, const u8 *l, u8 mode, u8 *dst);
s8 parse_file(ctx *ctx);

#endif // PARSER_H__
