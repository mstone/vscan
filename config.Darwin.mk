libc_CPPFLAGS :=

libarchive_LDFLAGS := -larchive

libz_LDFLAGS := -lz

libre2_LDFLAGS := -lre2 -lpthread

libsqlite3_LDFLAGS := -lsqlite3 -lpthread

liblua_CPPFLAGS := -I/usr/local/include
liblua_LDFLAGS := -llua -lm

# vim: noet sts=4 ts=4 sw=4 :
