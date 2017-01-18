/*
  gcc -o read_stdin_demo read_stdin.c -std=gnu99 -Wall

  binary read of <len> bytes from stdin, report hex data:

  $ cat test_2 | ./read_stdin_demo -l 20
  readed 20 items
  61626364650a61626364656667680a7374717576
  readed 20 items
  7a78610a7175776a796162630a7374756162630a

  or, $ ./read_stdin_demo -l <len>
  and input data from terminal
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>


int main(int argc, char **argv)
{
  int len = 0, c;
  opterr = 0;
  while((c = getopt(argc, argv, "l:")) != -1)
  {
    switch(c)
    {
      case 'l': len = atoi(optarg); break;
      case '?':
        if(optopt == 'l')
          fprintf(stderr, "Option -%c requires an argument.\n", optopt);
        else if(isprint(optopt))
          fprintf(stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
        return -1;

      default:
        abort();
    }
  }
  if(!len)
  {
    printf("read chunk of data size with -l\n");
    return -1;
  }

  unsigned char *a = malloc(sizeof(unsigned char) * len);
  if(!a) return -1;

  ssize_t size = 1;
  while(size)
  {
    size = read(STDIN_FILENO, a, len);
    if(size == len)
    {
      printf("readed %zu items\n", size);

      for(int i = 0; i < len; i++) printf("%.2x", *(a + i));

      puts("");
    }
    else break;
  }

  free(a);
  a = NULL;

  return 0;
}
