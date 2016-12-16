include variables.def

PROJECT = feisty_meow_sources
FIRST_TARGETS += start_make
BUILD_BEFORE = nucleus scripts octopi graphiq hypermedia kona production 
LAST_TARGETS += end_make

include rules.def

start_make: show_date.start

end_make: show_date.end

