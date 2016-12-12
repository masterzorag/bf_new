/*
  gcc -o read_stdin_demo read_stdin.c -std=gnu99 -Wall

  binary read of NUM_ELEM from stdin, report hex data:

  $ cat test_2 | ./read_stdin_demo
  readed 20 items
  61626364650a61626364656667680a7374717576
  readed 20 items
  7a78610a7175776a796162630a7374756162630a

  or, $ ./read_stdin_demo
  and input data from terminal
*/

#include <stdio.h>
#include <unistd.h>

#define NUM_ELEM  20

int main(int argc, char **argv)
{
  char a[NUM_ELEM];
  ssize_t size = 1;

  while(size)
  {
    size = read(STDIN_FILENO, &a, NUM_ELEM);
    if(size == 20)
    {
      printf("readed %zu items\n", size);

      for(int i = 0; i < NUM_ELEM; i++) printf("%.2x", a[i]);

      puts("");
    }
    else break;
  }

  return 0;
}