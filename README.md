# drow-loader
This is the repo for drow-loader, a glibc-compatible dynamic loader for x86-64 ELF executables. It is written primarily for use with the [Shadow network simulator](https://github.com/shadow/shadow). Its major advantage over the standard glibc dynamic linker is support for a practically unlimited number of namespaces, as created by [dlmopen](http://man7.org/linux/man-pages/man3/dlopen.3.html), but it supports a number of other features as well. It is forked from [elf-loader](https://www.nsnam.org/docs/dce/manual/html/dce-user-elfloader.html), also known as VDL.

**:warning: Do not use drow-loader with any program that handles untrusted input or runs as root! :warning:**

While drow-loader is compatible with programs linked against glibc, and is a drop-in replacement for research purposes, it is *not* intended as a drop-in replacement for normal use. For one, while drow-loader has the advantage of being a smaller, more easily managed code base than glibc, it has not had scrutiny applied to it the way the most popular Linux libc implementation has. As in any complicated C program, there is almost cerainly some form of vulnerability somewhere. More importantly though, because drow-loader is not intended for use outside of research purposes, it forgoes implementing some of the most basic security precautions one would expect from a dynamic linker, such as checking setuid before preloading, or ASLR. This means that not only are calls into the dynamic linker or loader potentially vulnerable, but the program being run (and its dependencies) will not have the normal protections they should (such as "don't let arbitrary users inject code"). For something like a network simulation, which never touches the actual internet and runs as a normal user, this is fine. But if you were to, e.g., run your normal Firefox session with it and browse the web, or ever run anything with it that uses setuid, you would be asking for trouble.

## Requirements
Currently, drow-loader only supports x86-64 Linux with glibc. To build drow-loader, you will need gcc and gcc-c++ (or llvm and clang), python 2, make, cmake, and debug symbols for glibc. The rest of this is written assuming you're using gcc, but clang should work by setting the CC environment variable as appropriate.
### YUM (Fedora/CentOS):
```
sudo yum install -y gcc gcc-c++ python make cmake
sudo debuginfo-install glibc
```
### APT (Debian/Ubuntu):
```
sudo apt-get install -y gcc g++ python make cmake libc-dbg
```

## Building
The project uses cmake for building and testing. A typical, fresh setup and build (with tests) might look like:
```
git clone https://github.com/jtracey/drow-loader.git
mkdir drow-loader/build
cd drow-loader/build
cmake ../. -DSHADOW_TEST=ON
make
ctest
```

### Build Options
Because of the crucial role the dynamic linker plays in program execution, drow-loader can behave quite differently from normal when built with debugging or valgrind enabled. While the valgrind option is only neccessary if you are testing drow-loader itself, the debugging option (`-DDEBUG`) should be enabled if you plan on using gdb at all -- even if the program being debugged isn't drow-loader. If you don't do this, you will see breakpoints applying to namespaces they shouldn't, and breakpoints being erased without gdb's knowledge when the same executables are loaded in another namespace. This is the result of a memory-saving technique, so if you are not planning on running in gdb and are concerned about memory overhead, do not compile drow-loader with `DEBUG` defined.

## Using
You can use drow-loader as a drop-in replacement for libdl but with larger namespace counts, or you can take advantage of some of the extra features documented in doc/dl-lmid.txt.

There are two components of drow-loader: the dynamic linker, and the dynamic loader. You will need both for drow-loader to work.

### Dynamic Linker
The dynamic linker is an ELF file named "ldso" in the build directory. The path of the dynamic linker must be specified in the main executable of your program prior to execution. This can be done at link time, or by modifying a binary. Link time should generally be preferred if you're able to compile the main executable from source.

To set the path at link time, add `--dynamic-linker=/path/to/ldso` to the arguments of the linker (as usual, you can also use `-Wl` to pass linker arguments to the compiler instead).

To modify an existing ELF file, drow-loader also compiles a tool called elfedit. This tool will find the dynamic-linker string in the ELF file, and replace it with one you provide. However, because the offsets of the file can't be modified without re-linking, the string passed must be the same length or shorter than the existing path to the glibc dynamic linker on your system. To use elfedit, simply run `elfedit your-binary /path/to/ldso ; echo $?`, where `echo $?` checks the return code (it should be 0).

### Dynamic Loader
The dynamic loader is an ELF shared object file (i.e. a library) named "libvdl" in the build directory. If only standard dynamic loading symbols like `dlmopen` are used, our dynamic linker can replace them at runtime. Otherwise, it is neccessary to link against if you're using any dynamic loading symbol not defined in glibc's version, e.g. `dl_lmid_swap_tls` (this is done the same way you would link to any shared library). In either case, it will need to be in the search path of the executable at run time. The path can either be supplied at link time, by using the rpath or runpath linker options (note that only one of the two can be used, see `man ld` for more information), or at run time, using the `LD_LIBRARY_PATH` environment variable.

### Running your program
Once the dynamic linker and dynamic loader are set up for your program, it should be able to run normally. Logging levels for drow-loader can be specified using the `LD_LOG` environment variable (`:` separated). Setting it to `symbol-ok` will log when symbols are succesfully resolved, while `symbol-fail` will log when symbol resolution fails. Setting it to `debug` prints some debugging information, and setting it to `function` prints extremely verbose information about execution.
