/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

#include "cgen.h"
#include "encode.h"
#include "decode.h"
#include "sensors.h"
#include "log.h"
#include "scanner_mode.h"
#include "config.h"

int main(int argc, char** argv)
{
	Config config;
	int ret = 0;
	StringPiece capture, bufp;
	bool did_match;
	string pat_all_str;
	RE2* pat_all = NULL;

	CHK(config.Init("./config.lua") != 0,
		"unable to load config", out_error);

	/* 0. Generate a comprehensive pattern by OR-ing together a
	 * non-capturing group for each individual sensor pattern inside a
	 * single top-level capturing group. */
	CHK(combine_sensors(config.sensors.begin(),
			    config.sensors.end(),
			    &pat_all_str) != 0,
		"unable to combine sensors", out_error);

	pat_all = new RE2(pat_all_str);

	for (int argi = 1; argi < argc; argi++)
	{
		/* 1. Prepare to read the file named in argv[i]. */
		int fd;
		struct stat st;
		void* data;
		off_t data_len;

		StringPiece qpath = StringPiece(argv[argi]);

		ostringstream oss_decoded_path;
		ret = uri_decode(qpath, oss_decoded_path);
		if (ret)
		{
			cout << "qpath: " << qpath << "\n";
			cout << "unable to uri_decode; skipping with ret " << ret << "\n";
			continue; // XXX: Blow up here?
		}

		string path = oss_decoded_path.str();

		LET(fd = open(path.c_str(), O_RDONLY), fd == -1,
			"unable to open path", out_error);
		CHK(fstat(fd, &st) == -1,
			"unable to fstat fd", out_error);
		data_len = st.st_size;

		LET(data = mmap(NULL, data_len, PROT_READ, MAP_SHARED, fd, 0),
			data == reinterpret_cast<void*>(-1),
			"unable to mmap data", out_error);

		StringPiece bufp((const char*)data, st.st_size);

		did_match = false;

		for (sensors_iterator sens = config.sensors.begin();
			sens != config.sensors.end();
			++sens)
			sens->count = 0;

		/* 2.2. For all pat_all matches, reconstruct which sensor fired
		 * by linear search. When you figure it out, write an F line. */
		while (RE2::FindAndConsume(&bufp, *pat_all, &capture))
		{
			if (did_match == false)
				did_match = true;

			for (sensors_iterator sens = config.sensors.begin();
				sens != config.sensors.end();
				++sens)
			{
				if (RE2::PartialMatch(capture, *sens->pat))
				{
					write_f_line(&cout, qpath, &*sens, capture);
					sens->count++;
				}
			}
		}
		/* 2.3. If you had any matches, write a summary P line and maybe
		 * an H line with highlighted content. */
		if (did_match)
		{
			char* cdata = reinterpret_cast<char*>(data);
			write_p_line(&config.sensors, &cout, qpath);
			write_h_line(&cout,
				qpath,
				StringPiece(cdata, data_len),
				pat_all);
		}

		CHK(munmap(data, data_len) == -1,
			"unable to munmap file", out_error);
		CHK(close(fd) == -1,
			"unable to close fd", out_error);
	}
	return 0;
out_error:
	return 1;
}
