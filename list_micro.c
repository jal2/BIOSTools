/*
   This program dumps the CPU microcodes found in a BIOS file.

   copyright (c) 2015 Joerg Albert, jal2@gmx.de

   GPLv2

*/

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

static int fd = -1;
static unsigned char *faddr = NULL;
static off_t flen = 0;

void usage(const char *name)
{
  fprintf(stderr, "usage: %s bios_file\n", name);
  fprintf(stderr, "  bios_file - BIOS file, either a dump of the SPI flash or a .FL1 file\n");
}

#define INTEL_MC_HEADER_SZ 48
/* this is a copy of the microcode header at faddr, all fields are in the cpu endianess */
struct mc_header_s {
  uint32_t header_ver;
  uint32_t update_rev;
  uint32_t date;
  uint32_t proc_sig;
  uint32_t checksum;
  uint32_t loader_rev;
  uint32_t proc_flags;
  uint32_t data_size;
  uint32_t total_size;
};

/* read a little endian uint32 from a given address */
static uint32_t get_u32(const unsigned char *addr)
{
  return *addr | (*(addr+1)<<8) | (*(addr+2)<<16) | (*(addr+3)<<24);
}

/* calc. the checksum by adding up DWORD (little endian).
   len is in byte */
static uint32_t calc_checksum(const unsigned char *p, uint32_t len)
{
  uint32_t rc = 0;
  assert((len % 4) == 0);
  
  while (len > 0) {
    rc += get_u32(p);
    p += 4;
    len -= 4;
  }

  return rc;
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

static void get_header(const unsigned char *addr, struct mc_header_s *str)
{
  str->header_ver = get_u32(addr);
  str->update_rev = get_u32(addr+4);
  str->date       = get_u32(addr+8);
  str->proc_sig   = get_u32(addr+12);
  str->checksum   = get_u32(addr+16);
  str->loader_rev = get_u32(addr+20);
  str->proc_flags = get_u32(addr+24);
  str->data_size  = get_u32(addr+28);
  str->total_size = get_u32(addr+32);
}

static void print_header(const unsigned char *addr, const struct mc_header_s *str)
{
  printf("microcode at 0x%08lx:\n", addr-faddr);
  //  printf("  header version:        %u\n", str->header_ver);
  printf("  update revision:       %u\n", str->update_rev);
  printf("  date:                  %02x/%02x/%04x\n", str->date>>24, (str->date>>16) & 0xff, (str->date) & 0xffff);
  printf("  processor signature:   0x%x\n", str->proc_sig);
  //  printf("  checksum:              0x%08x\n", str->checksum);
  printf("  loader revision:       %u\n", str->loader_rev);
  printf("  processor flags:       0x%x\n", str->proc_flags);
  printf("  data size:             0x%x (%u)\n", str->data_size, str->data_size);
  printf("  total size:            0x%x (%u)\n", str->total_size, str->total_size);
}

int main(int argc, const char **argv)
{
  const char *fname;
  int rc = 0;
  const unsigned char *p, *end;
  struct mc_header_s header;
  const unsigned char *first_mc; /* address of the first mc */
  const unsigned char *end_mc; /* first byte after the last mc */
  uint32_t size_sum = 0; /* sum of the total sizes of all mc so far */
  uint32_t nr_mc = 0;
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

  /* algorithm to find the microcodes without a start address:
     - search through the file (on Thinkpad T420 and Edge S430 the mc were located at the end) for byte sequence 01 00 00 00
     - assume this to be the header version, check checksum and data_size vs. total_size
     - print the mc
  */
  
  p = faddr;
  end = faddr + flen - INTEL_MC_HEADER_SZ;

  while (p < end) {
    if (get_u32(p) == 1) {
      get_header(p, &header);
      if ((header.total_size % 4) == 0 && (p + header.total_size) <= (faddr+flen)) {
	if (calc_checksum(p, header.total_size) == 0) {
	  if (header.data_size < header.total_size) {
	    print_header(p, &header);
	    if (nr_mc == 0) {
	      first_mc = p;
	    }
	    nr_mc++;
	    size_sum += header.total_size;
	    end_mc = p + header.total_size;
	  }
	}
      }
    }
    p++;
  }

 end:

  if (nr_mc > 0) {
    /* print some summary */
    fprintf(stderr, "\n%u CPU microcodes found", nr_mc);
    if ((end_mc - first_mc) == size_sum) {
      /* in a contiguous area */
      fprintf(stderr, " at file offset 0x%08lx, overall length 0x%x\n", first_mc-faddr, size_sum);
    } else
      fprintf(stderr, " at various locations\n");
  } else
    fprintf(stderr, "#ERR no CPU microcode found\n");

  if (NULL != faddr) {
    munmap(faddr, flen);
  }
  if (fd != -1) {
    close(fd);
  }
  return rc;
}
