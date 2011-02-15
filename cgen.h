/*
Copyright (c) 2008-2011 Michael Stone <michael@laptop.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#define STATIC_ASSERT(expr)  extern char __static_assertion_failed [(expr) ? 1 : -1]
#define SAVE_ERR(EXPR) {int __errno_save = errno; EXPR; errno = __errno_save;}
#define __XSTRING(X) __STRING(X)
#define MAYBE10_IMPL(__IGNORE, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, n, ...) n
#define MAYBE10(X, ...) MAYBE10_IMPL(/**/, ##__VA_ARGS__, X, X, X, X, X, X, X, X, X, X, {})
#define PERROR(msg, ...) {int __errno_cache = errno; fprintf(stderr, "E %s|%d| %s: %s\n", __FILE__, __LINE__, __func__, msg); if (__errno_cache) {fprintf(stderr, "E errno %d: %s\n", __errno_cache, strerror(__errno_cache));}; MAYBE10(fprintf(stderr, "E " __VA_ARGS__), ##__VA_ARGS__);}
/*#define PERROR(msg) {int __errno_cache = errno; if (getenv(DEBUG) != NULL) {fprintf(stderr, "%s|%d| %s: %s\n", __FILE__, __LINE__, __func__, msg); if (__errno_cache) {fprintf(stderr, "Error %d: %s\n", __errno_cache, strerror(__errno_cache));}};} */
/* #define PERROR(msg) */
#define CHK(EXPR, MSG, ERR_LABEL, ...) {if(EXPR) { PERROR(MSG, ## __VA_ARGS__); goto ERR_LABEL;}}
#define LET(LETEXPR, CONDEXPR, MSG, ERR_LABEL, ...) LETEXPR; if (CONDEXPR) { PERROR(MSG, ## __VA_ARGS__); goto ERR_LABEL;}
#define TST(EXPR, TRUE, MSG, ERR_LABEL, ...) {if (EXPR) {(TRUE); PERROR(MSG, ## __VA_ARGS__); goto ERR_LABEL;}}
#define INIT(BUF, LEN, TYPE, INIT, MSG, ERR_LABEL) { CHK(*(LEN) < sizeof(TYPE), (MSG), ERR_LABEL); (*(TYPE*)(*(BUF))) = (INIT); (*(LEN)) -= sizeof(TYPE); (*(BUF)) += sizeof(TYPE); }
#define COPY(BUF, LEN, TYPE, SRC, MSG, ERR_LABEL) { CHK(*(LEN) < sizeof(TYPE), (MSG), ERR_LABEL); memcpy(*(BUF), (SRC), sizeof(TYPE)); (*(LEN)) -= sizeof(TYPE); (*(BUF)) += sizeof(TYPE); }

#define WARN_RET __attribute__ ((warn_unused_result))

#define DBERR "SQLite error: %d, %d, %s\n", ret, sqlite3_errcode(db), sqlite3_errmsg(db)

// A Google macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
   TypeName(const TypeName&);              \
   void operator=(const TypeName&)
