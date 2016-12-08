/*
  gcc -o gmp_demo gmp.c -std=gnu99 -lgmp -Wall

  a binary data gmp import/export sample

  https://gmplib.org/manual/Integer-Import-and-Export.html
*/

#include <stdio.h>
#include <gmp.h>

#define NUM_ELEM  20

int main(int argc, char **argv)
{
  mpz_t z;
  mpz_init(z);
  unsigned char a[NUM_ELEM]; // input data

  /* a init */
  for(int i = 0; i < NUM_ELEM; i++) a[i] = 0x08;


//void mpz_import (mpz_t rop, size_t count, int order, size_t size, int endian, size_t nails, const void *op)
//Set rop from an array of word data at op. 

  /* Initialize z from a */
  mpz_import (z, NUM_ELEM, 1, sizeof(a[0]), 0, 0, a); // import a into z


  mpz_out_str(stdout, 16, z); printf(" ");
  mpz_out_str(stdout, 10, z); printf("\n");

  mpz_add_ui(z, z, 1);

  mpz_out_str(stdout, 16, z); printf(" ");
  mpz_out_str(stdout, 10, z); printf("\n");


//void * mpz_export (void *rop, size_t *countp, int order, size_t size, int endian, size_t nails, const mpz_t op)
//Fill rop with word data from op

  size_t count;
  mpz_export (a, &count, 1, sizeof(unsigned char), 0, 0, z); // export z into a
  printf("count=%zu\n", count);


  for(int i = 0; i < NUM_ELEM; i++) printf("%.2x", a[i]);
  printf("\n");


  // set from "string"
  mpz_set_str(z, "0x0badbabe", 0);

  mpz_out_str(stdout, 16, z); printf(" ");
  mpz_out_str(stdout, 10, z); printf("\n");

  mpz_clear(z); // release

  return 0;
}