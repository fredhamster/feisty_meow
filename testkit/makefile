include variables.def

PROJECT = testkit
FIRST_TARGETS += run_testkit_check

include rules.def

run_testkit_check:
	$(HIDESH) -c 'bash test_driver.sh'
