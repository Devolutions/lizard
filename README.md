# Lizard: A 7-Zip packer that sticks

Have you ever wanted to package your application as a single executable
that extracts itself when launched? You have come to the right place.

Traditional packers take an executable, compress its executable segment
and make a new executable that will decompress it in memory when launched.

Lizard aims at simplifying the task of decompressing a 7-Zip archive to
a temporary location from which an executable will be launched.

This project was originally created with a specific use case in mind:
creating an executable that contains both 32-bit and 64-bit versions
of a Windows executable. When launched, the packer detects if it is
running inside a 32-bit or 64-bit Windows environment, and proceeds
to extract and launch the optimal version of the program. Since both
executables are just different versions of the same program, their
resource segments are identical. Because they are compressed inside
the same 7-Zip archive rather than individually, the resource segment
is correctly compressed only once, resulting in efficient compression.

Lizard does not provide specific tooling to generate packed executables.
It is a library that contains a 7-Zip decompressor (LZMA SDK), along with
portability functions to deal with environment detection, files, paths,
Unicode encoding, etc. Embedding the 7-Zip archive inside the packer
program can be done using platform-specific resource compilers.
Otherwise, the [YARC](https://github.com/wayk/yarc) resource compiler can be used to achieve this task.

## Sample Program Usage

`lizard [options]`

Options:
 * `-e`                extract files from archive
 * `-l`                list files from archive
 * `-i` <file>         input file
 * `-o` <path>         output path
 * `-h`                print help
 * `-v`                print version (1.0.0)
 * `-V`                verbose mode

