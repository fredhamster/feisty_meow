
// huffware script: button pusher, by fred huffhines
//
// a simple API for reporting when buttons are pushed.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

// this is a kludgy thing, but this must be hard-coded to have the right button
// name for what the rest of the device is expecting.  it should vary depending on
// the actual button prim that this script is placed in.
string BUTTON_NAME = "next";

// the button pushing API.
//////////////
integer BUTTON_PUSHER_HUFFWARE_ID = 10035;
    // a unique ID within the huffware system for this script.
string HUFFWARE_PARM_SEPARATOR = "{~~~}";
    // this pattern is an uncommon thing to see in text, so we use it to separate
    // our commands in link messages.
string HUFFWARE_ITEM_SEPARATOR = "{|||}";
    // used to separate lists of items from each other when stored inside a parameter.
    // this allows lists to be passed as single string parameters if needed.
integer REPLY_DISTANCE = 100008;  // offset added to service's huffware id in reply IDs.
//////////////
string BUTTON_PUSHED_ALERT = "#btnp";
    // this event is generated when the button is pushed.  the number parameter will be
    // the huffware id plus the reply distance.  the id parameter in the link message will
    // contain the name of the button that was pushed.
//////////////

// generates our button pressed alert, when the user has finished clicking on the button.
send_button_event()
{
//llOwnerSay("user clicked on button " + BUTTON_NAME);
    llMessageLinked(LINK_SET, BUTTON_PUSHER_HUFFWARE_ID + REPLY_DISTANCE,
        BUTTON_PUSHED_ALERT, BUTTON_NAME);
}

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
    
    touch_end(integer count) {
        if (llDetectedLinkNumber(0) == llGetLinkNumber())
            send_button_event();
    }
}
