# How to build the application #

## Needed program/libraries ##
  1. MinGW (Any GNU C++ should be okay, but this program is only tested under Windows XP)
  1. MSYS (to compile PDFlib Lite 7)
  1. Qt4 Open Source Edition http://trolltech.com/downloads/opensource
  1. QuaZIP http://quazip.sourceforge.net/
  1. PDFlib Lite 7 http://www.pdflib.com/download/free-software/pdflib-lite/

## Build Procedures ##
  1. Download and install MinGW
  1. Download and compile Qt4. Commands:
    1. `configure -release -static -no-exceptions -no-qt3support -plugin-sql-sqlite -plugin-sql-odbc -qt-libpng -qt-libjpeg`
    1. `mingw32-make release`
  1. Download and compile QuaZIP. Put libquazip.a and all headers in the MinGW directory.
  1. Download and compile PDFlib Lite 7 using MSYS.
    1. The library is configured to produce static library only.
    1. Change `#undef PDC_UNICODE_FILENAME` to `#define PDC_UNICODE_FILENAME` in libs/pdcore/pc\_config.h _note: we absolutely need Unicode_
    1. `make`
    1. Add bind/pdflib/cpp/pdflib.lo to lib/pdflib/.libs/pdflib.a using the libtool given by PDFlib (note: I've forgotten the exact command line)
    1. Put the final library and the C & C++ headers in MinGW directory. The library should be named libpdf.a
  1. Compile this program