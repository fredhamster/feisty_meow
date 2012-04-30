
// testing of an opensim bug, where there was a new requirement that
// the object have a sit target before any changed events will be fired.
//
//   this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
//   do not use it in objects without fully realizing you are implicitly accepting that license.
//

integer link_changes = 0;


default {
    state_entry() { if (llSubStringIndex(llGetObjectName(), "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default {
    state_entry()
    {
        llSitTarget(<0, 0, 0.1>, ZERO_ROTATION);
//above line makes things work.  comment it out, and put in new object, and
// the object will not get changed events.
        llSay(0, "sit to run the test...");
    }
    
    on_rez(integer count) { llResetScript(); }
    
    changed(integer chang) {
        llSay(0, "got into changed event...");
        if (! (chang & CHANGED_LINK) ) {
            llSay(0, "change was not a link, leaving.");
            return;  // not for us.
        }
        llSay(0, "into changed event, CHANGED_LINK...");
        link_changes++;
        key av_sitting = llAvatarOnSitTarget();
        if (av_sitting == NULL_KEY) {
            llSay(0, "avatar stood up since key is null");
        } else {
            llSay(0, "avatar sat down: " + llDetectedName(0));
        }
    }
    
    touch_start(integer count) {
        llSay(0, "there have been " + (string)link_changes
            + " 'changed' events for links since the last reset.");
    }
}