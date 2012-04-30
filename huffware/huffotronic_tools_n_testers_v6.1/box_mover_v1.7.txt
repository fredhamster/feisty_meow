
// huffware script: box mover, by fred huffhines.
//
// a really simple script to make a box jump out of one's way.
//
//   this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
//   do not use it in objects without fully realizing you are implicitly accepting that license.
//


//hmmm: fix this to use a notecard for dimensions and members.


// modifiable constants.

vector jump_offset = <4.0, -3.0, 0.0>;  // how far to jump away from the starting place.

list allowed_avatars = [ "fred huffhines", "Chronical Koolhoven", "Mojopickle Haystack",
    "Zeno Olifone", "After9 NightFire", "Wam7c Macchi" ];

// variables below.

vector home;  // where do we live normally?

// makes our list of allowed keys into uniform lower case.
fix_list_items()
{
    integer i;
    list fixed_list;
    for (i = 0; i < llGetListLength(allowed_avatars); i++) {
        string curr = llList2String(allowed_avatars, i);
        curr = llToLower(curr);
        fixed_list += [curr];
    }
    allowed_avatars = fixed_list;
//llOwnerSay("got a new list of: " + (string)allowed_avatars);    
}

//////////////
// huffware script: auto-retire, by fred huffhines, version 2.5.
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
//log_it("invpo=" + (string)posn);
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
//log_it("pos=" + (string)space_v_posn);
    }
    if (space_v_posn < 2) return [];  // no space found.
//log_it("space v@" + (string)space_v_posn);
    // now we zoom through the stuff after our beloved v character and find any evil
    // space characters, which are most likely from SL having found a duplicate item
    // name and not so helpfully renamed it for us.
    integer indy;
    for (indy = llStringLength(to_chop_up) - 1; indy > space_v_posn; indy--) {
//log_it("indy=" + (string)space_v_posn);
        if (llGetSubString(to_chop_up, indy, indy) == " ") {
            // found one; zap it.  since we're going backwards we don't need to
            // adjust the loop at all.
            to_chop_up = llDeleteSubString(to_chop_up, indy, indy);
//log_it("saw case of previously redundant item, aieee.  flattened: " + to_chop_up);
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
    state_entry() {
        auto_retire();  // make sure newest addition is only version of script.
        fix_list_items();  // make sure entries are lower case.
        home = llGetPos();
    }
    
    touch_start(integer total_number) {
llOwnerSay("touched by " + llDetectedName(0));
        // if we're not the owner, then we've got a few extra requirements before
        // we do anything at all.
        if (llDetectedKey(0) != llGetOwner()) {
            list lowname = [ llToLower(llDetectedName(0)) ];
            if (llListFindList(allowed_avatars, lowname) < 0) {
                // this guy is not in our access list.
llOwnerSay("denying " + llDetectedName(0) + " because not in list and not owner.");
                return;
            }
        }
        float max_drift = 0.01;
            // are we considered close enough to home?  not if farther than this amount.
        vector diff = llGetPos() - home;
        if (diff.x < 0.0) diff = <diff.x * -1.0, diff.y, diff.z>;
        if (diff.y < 0.0) diff = <diff.x, diff.y * -1.0, diff.z>;
        if (diff.z < 0.0) diff = <diff.x, diff.y, diff.z * -1.0>;
        // this all is not really a vector difference, but more a check on each component.
        if ( (diff.x > max_drift) || (diff.y > max_drift) || (diff.z > max_drift) ) {
            // we're far away from home, so let's jump back there.
            llSetPos(home);
        } else {
            // we are at home, so we're jumping away.
            llSetPos(home - jump_offset);
            llSetTimerEvent(14.0);  // reset the position in a few secs.
        }
    }
    timer() {
        llSetPos(home);
        llSetTimerEvent(0.0);  // reset timer.
    }
}

