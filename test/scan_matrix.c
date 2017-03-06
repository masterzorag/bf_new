/*
  gcc -o scan_matrix_demo scan_matrix.c -std=gnu99 -Wall

  load a data matrix and check STDIN for (binary) items

  $ ../bf -c test_7.HEX -x -b | ./scan_matrix_demo
  Readed 25600b: 1280 items @0xeea250
  464101df2b5d342e046611c71ec11de8d173faf0 -> 20/20 @167
  Found one at 168 !!!
  Checked 75484 items
  or,
  $ cut -c 1-20 /dev/urandom | ./scan_matrix_demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define FILENAME   "../r.bin" // data matrix
#define DATA_LEN   20
#define MIN_MATCH  4


#include <time.h>
clock_t startm, stopm;
#define START if ((startm = clock()) == -1){ printf("Error calling clock"); exit(1); }
#define STOP  if ((stopm  = clock()) == -1){ printf("Error calling clock"); exit(1); }
#define PRINTTIME printf( "%6.3f seconds", ((double)stopm-startm) /CLOCKS_PER_SEC);


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
  Leftwise min data compare, return:
  0 for whole matches,
  -1 if less than MIN_MATCH,
  else num of matching bytes
*/
static signed char user_memcmp_v2(const unsigned char * const a, const unsigned char * const b)
{
  char count = 0;
  for(unsigned char i = 0; i < DATA_LEN; i++)
  {
    if(a[i] != b[i]) break;
    else count++;
  }

  if(count != DATA_LEN)
  {
    if(count >= MIN_MATCH) return count;

    return -1;
  }
  return 0;
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
  last best match honoring MIN_MATCH, else -1
*/
static signed int matrix_traverse(unsigned char * const M, unsigned char * const p)
{
  signed int last, res;
  last = res = -1;
  for(int i = 0; i < num_matrix_entries; i++) // ensure we scan whole table
  {
    res = user_memcmp_v2(p, M + (i * DATA_LEN));

    if(res != -1) last = i; // report the last best found
  }
  return last;
}


int main(int argc, char **argv)
{
  unsigned char *M = matrix_load(),
                *p = calloc(DATA_LEN, sizeof(unsigned char));
  if(!M || !p) exit(-1);

  signed int best, res;
  unsigned int count = 0;

  ssize_t size = 1;
  START;
  while(size)
  {
    size = read(STDIN_FILENO, p, DATA_LEN); // read data

    if(size == DATA_LEN)
    { //    printf("readed %zu items\n", size);

      // do stuff with p, for example: rx = p x G
      //print_cmp(p, M + (1 * DATA_LEN));

      best = matrix_traverse(M, p);
      if(best > -1)
      {  //if(memcmp(p, M + (i * 20), 20) == 0) {
        res = print_cmp(p, M + (best * DATA_LEN));
        printf(" -> %d/%d @%d\n", res, DATA_LEN, best);

        if(res == DATA_LEN) // exit if found one!
        {
          printf("Found one at %d !!!\n", best +1); // (no row zero)
          break;
        }
      }

      if(count && count %1000000 == 0) // output only every n attempt
      {
        STOP;
        printf("[%.2f/sec]\n", 1000000 /(((double)stopm-startm) /CLOCKS_PER_SEC));
        START;
      }

     // for(int i = 0; i < DATA_LEN; i++) printf("%.2x", *(M + i));
     // puts("");
      count++;
    }
    else break;
  }
  printf("Checked %d items\n", count); // done

  if(0)
  {
    for(int i = 0; i < num_matrix_entries; i++) // scan matrix
    {
      if(i %DATA_LEN == 0) puts("");

      printf("%.2x", M[i]); // dump
    }
  }

  //printf("\n@%p %d\n", M, num_matrix_entries);
  free(M), M = NULL;
  free(p), p = NULL;

  return 0;
}