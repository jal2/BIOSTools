# BIOSTools
A collection of tools to analyze BIOS images

## list_micro

Lists the CPU microcodes found in the input file.

Call:
```
./list_micro t420_bios.bin
microcode at 0x00740c48:
  update revision:       38
  date:                  02/17/2010
  processor signature:   0x206a2
  loader revision:       1
  processor flags:       0x12
  data size:             0x23d0 (9168)
  total size:            0x2400 (9216)
microcode at 0x00743048:
  update revision:       8
  date:                  05/26/2010
  processor signature:   0x206a3
  loader revision:       1
  processor flags:       0x12
  data size:             0x23d0 (9168)
  total size:            0x2400 (9216)
...
```