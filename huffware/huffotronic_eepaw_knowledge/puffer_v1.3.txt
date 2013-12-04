
// huffware script: puffer, by fred huffhines
//
// causes the prim that this script is located in to enlarge periodically.  this is
// mostly concerned with physical objects that are autonomous and can get stuck in
// prims as they move around.  thus the script also tracks the current position and
// ensures that it keeps changing.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

// constants...

vector NORMAL_SIZE = <1.0, 1.0, 0.04>;  // normal size when we're not puffing.
vector PUFFED_SIZE = <1.0, 1.0, 1.0>;  // size that we blow out to when puffing up.

float PUFFING_PERIOD = 8.4;  // number of seconds between puff-ups.
float STAY_PUFFED_TIME = 1.4;  // number of seconds to remain puffed up.

float PANIC_HOP_DISTANCE = 1.0;  // meters to jump upwards if feeling stuck.
float MINIMAL_DISTANCE_OF_MOTION = 0.1;  // must move at least this much to be considered unstuck.

// variables...

vector last_position;  // tracks where the object is hanging out.

// sets the object size to a new value.  to do this, we need to turn off physics
// temporarily.
change_size(vector new_size)
{
    llSetStatus(STATUS_PHYSICS, FALSE);
    llSetPrimitiveParams([PRIM_SIZE, new_size]);
    llSetStatus(STATUS_PHYSICS, TRUE);
}

// jumps upwards by a bit to try to escape fat prims we might get stuck inside of.
hop_up()
{
    llSetStatus(STATUS_PHYSICS, FALSE);
    llSetPos(<last_position.x, last_position.y, last_position.z + PANIC_HOP_DISTANCE>);
    llSetStatus(STATUS_PHYSICS, TRUE);
}

// this is our whole process for puffing out here.  we will make sure to
// change size briefly for the puffing.  we also track the last place we
// were at when puffing and if it's too close to our current position,
// then we'll jump up a bit.
emulate_puffer_fish()
{
    change_size(PUFFED_SIZE);
    list siz = llGetPrimitiveParams([PRIM_SIZE]);
//llOwnerSay("size puffed=" + llList2String(siz, 0));
    llSleep(STAY_PUFFED_TIME);  // snooze a bit.
    change_size(NORMAL_SIZE);
    siz = llGetPrimitiveParams([PRIM_SIZE]);
//llOwnerSay("size normal=" + llList2String(siz, 0));
    if (llVecDist(last_position, llGetPos()) < MINIMAL_DISTANCE_OF_MOTION) {
        // we seem to be stuck, so hop up a bit and see if that helps.
        hop_up();
    }
    // reset our last position to where we are now.
    last_position = llGetPos();
}

//////////////
// from hufflets...

//////////////
// huffware script: auto-retire, by fred huffhines, version 2.8.
// distributed under BSD-like license.
//   !!  keep in mind that this code must be *copied* into another
//   !!  script that you wish to add auto-retirement capability to.
// when a script has auto_retire in it, it can be dropped into an
// object and the most recent version of the script will destroy
// all older versions.
//
// the version numbers are embedded into the script names themselves.
// the notation for versions uses a letter 'v', followed by two numbers
// in the form "major.minor".
// major and minor versions are implicitly considered as a floating point
// number that increases with each newer version of the script.  thus,
// "hazmap v0.1" might be the first script in the "hazmap" script continuum,
// and "hazmap v3.2" is a more recent version.
//
// example usage of the auto-retirement script:
//     default {
//         state_entry() {
//            auto_retire();  // make sure newest addition is only version of script.
//        }
//     }
// this script is partly based on the self-upgrading scripts from markov brodsky
// and jippen faddoul.
//////////////
auto_retire() {
    string self = llGetScriptName();  // the name of this script.
    list split = compute_basename_and_version(self);
    if (llGetListLength(split) != 2) return;  // nothing to do for this script.
    string basename = llList2String(split, 0);  // script name with no version attached.
    string version_string = llList2String(split, 1);  // the version found.
    integer posn;
    // find any scripts that match the basename.  they are variants of this script.
    for (posn = llGetInventoryNumber(INVENTORY_SCRIPT) - 1; posn >= 0; posn--) {
        string curr_script = llGetInventoryName(INVENTORY_SCRIPT, posn);
        if ( (curr_script != self) && (llSubStringIndex(curr_script, basename) == 0) ) {
            // found a basic match at least.
            list inv_split = compute_basename_and_version(curr_script);
            if (llGetListLength(inv_split) == 2) {
                // see if this script is more ancient.
                string inv_version_string = llList2String(inv_split, 1);  // the version found.
                // must make sure that the retiring script is completely the identical basename;
                // just matching in the front doesn't make it a relative.
                if ( (llList2String(inv_split, 0) == basename)
                    && ((float)inv_version_string < (float)version_string) ) {
                    // remove script with same name from inventory that has inferior version.
                    llRemoveInventory(curr_script);
                }
            }
        }
    }
}
//
// separates the base script name and version number.  used by auto_retire.
list compute_basename_and_version(string to_chop_up)
{
    // minimum script name is 2 characters plus a version.
    integer space_v_posn;
    // find the last useful space and 'v' combo.
    for (space_v_posn = llStringLength(to_chop_up) - 3;
        (space_v_posn >= 2) && (llGetSubString(to_chop_up, space_v_posn, space_v_posn + 1) != " v");
        space_v_posn--) {
        // look for space and v but do nothing else.
    }
    if (space_v_posn < 2) return [];  // no space found.
    // now we zoom through the stuff after our beloved v character and find any evil
    // space characters, which are most likely from SL having found a duplicate item
    // name and not so helpfully renamed it for us.
    integer indy;
    for (indy = llStringLength(to_chop_up) - 1; indy > space_v_posn; indy--) {
        if (llGetSubString(to_chop_up, indy, indy) == " ") {
            // found one; zap it.  since we're going backwards we don't need to
            // adjust the loop at all.
            to_chop_up = llDeleteSubString(to_chop_up, indy, indy);
        }
    }
    string full_suffix = llGetSubString(to_chop_up, space_v_posn, -1);
    // ditch the space character for our numerical check.
    string chop_suffix = llGetSubString(full_suffix, 1, llStringLength(full_suffix) - 1);
    // strip out a 'v' if there is one.
    if (llGetSubString(chop_suffix, 0, 0) == "v")
        chop_suffix = llGetSubString(chop_suffix, 1, llStringLength(chop_suffix) - 1);
    // if valid floating point number and greater than zero, that works for our version.
    string basename = to_chop_up;  // script name with no version attached.
    if ((float)chop_suffix > 0.0) {
        // this is a big success right here.
        basename = llGetSubString(to_chop_up, 0, -llStringLength(full_suffix) - 1);
        return [ basename, chop_suffix ];
    }
    // seems like we found nothing useful.
    return [];
}
//
//////////////

default
{
    state_entry() { if (llSubStringIndex(llGetObjectName(),  "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default
{
    state_entry()
    {
        auto_retire();
        last_position = llGetPos();
        change_size(NORMAL_SIZE);
        llSetTimerEvent(PUFFING_PERIOD);  // puff out this frequently.
    }
    
    on_rez(integer parm) { llResetScript(); }

    timer() { emulate_puffer_fish(); }
}

