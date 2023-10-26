In order to analyze whether a header file is "self-contained", the checker
compiles the header file seperately and determines whether it's self-contained
based on the compilation results. However, because the current solution doesn't
consider the influence of compilation options like "-I", sometimes the seperate
compilation fails all though it's self-contained in fact. In order to solve
that, the compilation options are expected to add to the header file compilation.

In the libtooling, the checker reads header files which a file (source file or
header file) depending on and puts them in a temporary file, so the Go code
could read them later. The filename is assigned by the Go program randomly.
Every line in the file represents a pair of file and header file seperated by a
space. It's like
    (filename in absolute path) (header filename in relative path)\n
.

In the Go code, the checker finds all include paths for header files in function
`getHeaderCompileDirOptionsMap` as follows:

Step 1. Get relationship between files and header files:
Call function `getIncludeRelationship`, which reads the temporary file and parse
the including relationship, like
    file (src or header) - [headerFile1, headerFile2, ...]
and stores them in a map named `rawFileHeadersMap`.

Step 2. Get all compilation dir options of source files:
In function `getAbsSrcCompileDirOptions`, the checker gets all the absolute
including path options of source files from build actions.

Step 3. For a header file, find all source files which include it:
In function `getHeaderSrcsMap`, we recursively traverse `rawFileHeadersMap`, to
find all header files that a source file directly or indirectly include. In this
way, we can make a map `headerSrcsMap`. The key of `headerSrcsMap` is a header
file and its value is a set of source files that include it.

Step 4. Get all compilation dir options of header files:
Make a new map `headerCompileDirOptionsMap` based on `headerSrcsMap`, replacing
paths of source files with compilation dir options of source files.

Finally, when the header file is going to be compiled, the checker walks through
all compilation dir options and tries to compile it in all these compilation
option arrays seperately. If the header file succeeds in any compilation option
array, it's considered as self-contained.
