# Copyright (c) 2011 Akamai Technologies, Inc.

# Disable pre-existing implicit rules and suffix rules for easier dep debugging.
%.o : %.s
% : RCS/%,v
% : RCS/%
% : %,v
% : s.%
% : SCCS/s.%
.SUFFIXES:
SUFFIXES :=

include config.mk

# Debugging aid

pr-%:
	@echo '$*=$($*)'

# commands

gzip ?= gzip
mkdir ?= mkdir -p $@
pandoc ?= pandoc
pandoc_html ?= $(pandoc) -s -S -r markdown -w man $< | man2html -r - | tail -n +3 | sed -e '3i<link href="../man.css" rel="stylesheet" type="text/css"/>' > $@
pandoc_gz ?= $(pandoc) -s -S -r markdown -w man $< | gzip -c - > $@

# flags

WARNFLAGS = \
        -W -Wformat -Wall -Wundef -Wpointer-arith -Wcast-qual \
        -Wcast-align -Wwrite-strings -Wsign-compare \
        -Wmissing-noreturn \
        -Wextra -Wstrict-aliasing=2

DEPFLAGS = -MMD -MP
CFLAGS ?= -O3 -ggdb3
CXXFLAGS ?= -O3 -ggdb3
CPPFLAGS ?= -I.
LDFLAGS ?= -ggdb3
ALLCPPFLAGS = $(CPPFLAGS)
ALLCFLAGS = -std=gnu99 -fPIC $(WARNFLAGS) $(DEPFLAGS) $(CFLAGS)
ALLCXXFLAGS = -std=gnu++98 -fPIC $(WARNFLAGS) $(DEPFLAGS) $(CXXFLAGS)

# rules

%.o: %.c
	$(CC) $(ALLCFLAGS) $($(@)_CFLAGS) $(ALLCPPFLAGS) $($(@)_CPPFLAGS) -c -o $@ $<

%.o: %.cc system.hh.gch
	$(CXX) $(ALLCXXFLAGS) $($(@)_CXXFLAGS) $(ALLCPPFLAGS) $($(@)_CPPFLAGS) -c -o $@ $<

%.hh.gch: %.hh
	$(CXX) $(ALLCXXFLAGS) $($(@)_CXXFLAGS) $(ALLCPPFLAGS) $($(@)_CPPFLAGS) -c -o $@ $<

%.ok: % Makefile
	@{ $< && echo "$< OK"; } || { echo "$< FAIL" && head $<.log && exit 1; }
	@touch $@

.SECONDARY: system.hh.gch

# declarations

SCRIPTS = $(wildcard vscan*)

MANPAGES_GZ= $(patsubst %,man1/%.1.gz,$(SCRIPTS) $(BINARIES)) man1/vscan.1.gz man5/vscan-config.5.gz

MANPAGES_HTML= $(patsubst %.gz,%.html,$(MANPAGES))

DATA= $(wildcard data/*) config/PKGNAME config/VERSION config.lua

PROGRAMS = $(patsubst %,vscan-%,highlight report scan-dir scan-tarball summarize) $(patsubst %,t/%,b64_test config_test test)

# objects

vscan-highlight_OBJS = highlight.o log.o encode.o decode.o sensors.o config.o scanner_mode.o luaa.o
vscan-report_OBJS = report.o binder.o sensor_model.o
vscan-scan-dir_OBJS = scan_dir.o log.o encode.o decode.o sensors.o decider.o counter.o traversal.o fake_fdopendir.o fd_traversal.o stream_traversal.o config.o scanner_mode.o scanner.o regex_scanner.o tarball_scanner.o scannable.o fd_scannable.o luaa.o
vscan-scan-tarball_OBJS = scan_tarball.o log.o encode.o sensors.o config.o counter.o traversal.o tarball_traversal.o scanner_mode.o scanner.o regex_scanner.o scannable.o tarball_scannable.o luaa.o
vscan-summarize_OBJS = summarize.o encode.o sensors.o nat.o binder.o sample_model.o sensor_model.o path_model.o hits_model.o config.o scanner_mode.o luaa.o

t/b64_test_OBJS = t/b64_test.o decode.o
t/config_test_OBJS = t/config_test.o config.o scanner_mode.o luaa.o
t/test_OBJS = t/test.o decode.o

# deps

binder.o_DEPS = libc libre2 libsqlite3
config.o_DEPS = libc libre2 liblua
decider.o_DEPS = libc libre2
decode.o_DEPS = libc libre2
encode.o_DEPS = libc libre2
fake_fdopendir.o_DEPS = libc
fd_scannable.o_DEPS = libc libre2
fd_traversal.o_DEPS = libc libre2
highlight.o_DEPS = libc libre2 libsqlite3
hits_model.o_DEPS = libc libsqlite3
log.o_DEPS = libc libre2
luaa.o_DEPS = libc libre2 liblua
path_model.o_DEPS = libc libre2 libsqlite3
regex_scanner.o_DEPS = libc libre2
report.o_DEPS = libc libre2
sample_model.o_DEPS = libc libre2 libsqlite3
scannable.o_DEPS = libc libre2
scanner_mode.o_DEPS = libc libre2
scanner.o_DEPS = libc libre2
scan_dir.o_DEPS = libc libarchive libz libre2
scan_tarball.o_DEPS = libc libarchive libz libre2
sensor_model.o_DEPS = libc libre2 libsqlite3
sensors.o_DEPS = libc libre2
stream_traversal.o_DEPS = libc libre2
summarize.o_DEPS = libc libre2 libsqlite3
tarball_scannable.o_DEPS = libc libarchive libz libre2
tarball_scanner.o_DEPS = libc libarchive libz libre2
tarball_traversal.o_DEPS = libc libarchive libz libre2
traversal.o_DEPS = libc libre2

t/b64_test.o_DEPS = libc libre2
t/config_test.o_DEPS = libc libre2
t/test.o_DEPS = libc libre2

t/md5.t.ok: vscan-scan-dir t/md5.t.py t/md5.t.lua
t/collect.t.ok: vscan-scan-dir t/collect.t.py t/collect.t.lua
t/local.t.ok: vscan-scan-dir t/local.t.py t/local.t.lua
t/view.t.ok: vscan-view

vscan-view: vscan-highlight

-include private.mk

# codegen

define DEP_template
$(1)_CPPFLAGS += $(filter-out $($(1)_CPPFLAGS),$($(2)_CPPFLAGS))
$(1)_CFLAGS += $(filter-out $($(1)_CFLAGS),$($(2)_CFLAGS))
$(1)_CXXFLAGS += $(filter-out $($(1)_CXXFLAGS),$($(2)_CXXFLAGS))
$(1)_LDFLAGS += $(filter-out $($(1)_LDFLAGS),$($(2)_LDFLAGS))
endef

define OBJ_template
$(2)_DEPS += $(filter-out $($(2)_DEPS),$($(1)_DEPS))
$(foreach dep,$($(1)_DEPS),$(eval $(call DEP_template,$(1),$(dep))))
endef

define PROGRAM_template
$(foreach obj,$($(1)_OBJS),$(eval $(call OBJ_template,$(obj),$(1))))
$(foreach dep,$($(1)_DEPS),$(eval $(call DEP_template,$(1),$(dep))))
$(1): $$($(1)_OBJS)
	$(CXX) -o $$@ $$^ $(LDFLAGS) $$($(1)_LDFLAGS)
ALL_OBJS += $$($(1)_OBJS)
ifeq ($(shell sh -c "echo '$(1)' | egrep '^t/'"),)
BINARIES += $(1)
endif
CLEAN_BINARIES += $(1)
endef

$(foreach prog,$(PROGRAMS),$(eval $(call PROGRAM_template,$(prog))))

define MAN_template
man$(1)/%.$(1).gz: docs/%.txt
	mkdir -p $$(@D) && $$(pandoc_gz)
man$(1)/%.$(1).html: docs/%.txt
	mkdir -p $$(@D) && $$(pandoc_html) || true
endef
$(foreach N,1 2 3 4 5 6 7 8,$(eval $(call MAN_template,$(N))))

# targets

all: $(BINARIES) $(SCRIPTS) docs

docs: $(MANPAGES_GZ) $(MANPAGES_HTML)

clean:
	rm -f $(CLEAN_BINARIES) $(MANPAGES_GZ) $(MANPAGES_HTML) *.d *.o t/*.d t/*.o system.hh.gch
	rm -rf $(patsubst %,man%,1 2 3 4 5 6 7 8)
	find t -name '*.ok' -delete
	find t -name 'examples.tar.gz' -delete

check: all $(patsubst %,%.ok,$(shell find t -name '*.t'))

lint:
	python cpplint.py --filter=-whitespace,-build/include,-build/header_guard,-readability/multiline_comment *.c *.cc *.h

install: all
	install -d -m 0755 "$(bindir)"
	for f in $(BINARIES) $(SCRIPTS); do \
		install -m 0755 $$f "$(bindir)/$$(basename $$f)"; \
	done
	for p in $(MANPAGES_GZ); do \
		install -d -m 0755 "$(mandir)/$$(dirname $$p)"; \
		install -m 0644 $$p "$(mandir)/$$(dirname $$p)/$$(basename $$p)" ; \
	done
	for p in $(DATA); do \
		install -d "$(sharedir)/vscan/$$(dirname $$p)"; \
		install -m 0644 $$p "$(sharedir)/vscan/$$(dirname $$p)" ; \
	done

.PHONY: clean install all check lint docs
.DEFAULT_GOAL := all

-include $(ALL_OBJS:%.o=%.d)

# vim: noet sts=4 ts=4 sw=4 :
