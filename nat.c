/* Copyright (c) 2011 Akamai Technologies, Inc. */

#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>

#include "cgen.h"
#include "nat.h"

int nat_add(unsigned long* result, unsigned long arg)
{
   unsigned long tmp = *result + arg;
   if (tmp < arg) {
      errno = EOVERFLOW;
      return 1;
   }
   *result = tmp;
   return 0;
}

int nat_mult(unsigned long* result, unsigned long long arg)
{
    unsigned long long tmp = arg * (*result);
    if (tmp > (unsigned long)-1) {
       errno = EOVERFLOW;
       return 1;
    }
    *result = ((unsigned long) -1) & tmp;
    return 0;
}

int parse_nat(unsigned long* result, const char* buf, size_t buflen) {
  unsigned long total = 0;
  for (size_t i = 0; i < buflen; i++)
  {
      LET(char num = buf[i] - '0', num < 0 || num > 9,
          "Non-decimal character in string.", out_error_inval);
      CHK(nat_mult(&total, 10) == 1, "Multiplication overflow.", out_error_errno);
      CHK(nat_add(&total, num) == 1, "Addition overflow.", out_error_errno);
  }
  *result = total;
  return 0;
out_error_inval:
  errno = EINVAL;
out_error_errno:
  return 1;
}
