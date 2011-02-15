libc_CPPFLAGS := -DHAVE_FDOPENDIR -DHAVE_OPENAT -DHAVE_FSTATAT -DHAVE_LSEEK64

libarchive_LDFLAGS := -larchive

libz_LDFLAGS := -lz

libre2_LDFLAGS := -lre2 -lpthread

libsqlite3_LDFLAGS := -lsqlite3 -lpthread

liblua_CPPFLAGS := -I/usr/include/lua5.1
liblua_LDFLAGS := -llua5.1 -lm -Wl,-E -ldl
