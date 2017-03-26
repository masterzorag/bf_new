/*
  gcc -o matrix_demo matrix.c -std=gnu99 -Wall

  load a data matrix and check STDIN for (binary) items

  $ ../bf -c test_7.HEX -x -b | ./matrix_demo
  Readed 25600b: 1280 items @0xeea250
  464101df2b5d342e046611c71ec11de8d173faf0 -> 20/20 @167
  Found one at 167 !!!
  Checked 112348 items

  or,
  $ cut -c 1-20 /dev/urandom | ./matrix_demo
  [30130.22/sec]
  [30074.36/sec]
  0324c53296690a1e3d81c893552fdcaea446ff90 -> 4/20 @829
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define FILENAME   "../r.bin" // data matrix
#define DATA_LEN   20
#define MIN_MATCH   4


#include <time.h>
static clock_t startm, stopm;
#define START if ((startm = clock()) == -1){ printf("Error calling clock"); exit(1); }
#define STOP  if ((stopm  = clock()) == -1){ printf("Error calling clock"); exit(1); }
#define PTIME printf( "%6.3f seconds", ((double)stopm-startm) /CLOCKS_PER_SEC);


static int num_matrix_entries = 0;

/*
  Read data matrix from file, return:
  a pointer to allocated data raeded,
  setting num_matrix_entries
*/
static void *matrix_load()
{
  FILE *fp = fopen(FILENAME, "r");
  if(!fp) return NULL;

  fseek(fp, 0L, SEEK_END); // store file size
  long r = ftell(fp);
  rewind(fp);

  unsigned char *d = calloc(r, sizeof(unsigned char));
  if(!d) return NULL;

  size_t n = fread(d, sizeof(unsigned char), r, fp); // read
  if(n != r) return NULL;

  fclose(fp), fp = NULL;
  num_matrix_entries = r / DATA_LEN;

  printf("Readed %lub: %d items @%p\n", r, num_matrix_entries, d);

  return d;
}

/*
  Leftmost min data compare, return:
  0 for whole matches,
  -1 if less than MIN_MATCH,
  else num of matching bytes
*/
static signed char user_memcmp(const unsigned char * const a, const unsigned char * const b)
{
  char count = 0;
  for(unsigned char i = 0; i < DATA_LEN; i++)
  {
    if(a[i] != b[i]) break;
    else count++;
  }

  if(count == DATA_LEN) return 0; // whole match

  if(count >= MIN_MATCH)
    return count;
  else
    return -1;
}

static char print_cmp(const unsigned char * const a, const unsigned char * const b)
{
  char count = 0;
  for(unsigned char i = 0; i < DATA_LEN; i++)
  {
    if(a[i] != b[i])
      printf("%02x", a[i]);
    else {
      printf("\x1B[7;32m%02x\x1B[0m", a[i]);
      count++;
    }
  } //printf("\n");
  return count;
}

/*
  Scan matrix targets for p, returns:
  best match honoring MIN_MATCH,
  else -1
*/
static signed int matrix_scan(unsigned char * const M, unsigned char * const p)
{
  signed int best, last, res;
  best = last = res = -1;
  for(int i = 0; i < num_matrix_entries; i++) // ensure we scan whole table
  {
    res = user_memcmp(p, M + (i * DATA_LEN));

    if(res > best) last = i, best = res; // report the best found
  }
  return last;
}

/*
  Dump single record from matrix
*/
static void matrix_read(unsigned char * const M, const int num)
{
  if(num > num_matrix_entries) return;

  unsigned char * const p = M + (num * DATA_LEN);

  for(int i = 0; i < DATA_LEN; i++) printf("%.2x", *(p + i));

  puts("");
}

/*
  Dump entire matrix
*/
static void matrix_dump(unsigned char * const M)
{
  printf("data matrix @%p storing %d records\n", M, num_matrix_entries);

  for(int i = 0; i < num_matrix_entries; i++) matrix_read(M, i);
}


int main(int argc, char **argv)
{
  unsigned char *M = matrix_load(),
                *p = calloc(DATA_LEN, sizeof(unsigned char));
  if(!M || !p) exit(-1);

//matrix_dump(M); exit(0);

  signed int best, res;
  unsigned int count = 0;

  ssize_t size = 1;
  START;
  while(size)
  {
    size = read(STDIN_FILENO, p, DATA_LEN); // read data

    if(size == DATA_LEN)
    { // printf("readed %zu items\n", size);
      // do stuff with p, for example: R = p x G
      best = matrix_scan(M, p);

      if(best > -1)
      {  //if(memcmp(p, M + (i * 20), 20) == 0) {
        res = print_cmp(p, M + (best * DATA_LEN));
        printf(" -> %d/%d @%d\n", res, DATA_LEN, best);

        if(res == DATA_LEN) // exit if found one!
        {
          printf("Found one at %d !!!\n", best);
          break;
        }
      }

      if(count && count %1000000 == 0) // output only every n attempt
      {
        STOP;
        printf("[%.2f/sec]\n", 1000000 /(((double)stopm-startm) /CLOCKS_PER_SEC));
        START;
      }

      count++;
    }
    else break;
  }
  printf("Checked %d items\n", count); // done

  free(M), M = NULL;
  free(p), p = NULL;

  return 0;
}