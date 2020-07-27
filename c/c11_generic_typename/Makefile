CVERSIONS := gnu99 gnu11

test_%_preprocessed.c: c11_generic_typename.c
	gcc -E -Wall -Wextra -DNO_WARN_MISSING_GENERIC=1 -std=$* > $@ $<

test_%: test_%_preprocessed.c
	gcc -Wall -Wextra -DNO_WARN_MISSING_GENERIC=1 -std=$* -o $@ $<

define runtest_ver
runtest_$(CVERSION): test_$(CVERSION)
	./$$< || echo "Exited with $$$$?"
.PHONY: runtest_$(CVERSION)
.PRECIOUS: test_$(CVERSION) test_$(CVERSION)_preprocessed.c
endef

$(foreach CVERSION,$(CVERSIONS),$(eval $(runtest_ver)))

clean:
	rm -f $(foreach CVERSION,$(CVERSIONS),test_$(CVERSION) test_$(CVERSION)_preprocessed.c)

runall: $(foreach CVERSION,$(CVERSIONS),runtest_$(CVERSION))
.DEFAULT_GOAL := runall
