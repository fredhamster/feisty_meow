
// 
// used to control the door into the inner chamber of the old garleon
// land, where we had a basement with a crypt door.  this was in the rocks
// near the door.  it actually will work with any tl linked door script set
// to our 108 channel, but it currently shouts, which will fling all the nearby
// doors open.

default
{
    state_entry()
    {
    }

    touch_start(integer total_number)
    {
        if (llSameGroup(llDetectedKey(0))) llShout(108, "toggle");
    }
}
