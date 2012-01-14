
# opensim select statement to find items in the prims table (items that still
# exist in some sim or at least exist in the database) that are owned by
# a particular avatar.  the {uuid} below should be replaced with the UUID
# that you care about showing prim ownership for.

select * from prims where creatorid = '{uuid}'

# this is a very similar select that shows the items which are NOT owned by
# the avatar in question.

select * from prims where creatorid != '{uuid}'

