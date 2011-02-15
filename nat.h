/* Copyright (c) 2011 Akamai Technologies, Inc. */

int nat_add(unsigned long* result, unsigned long arg);
int nat_mult(unsigned long* result, unsigned long long arg);
int parse_nat(unsigned long* result, const char* buf, size_t buflen);
