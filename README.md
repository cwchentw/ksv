# ksv

A lightweight, pure C CSV/TSV parsing library.

## Overview

**ksv** is a minimal, dependency-free CSV/TSV parser written in ANSI C. It is designed to be portable, embeddable, and simple to integrate into C or C++ projects. The project includes a basic console tool and an optional C++ wrapper.

## System Requirements

* ANSI C-compliant compiler
* GNU Make

Tested with:

* GCC, Clang, Visual C++, Intel C++ Compiler
* Windows 10, Ubuntu 20.04, Fedora 35, openSUSE Leap 15.3, FreeBSD 13.0

> On BSD systems, use `gmake` instead of `make`. On Windows, use `mingw32-make` and consider installing from [MSYS2](https://www.msys2.org/) or [GnuWin32](http://gnuwin32.sourceforge.net/).

## Installation

### macOS (via Homebrew)

```sh
$ git clone https://github.com/cwchentw/ksv.git
$ cd ksv
$ brew install --HEAD ksv.rb
```

### Unix-like Systems

```sh
$ git clone https://github.com/cwchentw/ksv.git
$ cd ksv
$ make
$ make clean-obj
$ make exec
$ sudo make install
```

To install in a custom location:

```sh
$ sudo make install prefix=/opt
```

### C++ Binding

```sh
$ make dynamic-cpp
$ make clean-obj
$ make static-cpp
$ sudo make install-cpp
```

## Build Targets

* Dynamic C library:

  ```sh
  $ make
  ```

* Static C library:

  ```sh
  $ make static
  ```

* Console tool:

  ```sh
  $ make exec
  ```

* C++ dynamic binding:

  ```sh
  $ make dynamic-cpp
  ```

* C++ static binding:

  ```sh
  $ make static-cpp
  ```

## Library Usage

### C API Example

See full example in [`examples/csv_to_tsv.c`](examples/csv_to_tsv.c).

```c
FILE *fp = fopen("sheet.csv", "r");
ksv_t *ksv = ksv_new_default();

if (KSV_SUCCESS != ksv_load_header(ksv, fp)) exit(1);

char *field = ksv_next_header(ksv);
while (field) {
    printf("%s\t", field);
    field = ksv_next_header(ksv);
}

while (!feof(fp)) {
    if (KSV_SUCCESS != ksv_load_record(ksv, fp)) break;
    ksv_start(ksv);
    field = ksv_next_data_by_row(ksv);
    while (field) {
        printf("%s\t", field);
        field = ksv_next_data_by_row(ksv);
    }
    printf("\n");
}

ksv_delete(ksv);
fclose(fp);
```

### C++ API Example

See full example in [`examples/csv_to_tsv_cpp.cpp`](examples/csv_to_tsv_cpp.cpp).

```cpp
FILE *fp = fopen("sheet.csv", "r");
KSV *ksv = new KSV();

if (!ksv->load_header(fp)) exit(1);

std::string field = ksv->next_header();
while (!field.empty()) {
    std::cout << field << "\t";
    field = ksv->next_header();
}

while (!feof(fp)) {
    if (!ksv->load_record(fp)) break;
    ksv->start();
    field = ksv->next_data_by_row();
    while (!field.empty()) {
        std::cout << field << "\t";
        field = ksv->next_data_by_row();
    }
    std::cout << std::endl;
}

delete ksv;
fclose(fp);
```

## CLI Usage

```sh
$ ksv width path/to/sheet.csv         # Show number of columns
$ ksv height path/to/sheet.csv        # Show number of rows
$ ksv dimension path/to/sheet.csv     # Show dimensions
$ ksv table path/to/sheet.csv         # Display as table
$ ksv stats quartile path/to/sheet.csv  # Quartiles
$ ksv stats quintile path/to/sheet.csv  # Quintiles
$ ksv help                            # Show usage
```

> Note: `ksv` CLI is provided for convenience and testing, and is not a fully featured CSV tool.

## Known Issues and Bugs

* **Multiline Support**: `ksv table` does not currently support fields containing newline characters.
* **Trailing Empty Lines**: `ksv stats` commands may crash when encountering a trailing empty line at the end of a CSV file.

## License

MIT License © 2020 ByteBard
