
// huffware script: simple map tp, by fred huffhines.
//
// an inglorious little script for jumping somewhere else.

string region = "hippocampus";

default {
    state_entry() { if (llSubStringIndex(llGetObjectName(), "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default
{
    state_entry()
    {
    }
    
    touch_start(integer count)
    {
        llWhisper(0, "Showing map of " + region + " for teleport...");
        llMapDestination(region, <130, 130, 24>, ZERO_VECTOR);
    }
}

