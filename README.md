# Introduction

This is a personal project for an assembler. It's syntax is similar to Intel's with a few added functions.
It has no third party libraries apart from pugixml used for reading the opcode reference,
which is planned to be removed and replaced by a self-made file reader made specifically for the opcode reference
to make the program smaller and completely independent (except for the opcode reference itself which would be dumb to rewrite).<br>

The compiler is not finished and therefore does not create correctly compiled objects yet.
All the functions described below may not work on the current version.



# Usage

basm \<command> [\<options>] [\<file> \<file>...]



## Commands

1. help / -help / --help<br>
   Displays help message
2. compile<br>
   Compile all following files



## Executable

The compiler does not include a linker. This means that if you want to create an executable,
you will have to use a third party linker. Any linker should work.



# Syntax

Sections are marked with the "section" keyword, followed by it's name: ".text", ".data", ".bss".
Section marks must be at the start of the line with no indentation allowed.
Multiple sections of the same type are not allowed, e.g. having two .text sections in one file.

There are three keywords that define the scope of a variable or function declaration:<br>
A "static" symbol is inaccessible to the files this file gets linked to.<br>
A "global" symbol is accessible to other files, it is therefore exported.<br>
An "extern" symbol is imported from another file.<br>
The keyword "static" is used exclusively for functions, variables must not have that keyword.



## Variables

There is one variable size family yet. The family consists of fixed size variables, they will have the exact size you choose:<br>
"byte" for 8 bits, "word" for 16 bits, "dword" for 32 bits, "qword" for 64 bits<br>
More types could be added.



### .data section

This section contains initialised modifiable data.
The syntax in this section looks like this:<br>
[global] \<size> \<name> = \<value><br>

Notice that the spaces around '=' are required.<br>
Global variables have to be initialised.



### .bss section

This section contains uninitialised modifiable data.
The syntax is similar to the .data section's syntax:<br>
[extern] \<size> \<name><br>

Extern variables are not allowed to be initialised.



## .text section

This section contains executable code.
The syntax is derived from Intel's syntax, but has a few added functions:<br>
[\<prefix>...] \<mnemonic> [\<arg> \<arg>...] [; \<comment>]<br>

Apart from the standard arguments seen in Intel's syntax,
basm also supports an array access shortcut from the C language: "pointer[index]".<br><br>


Functions are declared similarly to variables, followed by the definition:<br>
static/global \<name>:<br>
Names without a "global" or "static" keyword are interpreted as lables.<br>
A function import is similar to a data import, but without a size:<br>
extern \<name><br><br>


Exiting the main function with a "retn" is required.