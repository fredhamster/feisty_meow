include cpp/variables.def

PROJECT = tests_crypto
TYPE = test
TARGETS = test_blowfish_crypto.exe  test_old_school_rsa_crypto.exe test_twofish_crypto.exe 
# test_blowfish_crypto.exe  --  currently blows up, which we think is an error in openssl, because the twofish crypto works perfectly using the same
#   cryptical envelopment base.  this is annoying, but we could stop using the legacy provider entirely if we didn't do blowfish any more.  not yet.
LOCAL_LIBS_USED = unit_test crypto application processes loggers configuration textual timely \
  filesystem structures basis 
USE_SSL = t
RUN_TARGETS = $(ACTUAL_TARGETS)

include cpp/rules.def

