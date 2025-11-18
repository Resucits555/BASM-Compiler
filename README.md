# Introduction

This is a personal project for an assember. It's syntax is similar to Intel's with a few added functions.
It has no third party libraries apart from pugixml used for reading the opcode reference,
which is planned to be removed and replaced by a self-made file reader made specifically for the opcode reference.



# Usage

basm \<command> [\<options>] [\<file>] [\<file>...] [-o \<outputname>]



## Commands

1. help / -help / --help<br>
   Displays help message
2. compile<br>
   Compile all specified files



# Syntax

Sections are marked with the "section" keyword, followed by it's name: ".text", ".data", ".bss". Section marks have to be at the start of the line, no indentation is allowed.



## .data section

This section contains initialised writeable data.<br>
The syntax in this section looks like this:<br>
[static] \<size> [\<name>] = \<value><br><br>

Sizes are chosen with the keywords "byte", "word", "dword" and "qword"<br>
The "static" keyword makes the value inaccessible to other files this file gets linked to.



## .bss section

This section contains uninitialised writeable data.<br>
The syntax is similar to the .data section's syntax, but without the initialising value and another function:<br>
[static/extern] \<size> [\<name>]<br><br>

The "extern" keyword marks the value as defined in another file



## .text section

This section contains executable code.<br>
The syntax is derived from Intel's syntax, but has a few added functions:<br>
[\<prefix>...] \<mnemonic> [\<arg>...] [; \<comment>]<br><br>

Apart from the standard arguments seen in Intel's syntax, basm also supports array shortcut from the language C: "pointer[index]".