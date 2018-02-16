#include <stdio.h>
#include <blosc.h>
#include <string.h>

#define SIZE 1000 * 1000

int main(int argc, char *argv[]){
  static int32_t data[SIZE];
  static int32_t data_out[SIZE];
  static int32_t data_dest[SIZE];
  int isize = SIZE * sizeof(int32_t), osize = SIZE*sizeof(int32_t);
  int dsize = SIZE * sizeof(int32_t), csize;
  int i;

  FILE *f;

  for(i=0; i<SIZE; i++){
    data[i] = i;
  }

  /* Register the filter with the library */
  printf("Blosc version info: %s (%s)\n", BLOSC_VERSION_STRING, BLOSC_VERSION_DATE);

  /* Initialize the Blosc compressor */
  blosc_init();

  /* Use the argv[2] compressor. The supported ones are "blosclz",
  "lz4", "lz4hc", "snappy", "zlib" and "zstd"*/
  blosc_set_compressor(argv[2]);

  if (strcmp(argv[1], "compress") == 0) {
    /* Compress with clevel=9 and shuffle active  */
    csize = blosc_compress(9, 1, sizeof(int32_t), isize, data, data_out, osize);
    if (csize == 0) {
      printf("Buffer is uncompressible.  Giving up.\n");
      return 1;
    }
    else if (csize < 0) {
      printf("Compression error.  Error code: %d\n", csize);
      return csize;
    }

    printf("Compression: %d -> %d (%.1fx)\n", isize, csize, (1.*isize) / csize);

    /* Write data_out to argv[3] */
    f = fopen(argv[3], "wb+");
    if (fwrite(data_out, 1, csize, f) == SIZE) {
      printf("Wrote %s\n", argv[3]);
    } else {
      printf("Write failed");
    }
  } else {
    /* Read from argv[3] into data_out. */
    f = fopen(argv[3], "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET); 
    if (fread(data_out, 1, fsize, f) == fsize) {
      printf("Read %s\n", argv[3]);
    } else {
      printf("Read failed");
    }

    /* Decompress */
    dsize = blosc_decompress(data_out, data_dest, dsize);
    if (dsize < 0) {
      printf("Decompression error.  Error code: %d\n", dsize);
      return dsize;
    }

    printf("Decompression succesful!\n");
  }

  /* After using it, destroy the Blosc environment */
  blosc_destroy();

  return 0;
}

