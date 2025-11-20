# Introduction

This is a personal project for an assembler. It's syntax is similar to Intel's with a few added functions.
It has no third party libraries apart from pugixml used for reading the opcode reference,
which is planned to be removed and replaced by a self-made file reader made specifically for the opcode reference
to make the program smaller and completely independent (except for the reference itself which would be dumb to rewrite).



# Usage

basm \<command> [\<options>] [\<file> \<file>...]



## Commands

1. help / -help / --help<br>
   Displays help message
2. compile<br>
   Compile all specified files



## Executable

The compiler does not include a linker. This means that if you want to create an executable,
you will have to use another linker to link the resulting object files. Any linker should work.



# Syntax

Sections are marked with the "section" keyword, followed by it's name: ".text", ".data", ".bss".
Section marks must be at the start of the line with no indentation allowed.



## .data section

This section contains initialised modifiable data.
The syntax in this section looks like this:<br>
[static] \<size> [\<name>] = \<value><br><br>

Sizes are chosen with the keywords "byte", "word", "dword" and "qword"<br>
The "static" keyword makes the value inaccessible to other files this file gets linked to.



## .bss section

This section contains uninitialised modifiable data.
The syntax is similar to the .data section's syntax, but without the initialising value and an another possible keyword:<br>
[static/extern] \<size> [\<name>]<br><br>

The "extern" keyword marks the value as defined in another file.



## .text section

This section contains executable code.
The syntax is derived from Intel's syntax, but has a few added functions:<br>
[\<prefix>...] \<mnemonic> [\<arg> \<arg>...] [; \<comment>]<br><br>

Apart from the standard arguments seen in Intel's syntax,
basm also supports an array access shortcut from the language C: "pointer[index]".



# P.S.

The compiler is not finished and therefore does not create correctly compiled objects yet.
All the functions described above may not work on the current version.