/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

#include "cgen.h"
#include "sensors.h"
#include "log.h"
#include "encode.h"
#include "decode.h"
#include "scanner.h"
#include "regex_scanner.h"
#include "scannable.h"

RegexScanner::RegexScanner()
	:
	sensors_(NULL),
	pat_all_(NULL)
	{}

RegexScanner::~RegexScanner() {}

int RegexScanner::Init(Counter* counter,
	list<struct sensor>* sensors,
	const RE2 * pat_all)
{
	CHK(Scanner::Init(counter) != 0,
		"unable to init scanner", out_error)
	CHK(sensors == NULL, "null sensors", out_error);
	CHK(pat_all == NULL, "null pat_all", out_error);

	sensors_ = sensors;
	pat_all_ = pat_all;
	return 0;
out_error:
	return 1;
}

int RegexScanner::scan_buf(Scannable* sc, StringPiece buf, void*)
{
	bool did_match = false;

	for (sensors_iterator sens = sensors_->begin(), sens_end = sensors_->end();
		sens != sens_end;
		++sens)
	{
		sens->count = 0;
	}

	StringPiece capture;
	string qpath;

	/* For all pat_all matches, reconstruct which sensor fired
	 * by linear search. When you figure it out, write an F line. */
	while (RE2::FindAndConsume(&buf, *pat_all_, &capture))
	{
		if (did_match == false)
		{
			CHK(sc->get_quoted_path(&qpath) != 0,
				"unable to get current qpath", out_error);
			did_match = true;
		}

		for (sensors_iterator sens = sensors_->begin(), sens_end = sensors_->end();
			sens != sens_end;
			++sens)
		{
			if (RE2::PartialMatch(capture, *sens->pat))
			{
				write_f_line(&cout, qpath, &(*sens), capture);
				sens->count++;
			}
		}
	}

	if (did_match)
		write_p_line(sensors_, &cout, qpath);

	return 0;
out_error:
	return 1;
}
