/*
  gcc -o signal_demo signal.c -std=gnu99 -Wall

  SIGINT, SIGUSR1, dump data by hitting CTRL-C
*/

#include<stdio.h>
#include<signal.h>
#include<unistd.h>

char *p = NULL; // at global scope


void sig_handler(int signo) // use p to access data
{
  if(signo == SIGUSR1)
    printf("received SIGUSR1\n");

  if (signo == SIGINT) // get Ctrl-c
  {
    printf("\nreceived SIGINT\n");
    FILE *fp = fopen(".bf.save", "w");
    if(!fp) exit(-1);

    size_t n = fwrite(p, sizeof(char), 5, fp); // dump
    fclose(fp); fp = NULL;

    printf("written %zu, %#.2x\n", n, *p);
    p = NULL;
    exit(1);
  }
}


int main(void)
{
  printf("send 'kill -USR1 %d' from another term\n", getpid());

  char a[10];
  a[0] = 0x61; a[1] = 0x66; a[2] = 0x90; p = a; // address p to a

  if(signal(SIGUSR1, sig_handler) == SIG_ERR) printf("\ncan't catch SIGUSR1\n");

  if(signal(SIGINT,  sig_handler) == SIG_ERR) printf("\ncan't catch SIGINT\n");

  while(1)
    sleep(1);
  return 0;
}