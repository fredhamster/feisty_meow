
# define an array.
@fred = ('x', 'y', 'z');

# define a hash.
%fred = (x => 'farfle', q => 'nuggy', r => 'bunko');

# show the array.
print "\@fred is: @fred\n";
# show the first element of the array.
print "\$fred[0] is $fred[0]\n";

# show the details of the hash.
@fredkeys = keys(%fred);
print "\%fred keys are: @fredkeys\n";
@fredvals = values(%fred);
print "\%fred values are: @fredvals\n";
# show the value for the first key we defined in the hash (although that's incidental;
# we don't expect to access things in the hash by their order of addition or even by
# numerical indexes at all.
print "\$fred['x'] is $fred{'x'}\n";
