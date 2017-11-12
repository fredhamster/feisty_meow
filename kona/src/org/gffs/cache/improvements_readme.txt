

it would be nice to fix the role based cache node and how these classes
are intertwingled.
it seems better to have some interfaces for the different concerns, like timeout and
ordering.
and then to inherit interfaces to build the real object as a composite,
rather than stuffing all characteristics any cache node has into one object,
the role based cache node.

also the structures like cachelist are not separable from the stored type right now.

these guys DO store any kind of type that you can implement with the parts,
and they do it well, but the internal organization could be cleaned a bit and i think
it would reduce the code size and also the complexity of the objects.



