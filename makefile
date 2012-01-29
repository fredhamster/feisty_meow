include variables.def

PROJECT = feisty_meow_sources
BUILD_BEFORE = start_make nucleus scripts octopi graphiq production end_make

include rules.def

start_make: show_date.start

end_make: show_date.end


