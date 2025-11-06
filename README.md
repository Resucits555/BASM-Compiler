# Introduction

This is a personal project for an assembly compiler. It's syntax will be similar to Intel's.
It has no third party dependencies apart from some that are only there because it's busy work,
like the xmlreference so there's no need to rewrite all the mnemonics, opcodes and argument attributes, + a library to read it.



# Usage

basm <command> \[<options>] \[<file>] \[<file>...] \[-o <outputname>]



## Commands

1. help / -help / --help  
   Displays help message
2. compile  
   Compile all specified files