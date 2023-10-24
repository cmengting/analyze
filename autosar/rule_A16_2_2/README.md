To know whether include directives are used or not, this checker will modify
source codes and run some compilation tests. Then it can get results from
compilation test results.

* Note that now the checker only implement check for source files (e.g., *.cc,
*.cpp), but we don't check unused include directives in header files.

The checker works as follows:

First, we get file names of all source files by calling `GetSourceFiles()`, and
generate a command argument array for later compilation.

Then we copy source files into temp files by calling `copyToTempFile()`, and use
`tempFileInfoArray` to save the source file path of the temp file (when we report
error, we need the source file path).

We try to compile temp files before we modify them. If there are some errors,
stop and return. We can't run check by running following tests, because
compilation tests will always fail.

In `runCompileTest()`, for each temp file (which corresponds to a source file),
we read by line. For each line, if it is an include directive, we delete it and
run a compilation test. If the deletion cause errors, then we know it is actually
used in the file. So we write it back. If files can be compiled without errors,
that means the include directive is redundant and we report error here.

We repeat the deletion and compilation steps until we finish checking all lines
in all temp files. In this process, all check results will be reported.
