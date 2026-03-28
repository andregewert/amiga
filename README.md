# Amiga
Playground for some amiga-related tests and projects


## AppSupport
This folder contains some generic structures and helper functions:

- Archive
Simple functions for wrapping files in an archive. The goal is to allow straightforward access to the contained files from a C program.

- Collections  
Linked lists, hash tables, trees

- Environment  
Functions for getting information about the host system like preferred language
and character encoding.

- filetools  
Functions for creating temporary files and directories.

- network  
Functions for fetching contents from urls. Require Roadshow and AmiSSL SDKs to compile.

- shell  
Functions for colored text output.

- testutils
Some macros and functions for testing.

- windows  
Functions for querying information about open windows and for manipulating them.


## AppSupportTests
Some simple tests for the AppSupport library.


## BuildingBlocks
These are some simple code examples for demonstrating specific function calls or library usages.


## Shelltools

- cldir  
Outputs colored directory listings.
- fetch  
Fetches a file from a url. Support HTTP and HTTPS (requires AmiSSL).
- mktemp  
Utility for creating temporary files and directories.
