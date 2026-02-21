# Introduction

This is a personal project for an assembler. It's syntax is similar to Intel's.<br>



# Usage

basm \<command> [\<file>...]



## Commands

1. help / -help / --help<br>
   Displays help message
2. compile<br>
   Compile all following files<br>



## Executable

The compiler does not include a linker. This means that if you want to create an executable,
you will have to use a third party linker. Any linker should work.<br>
The compiler only supports the win64 object format.



# Syntax

The compiler does not have full functionality, some registers are not supported and there are a lot of bugs hidden.<br>

Sections are marked with the "section" keyword, followed by it's name: ".text", ".data", ".bss".<br>
Section declarations must be at the start of the line with no indentation allowed.
Multiple sections of the same type are not allowed, like having two .text sections in one file.

There are three keywords that define the scope of a variable or function:<br>
A "static" symbol is inaccessible to the files this file gets linked to.<br>
A "global" symbol is accessible to other files, it is therefore exported.<br>
An "extern" symbol is defined in another file, it is therefore imported.<br>
The keyword "static" is used exclusively for functions, variables must not have that keyword.<br>

Note that spaces around '=' or after commas are required, unlike in other languages and compilers.



## Variables

There is only one variable type family. The family consists of fixed size variables, they will have the exact size you choose:<br>
"byte" for 8 bits, "word" for 16 bits, "dword" for 32 bits, "qword" for 64 bits<br>



### Initialized data sections

These sections contain initialized data.
The syntax looks like this:<br>
[global] \<type> \<name> = \<value><br>

Variables in this section have to be initialized.<br>

For any string, use doubled quotation marks: "abc".
Quotation marks inside the string do not need to be marked in any way, 
they're taken as any other character in the string.



### .bss section

This section contains uninitialized modifiable data.
The syntax is similar to the .data section's syntax:<br>
[extern] \<type> \<name>[\\[\<size>\\]]<br>

Optionally, a size can be given in square brackets. That will give the variable "type * size" bytes of memory.



## .text section

This section contains executable code.
The syntax is derived from Intel's syntax:<br>
\<mnemonic> [\<arg> \<arg>...] [; \<comment>]<br>

Functions are declared similarly to variables, followed by the definition:<br>
static/global \<name>:<br>
Names without a "global" or "static" keyword are interpreted as labels.<br>
A function import is similar to a data import, but without a size:<br>
extern \<name><br>

Every relative address has to be marked with the "rel" keyword,
like in function calls and jumps, or memory access.<br>