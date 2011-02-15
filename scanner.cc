/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

#include "cgen.h"
#include "sensors.h"
#include "log.h"
#include "encode.h"
#include "decode.h"
#include "decider.h"
#include "traversal.h"
#include "fd_traversal.h"
#include "counter.h"
#include "scanner.h"
#include "scannable.h"

extern const int WINDOW_LEN = 10000;


Scanner::Scanner() : destroyed_(false), counter_(NULL) {}

Scanner::~Scanner()
{
	if (!destroyed_)
	{
		PERROR("destructing live scanner");
	}
}

int Scanner::Init(Counter* counter)
{
	CHK(counter == NULL, "null counter", out_error);
	counter_ = counter;
	return 0;
out_error:
	return 1;
}

int Scanner::Destroy()
{
	destroyed_ = true;
	return 0;
}

int Scanner::scan_pre(Scannable* sc)
{
  (void)sc;
	return 0;
}

int Scanner::scan_post(Scannable* sc)
{
  (void)sc;
	counter_->num_interesting += 1;
	return 0;
}

int Scanner::scan(Scannable* sc)
{
	CHK(scan_pre(sc) != 0, "unable to pre-scan", out_error);

	if (sc->data_len() <= 2 * WINDOW_LEN)
	{
		CHK(sc->Scan(this, 0, sc->data_len(), NULL) != 0,
			"unable to scan total range", out_error);
	}
	else
	{
		CHK(sc->Scan(this, 0, WINDOW_LEN, NULL) != 0,
			"unable to scan start range", out_error);

		CHK(sc->Scan(this,
			sc->data_len() - WINDOW_LEN,
			WINDOW_LEN,
			NULL) != 0,
			"unable to scan end range", out_error);
	}

	CHK(scan_post(sc) != 0, "unable to post-scan", out_error);

	return 0;
out_error:
	return 1;
}
