include cpp/variables.def

PROJECT = tests_structures
TYPE = test
SOURCE = bogon.cpp
TARGETS = test_amorph.exe test_hash_table.exe test_int_hash.exe test_matrix.exe \
  test_memory_limiter.exe test_packing.exe test_stack.exe test_unique_id.exe \
  test_bit_vector.exe test_set.exe test_string_table.exe test_symbol_table.exe \
  test_version.exe
LOCAL_LIBS_USED = unit_test application loggers configuration textual timely filesystem \
  structures basis 
RUN_TARGETS = $(ACTUAL_TARGETS)

include cpp/rules.def
