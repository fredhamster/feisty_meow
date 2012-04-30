
// Megaprim! (auxiliary script if your viewer doesn't support custom size megaprims yet)
// Fill in x, y, z with the values you need for your prim. They should be less or equal than 64.

///////
// defaults are not very mega-primish, and instead are nice for our huffotronic updaters.
// --fred huffhines.
///////

float   x = .42;
float   y = .42;
float   z = .6;

/*

original author's note:

Even if your viewer does not support yet the ability to create the new megas that are now available in the RC servers (the regions running the software for mesh, which you can check in Help: About in your viewer), a simple script, attached to this notice, will help you.

Edit the script, change x, y, z with the values you want for the size, *up to 64 meters*, save it, and drop it into your prim. The script will do the work for you. These new megas will work in ANY sim when your rez them. Spread the love!

Auryn Beorn

*/

default
{
    state_entry()
    {
        llSetPrimitiveParams([PRIM_SIZE, <x, y, z>]);
//nooo...  we don't always want this.        llRemoveInventory(llGetScriptName());
    }
}
