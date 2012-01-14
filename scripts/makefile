include variables.def

PROJECT = Shells
TARGETS += regenerate 
OTHER_CLEANS += clean_generated 
# override setting from clam.
export SHELLDIR = $(YETI_DIR)/scripts

include rules.def

regenerate:
	@echo regenerating aliases.
	echo "SHELLDIR = $(SHELLDIR)"
	echo "GENERADIR = $(GENERADIR)"
	$(CATCHER)'if [ ! -d $(GENERADIR) ]; then mkdir $(GENERADIR); fi'
	$(CATCHER)perl $(SHELLDIR)/core/generate_aliases.pl
	$(CATCHER)'(export LIGHTWEIGHT_INIT=true; source $(SHELLDIR)/core/profile.sh; perl $(SHELLDIR)/text/new_sig.pl)'

clean_generated:
	@echo removing generated aliases.
	$(CATCHER)'if [ -d $(GENERADIR) ]; then \rm -rf $(GENERADIR); fi'

