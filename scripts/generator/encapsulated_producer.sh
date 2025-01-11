##############
#  Name   : encapsulated producer (for feisty meow building)
#  Author : Chris Koeritz
#  Purpose:
#    Runs the produce_feisty_meow script, assuming that the FEISTY_MEOW_APEX
#  variable has been set.  This is a useful encapsulated script that can be
#  used with the host_strider.
##############
# Copyright (c) 2024-$now By Author.  This program is free software; you can
# redistribute it and/or modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation; either version 2 of
# the License or (at your option) any later version.  This is online at:
#     http://www.fsf.org/copyleft/gpl.html
# Please send any updates to: fred@gruntose.com
##############

feisty_top=$FEISTY_MEOW_APEX

if [ -z "$feisty_top" ]; then
  echo "
The FEISTY_MEOW_APEX variable has not been set.  This indicates that the
Feisty Meow script environment does not exist on this host (which is named
'\$(hostname)').
"
  exit 1
fi

bash $feisty_top/scripts/generator/produce_feisty_meow.sh


