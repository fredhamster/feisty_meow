include variables.def

# had to move to this style for these two
# since having a makefile call another was doing
# really unpredictable things, where the makefile
# passed to the -f of make was actually dragging
# in bits from the main makefile.  this was
# tested even by saying:
#   make -f makefile.stub clean
# which totally isolates it to the stub file, but
# the main makefile was still being pulled in.
# hopefully this approach of having a neutral outer
# maker will get rid of that broken behavior.

LAST_TARGETS = maker
OTHER_CLEANS = breaker

include rules.def

maker:
	$(MAKE) -f makefile.bundle_creator
	$(MAKE) -f makefile.unpacker_stub

breaker:
	$(MAKE) CLEAN=t -f makefile.bundle_creator clean
	$(MAKE) CLEAN=t -f makefile.unpacker_stub clean


