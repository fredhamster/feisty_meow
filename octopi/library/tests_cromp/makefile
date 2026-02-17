CONSOLE_MODE = t

include cpp/variables.def

PROJECT = test_cromp
TYPE = test
TARGETS = test_cromp_client.exe test_cromp_server.exe test_many_cromp.exe
LOCAL_LIBS_USED = unit_test crypto application configuration filesystem loggers \
  mathematics nodes processes sockets structures textual timely cromp octopus \
  tentacles structures basis 
USE_SSL = t
LAST_TARGETS = create_decoder_ring run_client_server_test

include cpp/rules.def

create_decoder_ring:
	$(MAKE) -f makefile.decoder

run_client_server_test:
	# launch the server into the background first.  we have to hope it completes, eventually.
	($(FEISTY_MEOW_BINARIES)/test_cromp_server &)
	$(FEISTY_MEOW_BINARIES)/test_cromp_client --sends 1008
	# zap the server now, since it won't quit otherwise.
	killall test_cromp_server
