/* Copyright (c) 2011 Akamai Technologies, Inc. */

#include "system.hh"

#include "cgen.h"
#include "encode.h"
#include "sensors.h"
#include "counter.h"

const RE2 pat_highlight_start("`");
const RE2 pat_highlight_end("~");

void write_f_line(ostream* out,
	const StringPiece& qpath,
	const sensor* sens,
	const StringPiece& capture)
{
	StringPiece snippet = capture.substr(0, min(120, capture.size()));
	*out << "F "
	     << qpath << " "
	     << sens->name << " "
	     << uri_encode(snippet) << "\n";
}

void write_p_line(const list<struct sensor>* sensors,
	ostream* out,
	const StringPiece& qpath)
{
	map<string, size_t> matches = map<string, size_t> ();
	for (const_sensors_iterator sens = sensors->begin(), sens_end = sensors->end();
		sens != sens_end;
		++sens)
	{
		if (sens->count > 0)
			matches.insert(pair<string, size_t>(sens->name, sens->count));
	}

	*out << "P " << qpath;
	for (map<string, size_t>::const_iterator it = matches.begin(), ie = matches.end();
		it != ie;
		++it)
		*out << " " << it->first; // << " " << it->second;
	*out << "\n";
}

void write_h_line(ostream* out,
	const StringPiece& qpath,
	const StringPiece& text,
	const RE2* pat_all)
{
	/* 1. Pick two printable "control" characters to signal "start
	   highlighting" and "stop highlighting". (We chose ` and ~.) */
	ostringstream oss1, oss2;
	oss1 << highlight_encode(text);
	string hl = oss1.str();

	/* 2. Quote any pre-existing highlighting characters in your input text. */
	RE2::GlobalReplace(&hl, *pat_all, "`\\1~");

	/* 3. Replace all matches against pat_all with `\1~. */
	oss2 << html_encode(hl);
	string hl2 = oss2.str();

	/* 4. Replace all control chars with appropriate HTML tags. */
	RE2::GlobalReplace(&hl2, pat_highlight_start, "<b><span class=\"hit\">");
	RE2::GlobalReplace(&hl2, pat_highlight_end, "</span></b>");

	*out << "H " << qpath << " " << hl2 << "\n";
}

void write_c_line(ostream* out,
	const StringPiece& qpath,
	const Counter* counter)
{
	*out << "C " << qpath
		<< " " << counter->num_dirs
		<< " " << counter->num_files
		<< " " << counter->num_interesting
		<< "\n";
}
