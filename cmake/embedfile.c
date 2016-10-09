#include <stdlib.h>
#include <stdio.h>

FILE* open_or_exit(const char* fname, const char* mode)
{
  FILE* f = fopen(fname, mode);
  if (f == NULL) {
    perror(fname);
    exit(EXIT_FAILURE);
  }
  return f;
}

int main(int argc, char** argv)
{
  if (argc < 4) {
    fprintf(stderr, "USAGE: %s symbolName resourceFile outputFile\n\n", argv[0]);
    return EXIT_FAILURE;
  }

  const char* sym = argv[1];
  FILE* in = open_or_exit(argv[2], "rb");
  FILE* out = open_or_exit(argv[3], "wt");

  fprintf(out, "extern const char %s[];\n", sym);
  fprintf(out, "const char %s[] = {\n", sym);

  char buf[256];
  size_t nread = 0;
  size_t linecount = 0;
  do {
    nread = fread(buf, 1, sizeof(buf), in);
    size_t i;
    for (i=0; i < nread; i++) {
      fprintf(out, "0x%02x, ", buf[i]);
      if (++linecount == 10) { fprintf(out, "\n"); linecount = 0; }
    }
  } while (nread > 0);
  if (linecount > 0) fprintf(out, "\n");
  fprintf(out, " 0};\n"); // Make it null terminated (assuming text content!)

  fclose(in);
  fclose(out);

  return EXIT_SUCCESS;
}
