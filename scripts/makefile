include variables.def

PROJECT = Shells
TARGETS += regenerate 
# override setting from clam.
#hmmm: why is that needed?
export FEISTY_MEOW_SCRIPTS = $(FEISTY_MEOW_APEX)/scripts

include rules.def

regenerate:
	@echo regenerating aliases.
	echo "FEISTY_MEOW_SCRIPTS = $(FEISTY_MEOW_SCRIPTS)"
	echo "FEISTY_MEOW_LOADING_DOCK = $(FEISTY_MEOW_LOADING_DOCK)"
	$(CATCHER)'if [ ! -d $(FEISTY_MEOW_LOADING_DOCK) ]; then mkdir -p $(FEISTY_MEOW_LOADING_DOCK); fi'
	$(CATCHER)perl $(FEISTY_MEOW_SCRIPTS)/core/generate_aliases.pl
	$(CATCHER)'(source "$(FEISTY_MEOW_SCRIPTS)/core/launch_feisty_meow.sh"; perl $(FEISTY_MEOW_SCRIPTS)/text/new_sig.pl)'


