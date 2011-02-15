/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include <algorithm>
#include <cstdlib>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <iostream>
#include <list>
#include <map>
#include <re2/re2.h>
#include <re2/stringpiece.h>
#include <sstream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>
#include <vector>

using re2::StringPiece;
using std::cerr;
using std::cin;
using std::cout;
using std::getline;
using std::istringstream;
using std::list;
using std::map;
using std::min;
using std::ostream;
using std::ostringstream;
using std::pair;
using std::string;
using std::vector;

#include "compat_fstatat.h"
#include "compat_openat.h"
