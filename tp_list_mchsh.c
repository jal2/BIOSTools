/*
   This program dumps the hashes of the CPU microcodes found in a Thinkpad BIOS file.

   It searches for the string
   	TCPACPUH <13 chars> IBMSECUR
   and assumes that the microcode hashes start right after, with a counting byte at the
   start and the CPUID in little endian at offset 0x1b. Each hash is 163 bytes long.
   The end of the hash region is marked by 00 00 27, followed by 36 zero bytes.
   This is true for at least T420 and T430.

   copyright (c) 2015 Joerg Albert, jal2@gmx.de

   GPLv2

*/

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define TCPA_STR "TCPACPUH"
#define TCPA_STR_LEN 8
#define MIDDLE_SIZE 13
#define IBM_STR "IBMSECUR"
#define IBM_STR_LEN 8
#define HASH_SIZE 163
#define CPUID_OFF 0x1b

static int fd = -1;
static unsigned char *faddr = NULL;
static off_t flen = 0;

void usage(const char *name)
{
  fprintf(stderr, "This program tries to locate hashs for CPU microcode in Thinkpad BIOS (e.g. T420, T430).\n");
  fprintf(stderr, "usage: %s bios_file\n", name);
  fprintf(stderr, "  bios_file - BIOS file, either a dump of the SPI flash or a .FL1 file\n");
}

/* read a little endian uint32 from a given address */
static uint32_t get_u32(const unsigned char *addr)
{
  return *addr | (*(addr+1)<<8) | (*(addr+2)<<16) | (*(addr+3)<<24);
}

static unsigned char *do_mmap(const char *name, off_t *filelen)
{
  struct stat st;
  void *rc;

  /* check if the file is regular or symlink */
  if (stat(name, &st)) {
    fprintf(stderr, "#ERR failed to stat %s (%m)\n", name);
    return NULL;
  }
  if (!S_ISREG(st.st_mode) && !S_ISLNK(st.st_mode)) {
    fprintf(stderr, "#ERR %s is not a regular file or a symlink\n", name);
    return NULL;
  }

  fd = open(name, O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "#ERR failed to open %s (%m)\n", name);
    return NULL;
  }

  rc = mmap(NULL, (size_t)st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (rc == MAP_FAILED) {
    fprintf(stderr, "#ERR failed to mmap %s (%m)\n", name);
    return NULL;
  }
  
  *filelen = st.st_size;
  return rc;
}

int main(int argc, const char **argv)
{
  const char *fname;
  int rc = 0;
  const unsigned char *p, *end;

  if (argc < 2) {
    usage(argv[0]);
    return 1;
  }

  fname = argv[1];

  /* mmap the bios file and get its length */
  faddr=do_mmap(fname, &flen);
  if (NULL == faddr) {
    rc = 3;
    goto end;
  }

  p = faddr;
  end = faddr + flen - (TCPA_STR_LEN + MIDDLE_SIZE + IBM_STR_LEN + HASH_SIZE);

  while (p < end) {
    if (!memcmp(p, TCPA_STR, TCPA_STR_LEN) &&
	!memcmp(p+TCPA_STR_LEN+MIDDLE_SIZE, IBM_STR, IBM_STR_LEN))
      break;
    p++;
  }

  if (p < end) {
    p += TCPA_STR_LEN + MIDDLE_SIZE + IBM_STR_LEN;
    const unsigned char *start = p;
    /* found the leading strings */
    //    fprintf(stderr, "#DBG found %s.*%s at offset %08lx\n", TCPA_STR, IBM_STR, start - faddr);
    unsigned int nr = 0;
    while (nr == *p) {
      fprintf(stderr, "%u. CPU microcode hash at offset %08lx, cpuid %x\n", nr, p - faddr, get_u32(p+CPUID_OFF));
      p += HASH_SIZE;
      nr++;
    }
    /* print summary */
    if (nr > 0)
      fprintf(stderr, "%u CPU microcode hashs found at file offset 0x%08lx length 0x%lx\n", nr, start - faddr, p - start);
    else
      fprintf(stderr, "no CPU microcode hashs found\n");
  } else {
    fprintf(stderr, "#ERR couldn't find marker %s.*%s\n", TCPA_STR, IBM_STR);
  }

 end:

  if (NULL != faddr) {
    munmap(faddr, flen);
  }
  if (fd != -1) {
    close(fd);
  }
  return rc;
}
