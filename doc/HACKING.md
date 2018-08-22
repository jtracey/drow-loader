# Hacking
This document is intended to be a helpful start to developing or debugging drow-loader code.

## Organization
Here, we describe the files and directories that make up the drow-loader repo.

Control flow of the dynamic linker approximately goes:

x86_64/stage0.s -> stage1.c -> stage2.c => core functionality

then back up the call graph to stage0.s where it starts the target executable.


Control flow of the dynamic loader approximately goes:

application code -> libvdl.c -> vdl-dl-public.c -> vdl-dl.c => core functionality

The files that I would consider "core functionality" (i.e., code you wouldn't normally see in other programs) are, roughly in order of importance:
vdl-reloc.c, vdl-lookup.c, vdl-map.c, vdl-tls.c, vdl-linkmap.c, vdl-context.c

| File name(s)                                                         | Description                  |
| -------------------------------------------------------------------- | ---------------------------- |
| [README.md](README.md)                                               | Readme file for drow-loader. |
| [README](README)                                                     | Original readme file for elf-loader (the project drow-loader forked from). |
| [vdl.h](vdl.h) <br /> [vdl.c](vdl.c)                                 | Global header file. Contains includes, global variables, etc., instantiated by `vdl.c`. Not to be confused with `dl.h` or `vdl-dl.h`. |
| [vdl-dl.c](vdl-dl.c) <br /> [vdl-dl.h](vdl-dl.h)                     | Where most of the high-level dynamic loading functionality is implemented. If it has something to do with dynamic loading, your adventure probably starts here. |
| [stage1.c](stage1.c) <br /> [stage1.h](stage1.h)                     | Initial setup of global variables, minimal Linux/glibc ABI, etc. of the dynamic linker (and teardown). |
| [stage2.c](stage2.c) <br /> [stage2.h](stage2.h)                     | Main setup of dynamic linker (and therefore dynamic loader). |
| [vdl-reloc.c](vdl-reloc.c) <br /> [vdl-reloc.h](vdl-reloc.h)         | Code for relocating symbols. I.e., setting up the symbol to be used. |
| [vdl-lookup.c](vdl-lookup.c) <br /> [vdl-lookup.h](vdl-lookup.h)     | Code for looking up symbols. I.e., locating the requested symbol (if it can be found). |
| [vdl-map.c](vdl-map.c) <br /> [vdl-map.h](vdl-map.h)                 | Code for mapping files (executables and libraries) into memory, as well as storing various ELF fields. |
| [vdl-unmap.c](vdl-unmap.c) <br /> [vdl-unmap.h](vdl-unmap.h)         | Code for cleaning up file maps and structures. |
| [vdl-file.h](vdl-file.h)                                             | Structures for representing information about files (executables and libraries). |
| [vdl-linkmap.c](vdl-linkmap.c) <br /> [vdl-linkmap.h](vdl-linkmap.h) | Code for interfacing with the linkmap (part of the glibc ABI that enumerates all loaded binaries). |
| [vdl-context.c](vdl-context.c) <br /> [vdl-context.h](vdl-context.h) | Code for managing contexts (the internally used name for namespaces). |
| [vdl-tls.c](vdl-tls.c) <br /> [vdl-tls.h](vdl-tls.h)                 | Code for interfacing with and managing Thread-Local Storage (TLS). |
| [vdl-init.c](vdl-init.c) <br /> [vdl-init.h](vdl-init.h)             | Code for calling the initialization functions specified in linked/loaded ELF files. |
| [vdl-fini.c](vdl-fini.c) <br /> [vdl-fini.h](vdl-fini.h)             | Code for calling the finished functions specified in linked/loaded ELF files. |
| [vdl-sort.c](vdl-sort.c) <br /> [vdl-sort.h](vdl-sort.h)             | Code for sorting files into the required order for correct dynamic linker/loader operations. |
| [glibc.c](glibc.c) <br /> [glibc.h](glibc.h)                         | Some of the neccessary, more hacky code for maintaining interal ABI-compatibility with glibc. Other ABI details are sprinkled in other files as needed. |
| [gdb.c](gdb.c) <br /> [gdb.h](gdb.h)                                 | Some of the neccessary, more hacky code for maintaining interal ABI-compatibility with gdb. Other ABI details are sprinkled in other files as needed. |
| [valgrind.c](valgrind.c) <br /> [valgrind.h](valgrind.h) <br /> [get-valgrind-cflags.py](get-valgrind-cflags.py) | Some of the neccessary, more hacky code for getting valgrind working properly. |
| [x86_64/](x86_64)                                                    | Files specific to the x86-64 CPU architecture. Ostensibly everything architecture-specific should be in here, but because we only support one architecture, it's likely there are assumptions made elsewhere as well. Note that we no longer support i386. |
| [x86_64/stage0.S](x86_64/stage0.S)                                   | Assembly for the true entry point of the dynamic linker (and, by extension, the target program). |
| [x86_64/resolv.S](x86_64/resolv.S)                                   | Assembly stub used by unresolved symbols. I.e., every time a symbol is used but not yet relocated, this is what's run. |
| [x86_64/machine.c](x86_64/machine.c)                                 | Any C code specifically for x86-64 --- e.g., handling the different symbol types, and implementing the Linux syscall ABI. |
| [machine.h](machine.h)                                               | Header file specifying what any supported platform must implement (right now, only x86-64). |
| [system.c](system.c) <br /> [system.h](system.h)                     | Syscall wrappers we implement because we don't have libc. |
| [vdl-utils.c](vdl-utils.c) <br /> [vdl-utils.h](vdl-utils.h) <br /> [macros.h](macros.h) | Misc. utility functions and macros, mostly for things normally found in libc. |
| [vdl-alloc.c](vdl-alloc.c) <br /> [vdl-alloc.h](vdl-alloc.h) <br /> [vdl-mem.c](vdl-mem.c) <br /> [vdl-mem.h](vdl-mem.h) | Memory-management functions normally found in libc. |
| [alloc.c](alloc.c) <br /> [alloc.h](alloc.h)                         | Implementation of our internally used memory allocator. |
| [avprintf-cb.c](avprintf-cb.c) <br /> [avprintf-cb.h](avprintf-cb.h) | A self-contained, pretty much fully-featured printf implementation. I don't know where this originally came from; elf-loader had it when I found it, but it was clearly written by someone else. |
| [dprintf.c](dprintf.c) <br /> [dprintf.h](dprintf.h)                 | A minimal wrapper for `avprintf-cb.c`, for use in `stage1.c` before global variables can be used. |
| [vdl-log.c](vdl-log.c) <br /> [vdl-log.h](vdl-log.h)                 | Logging and internal error code. |
| [futex.c](futex.c) <br /> [futex.h](futex.h) <br /> [vdl-list.c](vdl-list.c) <br /> [vdl-list.h](vdl-list.h) <br /> [vdl-rbtree.c](vdl-rbtree.c) <br /> [vdl-rbtree.h](vdl-rbtree.h) <br /> [vdl-hashmap.c](vdl-hashmap.c) <br /> [vdl-hashmap.h](vdl-hashmap.h)| Basic thread-safe data structure implementations (futex and readers-writer lock, doubly linked list, red-black tree, and hash map, respectively). |
| [vdl-gc.c](vdl-gc.c) <br /> [vdl-gc.h](vdl-gc.h)                     | Garbage collector for file structures and maps "freed" by `dlclose()`. |
| [libvdl.c](libvdl.c) <br /> [libvdl.version](libvdl.version) <br /> [vdl-dl-public.c](vdl-dl-public.c) <br /> [vdl-dl-public.h](vdl-dl-public.h) <br /> [vdl-dl.version](vdl-dl.version) <br /> [dl.h](dl.h) | Various pieces of code for building a public-facing API. |
| [readversiondef.c](readversiondef.c)                                 | Source for standalone readversiondef program, which reads fields from system (glibc) libraries, and produces version scripts for building drow-loader. |
| [extract-system-config.py](extract-system-config.py)                 | Script for generating `vdl-config.h` header file, for defining various system-specific glibc values. |
| [doc/](doc/)                                                         | Documentation beyond the basics in the root directory. |
| [doc/references/](doc/references/)                                   | 3rd party documents that are/were useful for understanding the various roles of a dynamic linker/loader in GNU/Linux. |
| [doc/references.txt](doc/references.txt)                             | Citations to sources that would normally go in the `references/` dir, but for whatever reason opted to just cite/link instead. |
| [doc/dl-lmid.txt](doc/dl-lmid.txt)                                   | Documentation on the non-GNU features we support. |
| [doc/challenges.txt](doc/challenges.txt)                             | Some notes from the original elf-loader developers. |
| [test/](test/)                                                       | Source files and scripts for tests. |
| [test/output/](test/output/)                                         | Reference outputs to compare against when running tests. |
| [elfedit.c](elfedit.c)                                               | Source for the standalone elfedit program, which makes an existing ELF binary use elf-loader/drow-loader instead of glibc. |
| [display-relocs.c](display-relocs.c)                                 | Source for the standalone display-relocs program, which displays some symbol table information of the ELF file listed as an argument for debugging purposes. |

