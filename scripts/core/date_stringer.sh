#!/bin/bash

# a handy little method that can be used for date strings.  it was getting
# really tiresome how many different ways the script did the date formatting.

function date_stringer()
{
  date +"%Y_%m_%d_%a_%H%M_%S" | tr -d '/\n/'
}

