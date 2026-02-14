CONSOLE_MODE = t

include cpp/variables.def

PROJECT = test_cromp
TYPE = test
TARGETS = test_cromp_client.exe test_cromp_server.exe test_many_cromp.exe
#DEFINITIONS += USE_HOOPLE_DLLS
LAST_TARGETS = create_decoder_ring
#LOCAL_LIBS_USED = basis i_library i_communication
USE_SSL = t
#VCPP_USE_SOCK = t
RUN_TARGETS = $(ACTUAL_TARGETS)

include cpp/rules.def

create_decoder_ring:
	$(MAKE) -f makefile.decoder

