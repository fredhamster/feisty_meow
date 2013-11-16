
# this sql statement fixes ownership for items that may have come in with
# erroneous owner UUIDs due to second inventory and the vagaries of crossing
# between grids.
# this particular example is pretty brutal.  it finds any item that is owned
# by the old avatar ID {old_ID} and swaps out the owner with the {new_ID}
# avatar.  This task is made more difficult by the lack of a wildcard in
# the SQL replace function, so we only stomp out one bogus ID at a time.

UPDATE inventoryitems
SET creatorid = replace(creatorid, '{old_ID}', '{new_ID}')
WHERE creatorid == '{old_ID}';


