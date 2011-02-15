/* Copyright (c) 2011 Akamai Technologies, Inc. */

struct sensor {
	string name;
	RE2* pat;
	size_t count;
};

typedef list<struct sensor>::iterator sensors_iterator;
typedef list<struct sensor>::const_iterator const_sensors_iterator;

int combine_sensors(const_sensors_iterator begin,
  const_sensors_iterator end,
  string* out);
