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

## Thinkpad Tools

These tools are specific for Lenovo Thinkpads and may even not work for all of this brand).

### tp_list_mchsh

Lists the hashs of CPU microcodes found in the input file. Tested with T420 and T430 BIOS.

Call:
```
$ ./tp_list_mchsh ../../t430/BIOS-2.69/G1ETA9WW.bin
0. CPU microcode hash at offset 002b06db, cpuid 206a7
1. CPU microcode hash at offset 002b077e, cpuid 306a4
2. CPU microcode hash at offset 002b0821, cpuid 306a5
3. CPU microcode hash at offset 002b08c4, cpuid 306a6
4. CPU microcode hash at offset 002b0967, cpuid 306a8
5. CPU microcode hash at offset 002b0a0a, cpuid 306a9
6 CPU microcode hashs found at file offset 0x002b06db length 0x3d2
```