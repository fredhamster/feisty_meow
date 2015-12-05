include variables.def

PROJECT = Shells
TARGETS += regenerate 
OTHER_CLEANS += clean_generated 
# override setting from clam.
export FEISTY_MEOW_SCRIPTS = $(FEISTY_MEOW_DIR)/scripts

include rules.def

regenerate:
	@echo regenerating aliases.
	echo "FEISTY_MEOW_SCRIPTS = $(FEISTY_MEOW_SCRIPTS)"
	echo "FEISTY_MEOW_LOADING_DOCK = $(FEISTY_MEOW_LOADING_DOCK)"
	$(CATCHER)'if [ ! -d $(FEISTY_MEOW_LOADING_DOCK) ]; then mkdir -p $(FEISTY_MEOW_LOADING_DOCK); fi'
	$(CATCHER)perl $(FEISTY_MEOW_SCRIPTS)/core/generate_aliases.pl
	$(CATCHER)'(export LIGHTWEIGHT_INIT=true; source $(FEISTY_MEOW_SCRIPTS)/core/launch_feisty_meow.sh; perl $(FEISTY_MEOW_SCRIPTS)/text/new_sig.pl)'

clean_generated:
	@echo removing generated aliases.
	$(CATCHER)'if [ -d $(FEISTY_MEOW_LOADING_DOCK) ]; then \rm -rf $(FEISTY_MEOW_LOADING_DOCK); fi'

