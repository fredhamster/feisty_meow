CONSOLE_MODE = t

include cpp/variables.def

PROJECT = test_cromp
TYPE = test
TARGETS = test_cromp_client.exe test_cromp_server.exe test_many_cromp.exe
LAST_TARGETS = create_decoder_ring
LOCAL_LIBS_USED = unit_test application configuration filesystem loggers \
  mathematics nodes octopi processes sockets structures textual timely \
  structures basis 
USE_SSL = t
#RUN_TARGETS = $(ACTUAL_TARGETS)

include cpp/rules.def

create_decoder_ring:
	$(MAKE) -f makefile.decoder

