#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define TRANS 1
#define NOTRANS 0

#ifndef DEBUG
#define DEBUG 0
#endif

#define ENCODE 0
#define DECODE 1

#ifndef MODE
#define MODE ENCODE
#endif

void vigenere_buffer(char *buffer, char *key, int maxkey, int mode) {
  char c;
  if(mode == 1) {
    for(int i = 0; i < 16; i++) {
      c = (char)((buffer[i] + key[i % maxkey]) % 256);
      buffer[i] = c;
      }
    }
  if(mode ==0){
    for(int i = 0; i < 16; i++) {
      c= (char)((buffer[i] - key[i % maxkey]) % 256);
      buffer[i] = c;
      }
  }
}

int transpose_buffer(char *out, char *in, unsigned int dim) {
/* do a columnar encipher/decipher
* from in to out
* using box of size dim*dim
* since it's a square, enciphering and deciphering is the same
*/

/* your elegant code goes here */
   
char matrix[dim][dim];

  for(int i=0;i<dim;i++){
    for(int j=0;j<dim;j++){
      matrix[i][j] = in[(i*dim)+j];
    }
  }
  for(int i=0;i<dim;i++){
    for(int j=0;j<dim;j++){
      out[(i*dim)+j]= matrix[j][i];
    }
  }
	return 0;

}

int dump_buffer(char *buffer, unsigned int bufsize,
unsigned int bytes, FILE *OUTPUT) {

	/* prints a buffer one character at a time to a file using %c
	 * takes in:
	 *  buffer -- pointer to a buffer
	 *  bufsize -- size of 'buffer'
	 *  bytes -- number of bytes from buffer to print
	 *  output -- path to the file to open and output to
	 */
	
	/* open the output or quit on error */
	/* print 'bytes' bytes from buffer to output file one char at a time */
  int i =0;
  while (i<bufsize){
    fprintf(OUTPUT,"%c",buffer[i]);
    i++;
  }
	/* optional: wipe buffer using memset */
  memset(buffer,0,bufsize);
	/* close output file */

	return bytes;


}



int pad_buffer(char *buffer, unsigned int bufsize, unsigned int rbuf_index) {
	/* pad_buffer pads the empty space in a buffer
	 *  buffer -- pointer to buffer
	 *  bufsize -- size of 'buffer'
	 *  rbuf_index -- first "empty" spot in buffer, i.e.,
	 *                put the 'X' at rbuf_index and fill the
	 *                rest with 'Y' characters.
	 */

	int padded = 0;
	
	/* code goes here */
	
	buffer[rbuf_index] = 'X';
	rbuf_index++;
	padded++;
	while(rbuf_index < bufsize){
	  buffer[rbuf_index] = 'Y';
	  rbuf_index++;
	  padded++;
	}

	return padded;


}

int unpad_buffer(char *buffer, unsigned int bufsize) {
	/* unpads a buffer of a given size
	 *  buffer -- buffer containing padded data
	 *  bufsize -- size of 'buffer'
	 */

	int unpadded = bufsize;

  while(buffer[unpadded-1]=='Y'){
    --unpadded;
  }

  return unpadded -1;

}

int main(int argc, char *argv[]) {

int i = 0; /* iterator we'll reuse */

if (argc < 4) {
  printf("Missing arguments!\n\n");
  printf("Usage: encoder rounds keyfile infile outfile ['notrans']\n\n");
  printf("Note: outfile will be overwritten.\n");
  printf("Optional '1' as last parameter will disable transposition.\n");
  return 1;
}

/* give input and output nicer names */
unsigned int rounds = atoi(argv[1]);//rlunds
char *key = argv[2];  //saving key bytes
unsigned int dim = 4; /* dimension of the box */
char *input = argv[3]; /* input file path */
char *output = argv[4]; /* output file path */

/* use 'transmode' to determine if we are just padding or also
* doing transposition. very helpful for debugging! */

unsigned int transmode = TRANS; /* default is TRANS */
unsigned int rbuf_count = 0;
unsigned int bufsize = dim * dim;
char read_buf[bufsize]; /* buffer for reading and padding */


/* open the input or quit on error. */
FILE *INPUT;
if ((INPUT = fopen(input, "r")) == NULL) {
  printf("Problem opening input file '%s'; errno: %d\n", input, errno);
  return 1;
}


/* get length of input file */
unsigned int filesize; /* length of file in bytes */
unsigned int bytesleft; /* counter we reduce on reading */
struct stat filestats; /* struct for file stats */
int err;

if ((err = stat(input, &filestats)) < 0) {
  printf("error statting file! Error: %d\n", err);
}

filesize = filestats.st_size;
bytesleft = filesize;

if (DEBUG) printf("Size of 'input' is: %u bytes\n", filesize);

/* truncate output file if it exists */
   
FILE *OUTPUT;
if ((OUTPUT = fopen(output, "w+")) == NULL) {
  printf("Problem truncating output file '%s'; errno: %d\n", output, errno);
  return 1;
}

//Key Open
FILE *KEY;
if ((KEY = fopen(key, "r")) == NULL) {
printf("Problem opening key file '%s'; errno: %d\n", key, errno);
return 1;
}

/* loop through the input file, reading into a buffer and
* processing the buffer when 1) the buffer is full or
* 2) the file has ended (or in the case of decoding, when
* the last block is being processed.
*/
int psymbol;
int maxkey = 0;
//Key
while((psymbol = fgetc(KEY)) != EOF) {
  key[maxkey] = (char)psymbol;
  maxkey++;
}
//close key
fclose(KEY);
int rbuf_index = 0; /* index into the read buffer */
int num;

/******************
*  do stuff here *
******************/
//ENCODE
if(MODE == ENCODE) {
  while (((psymbol = fgetc(INPUT)) != EOF) && bytesleft != 0) {
          read_buf[rbuf_index] = (char)psymbol;
          ++rbuf_index;
          --bytesleft;
  if(rbuf_index == bufsize) {
  for(int i = 0; i < rounds; ++i) {
    vigenere_buffer(read_buf, key, maxkey, 1);
    num= transpose_buffer(read_buf, read_buf, dim);
  }

    num= dump_buffer(read_buf, bufsize, rbuf_index, OUTPUT);
    rbuf_index = 0;
  }
  }
  num=pad_buffer(read_buf, bufsize, rbuf_index);
  for(int i = 0; i < rounds; ++i) {
        vigenere_buffer(read_buf, key, maxkey, 1);
        num=transpose_buffer(read_buf, read_buf, dim);
  }
  dump_buffer(read_buf, bufsize, rbuf_index, OUTPUT);
}

//DECODE
if(MODE == DECODE) {
  while((psymbol = fgetc(INPUT)) != EOF) {
    read_buf[rbuf_index] = (char)psymbol;
    ++rbuf_index;
    --bytesleft;
    if(rbuf_index == bufsize) {
      if(bytesleft == 0) {
        for(int i = 0; i < rounds; ++i) {
          num=transpose_buffer(read_buf, read_buf, dim);
          vigenere_buffer(read_buf, key, maxkey, 0);
        }
      num=dump_buffer(read_buf, unpad_buffer(read_buf, bufsize), rbuf_index, OUTPUT);
      } 
      else {
        for(int i = 0; i < rounds; ++i) {
          num=transpose_buffer(read_buf, read_buf, dim);
          vigenere_buffer(read_buf, key, maxkey, 0);
        }
        num=dump_buffer(read_buf, bufsize, rbuf_index, OUTPUT);
        rbuf_index = 0;
      }
    }
  }
}

fclose(INPUT);
fclose(OUTPUT);
return 0;
 
}

