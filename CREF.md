# C Standard Library Reference (Cross-Platform)

Functions in the **ISO C standard library** (C99/C11 baseline). This set ships with the compiler on macOS (clang), Linux (gcc/clang), and Windows (MSVC), so code using only these is portable across all three.

**Not in this doc:** POSIX / OS functions like `fork`, `open`, `read`, `write`, `close`, `pthread_*`, `getline`, `strdup`, `sleep`, `usleep`, `mkdir`, `opendir`. These live in `<unistd.h>`, `<pthread.h>`, etc. Native on Unix, **absent or renamed on Windows** (MSVC). Not portable. Use them only behind `#ifdef`.

---

## Portability gotchas (read first)

- **`long` size differs.** On 64-bit builds: Linux/macOS 64-bit (LP64), Windows 32-bit (LLP64). On 32-bit builds `long` is 32-bit everywhere (ILP32). For a guaranteed-width integer use `<stdint.h>` types (`int32_t`, `int64_t`, `uint64_t`). `int` is 32-bit everywhere common; `long long` is 64-bit everywhere.
- **`char` signedness is implementation-defined.** Use `signed char` / `unsigned char` when the sign matters (e.g. before passing to `<ctype.h>` funcs, cast to `unsigned char`).
- **Binary vs text mode in `fopen`.** Windows translates `\n` ↔ `\r\n` in text mode. Always pass `"rb"`/`"wb"` for binary data. Harmless no-op on Unix.
- **`gets` is gone.** Removed in C11. Never use. Use `fgets`.
- **`sprintf` / `strcpy` / `strcat` have no bounds.** Buffer overflow risk. Prefer `snprintf`, and check lengths before `strcpy`/`strcat`. `strncpy` does **not** null-terminate if source length ≥ `n`.
- **`scanf("%s", ...)` has no bounds.** Use a width: `scanf("%99s", buf)` for a 100-byte buffer. `scanf` also leaves the trailing newline in the buffer; mixing with `fgets` bites people.
- **C11 `<threads.h>`, `<stdatomic.h>`, `<complex.h>` are NOT reliably portable.** MSVC added C11 threads/atomics only recently (VS2022, behind `/std:c11`+`/experimental` historically) and its `complex` is nonstandard. Apple clang shipped no `<threads.h>` for years. For real cross-platform threading/atomics, use C++ `std` or a library (pthreads + Win32, or SDL/glib). Treat these three headers as "maybe."
- **`%zu` for `size_t`, `PRId64`/`PRIu64` for fixed-width.** Modern MSVC supports `%zu`; very old (pre-VS2015) did not. `<inttypes.h>` macros are the portable way to print `int64_t` etc.

---

## `<stdio.h>` — input/output

### File operations

| Function                                                          | Description                                                                       |
| ----------------------------------------------------------------- | --------------------------------------------------------------------------------- |
| `FILE *fopen(const char *path, const char *mode)`                 | Open file. Returns stream or `NULL`. Modes: `r w a r+ w+ a+`, add `b` for binary. |
| `FILE *freopen(const char *path, const char *mode, FILE *stream)` | Reopen `stream` with new file/mode.                                               |
| `int fclose(FILE *stream)`                                        | Close. Flushes buffers. `0` on success, `EOF` on error.                           |
| `int fflush(FILE *stream)`                                        | Flush output buffer. `NULL` flushes all.                                          |
| `int setvbuf(FILE *stream, char *buf, int mode, size_t size)`     | Set buffering. `mode`: `_IOFBF _IOLBF _IONBF`.                                    |
| `void setbuf(FILE *stream, char *buf)`                            | Simpler buffer set. `NULL` = unbuffered.                                          |
| `int remove(const char *path)`                                    | Delete file. `0` on success.                                                      |
| `int rename(const char *old, const char *new)`                    | Rename/move. `0` on success.                                                      |
| `FILE *tmpfile(void)`                                             | Open auto-deleted temp file in `wb+`.                                             |

### Formatted output

| Function                                                | Description                                                                                                         |
| ------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------- |
| `int printf(const char *fmt, ...)`                      | Print to `stdout`. Returns chars written or negative on error.                                                      |
| `int fprintf(FILE *stream, const char *fmt, ...)`       | Print to `stream`.                                                                                                  |
| `int snprintf(char *s, size_t n, const char *fmt, ...)` | Print to buffer, max `n-1` chars + null. **Use this, not `sprintf`.** Returns chars that _would_ have been written. |
| `int sprintf(char *s, const char *fmt, ...)`            | Print to buffer, **no bounds**. Overflow risk.                                                                      |
| `int vprintf(const char *fmt, va_list ap)`              | `printf` taking a `va_list`. Variants: `vfprintf`, `vsnprintf`, `vsprintf`.                                         |

### Formatted input

| Function                                          | Description                                                   |
| ------------------------------------------------- | ------------------------------------------------------------- |
| `int scanf(const char *fmt, ...)`                 | Read from `stdin`. Returns count of items assigned, or `EOF`. |
| `int fscanf(FILE *stream, const char *fmt, ...)`  | Read from `stream`.                                           |
| `int sscanf(const char *s, const char *fmt, ...)` | Read from string.                                             |
| `int vscanf(const char *fmt, va_list ap)`         | `va_list` variants: `vfscanf`, `vsscanf`.                     |

### Character / line I/O

| Function                                    | Description                                                                  |
| ------------------------------------------- | ---------------------------------------------------------------------------- |
| `int fgetc(FILE *stream)`                   | Read one char as `int`. `EOF` at end.                                        |
| `int getc(FILE *stream)`                    | Like `fgetc`, may be macro.                                                  |
| `int getchar(void)`                         | Read char from `stdin`.                                                      |
| `char *fgets(char *s, int n, FILE *stream)` | Read line, max `n-1` chars, keeps `\n`. `NULL` at EOF. **Safe line reader.** |
| `int fputc(int c, FILE *stream)`            | Write one char.                                                              |
| `int putc(int c, FILE *stream)`             | Like `fputc`, may be macro.                                                  |
| `int putchar(int c)`                        | Write char to `stdout`.                                                      |
| `int fputs(const char *s, FILE *stream)`    | Write string, no `\n` added.                                                 |
| `int puts(const char *s)`                   | Write string to `stdout`, **adds `\n`**.                                     |
| `int ungetc(int c, FILE *stream)`           | Push one char back onto stream.                                              |

### Direct (binary) I/O

| Function                                                              | Description                                             |
| --------------------------------------------------------------------- | ------------------------------------------------------- |
| `size_t fread(void *ptr, size_t size, size_t n, FILE *stream)`        | Read `n` objects of `size` bytes. Returns objects read. |
| `size_t fwrite(const void *ptr, size_t size, size_t n, FILE *stream)` | Write `n` objects. Returns objects written.             |

### Positioning

| Function                                        | Description                                            |
| ----------------------------------------------- | ------------------------------------------------------ |
| `int fseek(FILE *stream, long off, int whence)` | Move position. `whence`: `SEEK_SET SEEK_CUR SEEK_END`. |
| `long ftell(FILE *stream)`                      | Current position. `-1L` on error.                      |
| `void rewind(FILE *stream)`                     | Seek to start, clear error.                            |
| `int fgetpos(FILE *stream, fpos_t *pos)`        | Save position (large-file safe). Pair with `fsetpos`.  |
| `int fsetpos(FILE *stream, const fpos_t *pos)`  | Restore saved position.                                |

### Error handling

| Function                      | Description                             |
| ----------------------------- | --------------------------------------- |
| `int feof(FILE *stream)`      | Nonzero if end-of-file flag set.        |
| `int ferror(FILE *stream)`    | Nonzero if error flag set.              |
| `void clearerr(FILE *stream)` | Clear EOF and error flags.              |
| `void perror(const char *s)`  | Print `s: <errno message>` to `stderr`. |

---

## `<stdlib.h>` — general utilities

### Memory

| Function                                         | Description                                                                                    |
| ------------------------------------------------ | ---------------------------------------------------------------------------------------------- |
| `void *malloc(size_t size)`                      | Allocate uninitialized. `NULL` on failure.                                                     |
| `void *calloc(size_t n, size_t size)`            | Allocate `n*size`, zeroed. Checks overflow.                                                    |
| `void *realloc(void *ptr, size_t size)`          | Resize block. May move. On failure returns `NULL` and **leaves old block valid**.              |
| `void free(void *ptr)`                           | Release. `free(NULL)` is safe (no-op).                                                         |
| `void *aligned_alloc(size_t align, size_t size)` | C11. `size` must be multiple of `align`. (MSVC: use `_aligned_malloc` instead — not portable.) |

### String → number conversion

| Function                                                      | Description                                                                                          |
| ------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------- |
| `long strtol(const char *s, char **end, int base)`            | Parse long. `base` 0 = auto-detect. Sets `errno` to `ERANGE` on overflow. **Preferred over `atoi`.** |
| `long long strtoll(const char *s, char **end, int base)`      | Parse long long.                                                                                     |
| `unsigned long strtoul(const char *s, char **end, int base)`  | Parse unsigned long. Also `strtoull`.                                                                |
| `double strtod(const char *s, char **end)`                    | Parse double. Also `strtof`, `strtold`.                                                              |
| `int atoi(const char *s)`                                     | Quick parse, **no error reporting**. UB on overflow.                                                 |
| `long atol(const char *s)` / `long long atoll(const char *s)` | Quick parse to long / long long.                                                                     |
| `double atof(const char *s)`                                  | Quick parse to double.                                                                               |

### Random

| Function                        | Description                                                                |
| ------------------------------- | -------------------------------------------------------------------------- |
| `int rand(void)`                | Pseudo-random `0..RAND_MAX`. Low quality; fine for toys, not crypto/stats. |
| `void srand(unsigned int seed)` | Seed the generator. Common: `srand((unsigned)time(NULL))`.                 |

### Search & sort

| Function                                                                                   | Description                                                           |
| ------------------------------------------------------------------------------------------ | --------------------------------------------------------------------- |
| `void qsort(void *base, size_t n, size_t size, int (*cmp)(const void *, const void *))`    | Sort array in place. `cmp` returns <0 / 0 / >0. Not stable.           |
| `void *bsearch(const void *key, const void *base, size_t n, size_t size, int (*cmp)(...))` | Binary search a **sorted** array. Returns matching element or `NULL`. |

### Process control & environment

| Function                         | Description                                                                            |
| -------------------------------- | -------------------------------------------------------------------------------------- |
| `void exit(int status)`          | Normal exit. Runs `atexit` handlers, flushes streams. `EXIT_SUCCESS` / `EXIT_FAILURE`. |
| `void _Exit(int status)`         | Exit **without** cleanup/handlers.                                                     |
| `void abort(void)`               | Abnormal termination, raises `SIGABRT`.                                                |
| `int atexit(void (*func)(void))` | Register function to run at `exit`.                                                    |
| `char *getenv(const char *name)` | Read env var. Returns `NULL` if unset. (Setting is non-portable.)                      |
| `int system(const char *cmd)`    | Run shell command. `NULL` tests for shell presence.                                    |

### Integer arithmetic

| Function                                                            | Description                                                                 |
| ------------------------------------------------------------------- | --------------------------------------------------------------------------- |
| `int abs(int n)` / `long labs(long)` / `long long llabs(long long)` | Absolute value.                                                             |
| `div_t div(int num, int den)`                                       | Quotient + remainder in one struct (`.quot`, `.rem`). Also `ldiv`, `lldiv`. |

---

## `<string.h>` — strings & memory blocks

### Copy & concatenate

| Function                                              | Description                                                                           |
| ----------------------------------------------------- | ------------------------------------------------------------------------------------- |
| `void *memcpy(void *dst, const void *src, size_t n)`  | Copy `n` bytes. Regions **must not overlap**.                                         |
| `void *memmove(void *dst, const void *src, size_t n)` | Copy `n` bytes, overlap-safe.                                                         |
| `void *memset(void *s, int c, size_t n)`              | Fill `n` bytes with byte `c`.                                                         |
| `char *strcpy(char *dst, const char *src)`            | Copy string + null. **No bounds.**                                                    |
| `char *strncpy(char *dst, const char *src, size_t n)` | Copy ≤ `n` bytes. **Not null-terminated** if `src` ≥ `n`. Pads with nulls if shorter. |
| `char *strcat(char *dst, const char *src)`            | Append. **No bounds.**                                                                |
| `char *strncat(char *dst, const char *src, size_t n)` | Append ≤ `n` chars, always null-terminates.                                           |

### Compare

| Function                                               | Description                                      |
| ------------------------------------------------------ | ------------------------------------------------ |
| `int strcmp(const char *a, const char *b)`             | Compare strings. <0 / 0 / >0.                    |
| `int strncmp(const char *a, const char *b, size_t n)`  | Compare first `n` chars.                         |
| `int memcmp(const void *a, const void *b, size_t n)`   | Compare `n` bytes.                               |
| `int strcoll(const char *a, const char *b)`            | Locale-aware compare.                            |
| `size_t strxfrm(char *dst, const char *src, size_t n)` | Transform for locale compare via plain `strcmp`. |

### Search

| Function                                            | Description                                                                 |
| --------------------------------------------------- | --------------------------------------------------------------------------- |
| `size_t strlen(const char *s)`                      | Length, excluding null.                                                     |
| `char *strchr(const char *s, int c)`                | First occurrence of `c`. `NULL` if none.                                    |
| `char *strrchr(const char *s, int c)`               | Last occurrence of `c`.                                                     |
| `char *strstr(const char *hay, const char *needle)` | First substring. `NULL` if none.                                            |
| `void *memchr(const void *s, int c, size_t n)`      | First byte `c` in `n` bytes.                                                |
| `size_t strspn(const char *s, const char *set)`     | Length of initial run consisting only of chars in `set`.                    |
| `size_t strcspn(const char *s, const char *set)`    | Length of initial run with **no** chars from `set`.                         |
| `char *strpbrk(const char *s, const char *set)`     | First char that is in `set`.                                                |
| `char *strtok(char *s, const char *delim)`          | Split into tokens. **Modifies input, not reentrant** (uses internal state). |

### Misc

| Function                     | Description                          |
| ---------------------------- | ------------------------------------ |
| `char *strerror(int errnum)` | Message string for an `errno` value. |

---

## `<ctype.h>` — character classification

All take `int` (a char as `unsigned char`, or `EOF`) and return `int`. **Cast `char` args to `unsigned char` first** to avoid UB on negative values.

| Function                                    | True when char is…                           |
| ------------------------------------------- | -------------------------------------------- |
| `isalpha(c)`                                | letter                                       |
| `isdigit(c)`                                | decimal digit `0–9`                          |
| `isalnum(c)`                                | letter or digit                              |
| `isspace(c)`                                | whitespace (space, `\t \n \v \f \r`)         |
| `isblank(c)`                                | space or tab (C99)                           |
| `isupper(c)` / `islower(c)`                 | upper / lower case                           |
| `isxdigit(c)`                               | hex digit                                    |
| `ispunct(c)`                                | printable, not space/alnum                   |
| `isprint(c)`                                | printable incl. space                        |
| `isgraph(c)`                                | printable excl. space                        |
| `iscntrl(c)`                                | control char                                 |
| `int tolower(int c)` / `int toupper(int c)` | convert case (returns `c` if not applicable) |

---

## `<math.h>` — floating-point math

Signatures shown for `double`. Each has a `float` variant (`f` suffix, e.g. `sinf`) and a `long double` variant (`l` suffix, e.g. `sinl`). Link with `-lm` on Linux.

| Function                                            | Description                                                                 |
| --------------------------------------------------- | --------------------------------------------------------------------------- |
| `double sqrt(double x)`                             | Square root.                                                                |
| `double cbrt(double x)`                             | Cube root.                                                                  |
| `double pow(double x, double y)`                    | `x` raised to `y`.                                                          |
| `double hypot(double x, double y)`                  | `sqrt(x*x+y*y)`, overflow-safe.                                             |
| `double exp(double x)` / `exp2` / `expm1`           | `e^x` / `2^x` / `e^x - 1`.                                                  |
| `double log(double x)` / `log10` / `log2` / `log1p` | natural / base-10 / base-2 / `log(1+x)`.                                    |
| `double sin/cos/tan(double x)`                      | Trig (radians).                                                             |
| `double asin/acos/atan(double x)`                   | Inverse trig.                                                               |
| `double atan2(double y, double x)`                  | Angle of `(x,y)`, full quadrant.                                            |
| `double sinh/cosh/tanh(double x)`                   | Hyperbolic. Also `asinh/acosh/atanh`.                                       |
| `double fabs(double x)`                             | Absolute value.                                                             |
| `double fmod(double x, double y)`                   | Floating remainder of `x/y`.                                                |
| `double remainder(double x, double y)`              | IEEE remainder. Also `remquo`.                                              |
| `double ceil/floor(double x)`                       | Round up / down.                                                            |
| `double round(double x)`                            | Round half away from zero. Also `lround`, `llround` (return integer types). |
| `double trunc(double x)`                            | Toward zero.                                                                |
| `double rint/nearbyint(double x)`                   | Round per current FP mode. Also `lrint`, `llrint`.                          |
| `double copysign(double x, double y)`               | Magnitude of `x`, sign of `y`.                                              |
| `double fmax/fmin(double x, double y)`              | Larger / smaller.                                                           |
| `double fma(double x, double y, double z)`          | `x*y+z` with one rounding.                                                  |
| `double frexp(double x, int *exp)`                  | Split into mantissa + exponent.                                             |
| `double ldexp(double x, int exp)`                   | `x * 2^exp`.                                                                |
| `double modf(double x, double *ip)`                 | Split fractional + integer parts.                                           |
| `double erf/erfc(double x)`                         | Error function / complement.                                                |
| `double tgamma/lgamma(double x)`                    | Gamma / log-gamma.                                                          |

### Classification (macros, work on any FP type)

| Macro           | Description                                                  |
| --------------- | ------------------------------------------------------------ |
| `isnan(x)`      | True if NaN.                                                 |
| `isinf(x)`      | True if ±infinity.                                           |
| `isfinite(x)`   | True if finite.                                              |
| `isnormal(x)`   | True if normal (not zero/subnormal/inf/nan).                 |
| `signbit(x)`    | True if sign bit set (incl. `-0.0`).                         |
| `fpclassify(x)` | Returns `FP_NAN FP_INFINITE FP_ZERO FP_SUBNORMAL FP_NORMAL`. |

---

## `<time.h>` — dates & time

| Function                                                                     | Description                                                                                                                                                                                                   |
| ---------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `time_t time(time_t *t)`                                                     | Current calendar time (usually Unix seconds). Also stores via `t` if non-NULL.                                                                                                                                |
| `double difftime(time_t end, time_t start)`                                  | Seconds between two `time_t`.                                                                                                                                                                                 |
| `clock_t clock(void)`                                                        | Divide by `CLOCKS_PER_SEC` for seconds. **Measures CPU time on Linux/macOS but wall-clock time on Windows (MSVC) — not portable for benchmarking.** `CLOCKS_PER_SEC` also differs (1000000 glibc, 1000 MSVC). |
| `struct tm *localtime(const time_t *t)`                                      | Break down into local time. **Returns pointer to static buffer** (not reentrant).                                                                                                                             |
| `struct tm *gmtime(const time_t *t)`                                         | Break down into UTC. Same static-buffer caveat.                                                                                                                                                               |
| `time_t mktime(struct tm *tm)`                                               | `struct tm` (local) → `time_t`. Normalizes the struct.                                                                                                                                                        |
| `size_t strftime(char *s, size_t max, const char *fmt, const struct tm *tm)` | Format time to string. `%Y %m %d %H %M %S` etc. **Preferred formatter.**                                                                                                                                      |
| `char *asctime(const struct tm *tm)`                                         | Fixed-format string. Static buffer.                                                                                                                                                                           |
| `char *ctime(const time_t *t)`                                               | `asctime(localtime(t))`. Static buffer.                                                                                                                                                                       |
| `int timespec_get(struct timespec *ts, int base)`                            | C11. High-res time with `TIME_UTC`.                                                                                                                                                                           |

`struct tm` fields: `tm_sec tm_min tm_hour tm_mday tm_mon` (0–11) `tm_year` (since 1900) `tm_wday tm_yday tm_isdst`.

---

## `<stdint.h>` & `<inttypes.h>` — fixed-width integers

**Types (use these for portable sizes):** `int8_t int16_t int32_t int64_t` and `uint8_t … uint64_t`; fast/least variants (`int_fast32_t`, `int_least16_t`); `intptr_t`/`uintptr_t` (hold a pointer); `intmax_t`/`uintmax_t`. Limits: `INT32_MAX`, `UINT64_MAX`, etc.

**`<inttypes.h>` adds:** print/scan format macros — `printf("%" PRId64 "\n", x)`, also `PRIu64 PRIx32 SCNd64` etc. Plus `intmax_t strtoimax(...)`, `imaxabs`, `imaxdiv`.

---

## `<stdbool.h>` & `<stddef.h>` — core types

- `<stdbool.h>` (C99): `bool`, `true`, `false`.
- `<stddef.h>`: `size_t` (unsigned size/index), `ptrdiff_t` (pointer difference), `NULL`, `offsetof(type, member)`, `max_align_t`.

---

## `<assert.h>` — assertions

| Macro                      | Description                                                                                              |
| -------------------------- | -------------------------------------------------------------------------------------------------------- |
| `assert(expr)`             | Abort with message if `expr` is false. Disabled when `NDEBUG` is defined. Don't put side effects inside. |
| `static_assert(expr, msg)` | C11. Compile-time check. (`_Static_assert` is the underlying keyword.)                                   |

---

## `<errno.h>` — error codes

`errno` is a modifiable `int` lvalue, thread-local since C11 (and thread-local in practice on all major implementations before that), set by many library calls on failure. **Only check it after a call that documents it, and after confirming failure** (e.g. return value). Set `errno = 0` before if you need to distinguish. Portable codes include `EDOM`, `ERANGE`, `EILSEQ`. Translate with `strerror`/`perror`.

---

## `<stdarg.h>` — variadic functions

| Macro                   | Description                                            |
| ----------------------- | ------------------------------------------------------ |
| `va_start(ap, last)`    | Init `va_list ap`; `last` is the last named parameter. |
| `type va_arg(ap, type)` | Fetch next argument as `type`.                         |
| `va_copy(dst, src)`     | Copy a `va_list` (needed before reusing).              |
| `va_end(ap)`            | Clean up. Always pair with `va_start`.                 |

---

## `<setjmp.h>` — non-local jumps

| Function                             | Description                                                                      |
| ------------------------------------ | -------------------------------------------------------------------------------- |
| `int setjmp(jmp_buf env)`            | Save context. Returns `0` directly, nonzero when returned to via `longjmp`.      |
| `void longjmp(jmp_buf env, int val)` | Jump back to matching `setjmp`, which then returns `val` (or `1` if `val` is 0). |

Niche (error escape, coroutine-ish). Locals not declared `volatile` may be clobbered. Don't use across function returns.

---

## `<signal.h>` — signals (portable subset)

| Function                                             | Description                                           |
| ---------------------------------------------------- | ----------------------------------------------------- |
| `void (*signal(int sig, void (*handler)(int)))(int)` | Install handler. Special values `SIG_DFL`, `SIG_IGN`. |
| `int raise(int sig)`                                 | Send signal to self.                                  |

Portable signal numbers: `SIGABRT SIGFPE SIGILL SIGINT SIGSEGV SIGTERM`. Inside a handler, only very limited operations are safe in standard C (essentially: set a `volatile sig_atomic_t` flag, call `_Exit`/`abort`). Everything richer is platform-specific.

---

## `<locale.h>` — localization

| Function                                            | Description                                                                                         |
| --------------------------------------------------- | --------------------------------------------------------------------------------------------------- |
| `char *setlocale(int category, const char *locale)` | Set/query locale. `LC_ALL`, `LC_NUMERIC`, etc. `""` = environment default, `"C"` = minimal default. |
| `struct lconv *localeconv(void)`                    | Numeric/monetary formatting info.                                                                   |

---

## `<wchar.h>` / `<wctype.h>` — wide characters

Wide (`wchar_t`) analogues of `<stdio.h>`, `<string.h>`, `<ctype.h>`. **Portability note:** `wchar_t` is 16-bit on Windows (UTF-16) and 32-bit on Linux/macOS (UTF-32) — do not assume it holds a full Unicode code point. For portable Unicode use UTF-8 in `char` buffers, or `<uchar.h>` (`char16_t`, `char32_t`, C11).

Common functions: `wprintf`, `swprintf`, `fgetws`, `fputws`, `wcslen`, `wcscpy`, `wcscmp`, `wcsstr`, `wcstok`, `wcsftime`, plus `iswalpha`/`iswdigit`/`towlower` etc. in `<wctype.h>`. Conversions: `mbstowcs`, `wcstombs`, `mbrtowc`, `wcrtomb`.

---

## `<fenv.h>` — floating-point environment

| Function                                                    | Description                                                                                 |
| ----------------------------------------------------------- | ------------------------------------------------------------------------------------------- |
| `int fegetround(void)` / `int fesetround(int mode)`         | Get/set rounding: `FE_TONEAREST FE_DOWNWARD FE_UPWARD FE_TOWARDZERO`.                       |
| `int fetestexcept(int excepts)`                             | Check raised FP exceptions (`FE_DIVBYZERO FE_INVALID FE_OVERFLOW FE_UNDERFLOW FE_INEXACT`). |
| `int feclearexcept(int excepts)` / `int feraiseexcept(int)` | Clear / raise FP exception flags.                                                           |

Requires `#pragma STDC FENV_ACCESS ON` to be strictly correct. Support varies; functional on all three but compiler honoring of the pragma is uneven.

---

## C11 headers to treat as "maybe portable"

Standardized but **inconsistently implemented**. Verify on your target before relying on them.

| Header          | Provides                                                          | Reality                                                                                    |
| --------------- | ----------------------------------------------------------------- | ------------------------------------------------------------------------------------------ |
| `<threads.h>`   | `thrd_create`, `mtx_lock`, `cnd_wait`, `tss_*`, `call_once`       | Missing on Apple clang for years; late/partial in MSVC. Use pthreads+Win32 or C++ instead. |
| `<stdatomic.h>` | `_Atomic`, `atomic_load/store`, `atomic_fetch_add`, `atomic_flag` | Late MSVC support; works on recent gcc/clang.                                              |
| `<complex.h>`   | `_Complex`, `complex`, `cabs`, `creal`, `cimag`, `cexp`           | MSVC implementation is nonstandard (`_Dcomplex`, no native `_Complex`).                    |
