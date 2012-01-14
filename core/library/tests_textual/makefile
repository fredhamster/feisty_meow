include cpp/variables.def

PROJECT = tests_textual
TYPE = test
LAST_TARGETS = copy_datafile
TARGETS = test_byte_format.exe test_parse_csv.exe test_splitter.exe test_xml_generator.exe
LOCAL_LIBS_USED = unit_test application loggers configuration textual timely filesystem \
  structures basis 
RUN_TARGETS = $(ACTUAL_TARGETS)

include cpp/rules.def

copy_datafile:
	$(HIDER)cp -f df_1.csv $(EXECUTABLE_DIR)

