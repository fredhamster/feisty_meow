
// huffware script: jaunter button pusher, by fred huffhines
//
// the button pusher script warped into service for jaunters.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

string JAUNT_NEXT_BUTTON_NAME = "next";
string JAUNT_MENU_BUTTON_NAME = "menu";

// the button pushing API.
//////////////
integer BUTTON_PUSHER_HUFFWARE_ID = 10029;
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

float MENU_TIMER_FOR_TOUCH = 0.8;  // click hold delay before we decide they want the menu.

key last_toucher;  // the last person who touched the jaunter.

integer sent_menu_event = FALSE;  // true when we just sent out a menu event.

//////////////

handle_timer()
{
    if (last_toucher != NULL_KEY) {
//log_it("decided to send menu event!");
        // this is a timer elapsing for a touch-hold menu event.
        send_menu_button_event();
        last_toucher = NULL_KEY;
        llSetTimerEvent(0.0);
        sent_menu_event = TRUE;
        return;
    }
}

// generates our next button alert, once the user has finished clicking on the button.
send_next_button_event()
{
    llMessageLinked(LINK_SET, BUTTON_PUSHER_HUFFWARE_ID + REPLY_DISTANCE,
        BUTTON_PUSHED_ALERT, JAUNT_NEXT_BUTTON_NAME);
}

// tells the jaunter to show the menu of places.
send_menu_button_event()
{
    llMessageLinked(LINK_SET, BUTTON_PUSHER_HUFFWARE_ID + REPLY_DISTANCE,
        BUTTON_PUSHED_ALERT, wrap_parameters([JAUNT_MENU_BUTTON_NAME, last_toucher]));
}


//////////////
// from hufflets...

integer debug_num = 0;

// a debugging output method.  can be disabled entirely in one place.
log_it(string to_say)
{
    debug_num++;
    // tell this to the owner.    
    llOwnerSay(llGetScriptName() + "[" + (string)debug_num + "] " + to_say);
    // say this on open chat, but use an unusual channel.
//    llSay(108, (string)debug_num + "- " + to_say);
}

string wrap_parameters(list to_flatten)
{ return llDumpList2String(to_flatten, HUFFWARE_PARM_SEPARATOR); }

///////////////

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

    touch_start(integer total_number) {
        if (llDetectedLinkNumber(0) != llGetLinkNumber()) return;
//log_it("touch start");
        llSetTimerEvent(MENU_TIMER_FOR_TOUCH);
            // how long before "hold touch" kicks in and we show the destination menu.
        last_toucher = llDetectedKey(0);
    }

    timer() { handle_timer(); }

    touch_end(integer total_number)
    {
        if (llDetectedLinkNumber(0) != llGetLinkNumber()) return;
//log_it("touch end");
        if (sent_menu_event) {
            // don't send a click event if we popped the menu.            
            sent_menu_event = FALSE;
            return;
        }
        // clear timer since we got to here.
        llSetTimerEvent(0.0);
        last_toucher = NULL_KEY;
        send_next_button_event();
    }
}
