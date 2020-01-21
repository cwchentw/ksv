# ksv

Yet another CSV or TSV library for C.

## System Requirements

* A C compiler that supports ANSI C
* GNU Make
* Valgrind (only for tests on GNU/Linux)

To use GNU Make on non-Linux Unix or Unix-like systems, use `gmake` instead of `make`. To use GNU Make on Windows, use `mingw32-make`.

We compile and run **ksv** with GCC, Clang, Visual C++ and Intel C++ Compiler.

We test **ksv** against several Unix or Unix-like systems:

* Ubuntu 18.04 LTS
* CentOS 8
* openSUSE Leap 15.1
* TrueOS, which is FreeBSD-compatible
* Solaris 11

In addition, we test **ksv** against Windows 10 as well.

## Usage of the Library

### C API

See a full example [here](/example/csv_to_tsv.c).

Load a CSV sheet into a C file stream:

```c
FILE *fp = fopen("path/to/sheet.csv", "r");
```

Create a `ksv` object:

```c
ksv_t *ksv = ksv_new_default();
```

Load sheet header:

```c
/* Load sheet header. */
if (KSV_SUCCESS != ksv_load_header(ksv, fp)) {
    goto ERROR_MAIN;
}
```

Read each header field:

```c
/* Read each header field. */
char *field = ksv_next_header(ksv);
while (field) {
    printf("%s\t", field);

    field = ksv_next_header(ksv);
}
printf("\b\n");
```

Load sheet records and read the fields in each record line by line:

```c
/* Load sheet records line by line. */
while (!feof(fp)) {
    /* Load a sheet record. */
    if (KSV_SUCCESS != ksv_load_record(ksv, fp)) {
        fprintf(stderr, "Failed to load record\n");
        goto ERROR_MAIN;
    }

    /* Reset the internal index. */
    ksv_restart(ksv);

    /* Read each field in the record. */
    field = ksv_next_data_by_row(ksv);
    while (field) {
        printf("%s\t", field);

        field = ksv_next_data_by_row(ksv);
    }
    printf("\b\n");
}
```

Release system resources:

```c
ksv_delete(ksv);
fclose(fp);
```

### C++ API

Pending.

## Usage of the Console Tool

Pending.

## Known Issues or Bugs

* C file streams may not work in C++ binding
* The dynamic libraries compiled by MSVC still crash

## Note

Currently, **ksv** is distributed mainly as a C library. The **ksv** cli tool is just a byproduct of the library, never a feature-rich CSV or TSV tool.

We copy some utility code from [cwchentw/clibs](https://github.com/cwchentw/clibs).

## Copyright

Copyright (c) 2020 Michael Chen. Licensed under MIT.
