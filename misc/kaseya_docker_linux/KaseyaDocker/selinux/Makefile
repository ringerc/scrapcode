POLICY=docker-cgroupv2-systemd

%.mod: %.te
	checkmodule -M -m -o $@ $<

%.pp: %.mod
	semodule_package -o $@ -m $<

compile: $(POLICY).pp
	@echo "use 'install' to activate the policy'"

install: $(POLICY).pp
	semodule -i $(POLICY).pp

clean: $(POLICY).pp

manpages:
	mkdir man
	sepolicy manpage -p ./man -d container_init_t

.PHONY: install clean manpages
