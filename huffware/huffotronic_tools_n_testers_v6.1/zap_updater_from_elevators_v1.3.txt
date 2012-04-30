
// huffware script: zap updater from elevators, by fred huffhines
//
// this script is an evil little assassin that is supposed to remove the updater
// script from the objects it's added to.  it should not remove that script from
// any huffotronic devices though.
// this is an attempt to quell some SL stability problems due to some seeming
// combination of numbers of objects and numbers of scripts.
// we hope to be able to add the updater back in automatically later because the
// script pin should still be set.  let's see if osgrid has that behavior the same
// as second life did.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//


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

// returns the index of the first occurrence of "pattern" inside
// the "full_string".  if it is not found, then a negative number is returned.
integer find_substring(string full_string, string pattern)
{ return llSubStringIndex(llToLower(full_string), llToLower(pattern)); }

// returns TRUE if the "prefix" string is the first part of "compare_with".
integer is_prefix(string compare_with, string prefix)
{ return find_substring(compare_with, prefix) == 0; }

// returns true if this object is a huffotronic updater of some sort.
integer inside_of_updater()
{
    return find_substring(llGetObjectName(), "huffotronic") >= 0;
}

// stops all the scripts besides this one.
knock_around_other_scripts(integer running_state)
{
    integer indy;
    integer insider = inside_of_updater();
    string self_script = llGetScriptName();
    // we set all other scripts to the running state requested.
    for (indy = 0; indy < llGetInventoryNumber(INVENTORY_SCRIPT); indy++) {
        string curr_script = llGetInventoryName(INVENTORY_SCRIPT, indy);
        if ( (curr_script != self_script)
            && (!insider || matches_server_script(curr_script)) ) {
            // this one seems ripe for being set to the state requested.
            llSetScriptState(curr_script, running_state);
        }
    }
}

string SERVER_SCRIPT = "huffotronic update server";
    // the prefix of our server script that hands out updates.

// returns true if a script is a version of our update server.
integer matches_server_script(string to_check)
{
    return is_prefix(to_check, SERVER_SCRIPT);
}

// end hufflets.
//////////////

default
{
    state_entry()
    {
        auto_retire();
        if (inside_of_updater()) return;  // do nothing else.
        
        llWhisper(0, "hello, i'm jeeves.  pleased to meet you.");
        llSleep(32);
        
        string gotta_zap = "huff-update client v";
        integer i;
        for (i = llGetInventoryNumber(INVENTORY_SCRIPT) - 1; i >= 0; i--) {
            string cur = llGetInventoryName(INVENTORY_SCRIPT, i);
            if (is_prefix(cur, gotta_zap)) {
                llWhisper(0, "i, jeeves, will now clean out this script: " + cur);
                llRemoveInventory(cur);
            }
        }
        llSleep(14);
        knock_around_other_scripts(TRUE);
        llSleep(4);
        llWhisper(0, "i'm sorry to say sir that i now must bid you adieu, as i am removing myself from the world.");
        llSleep(1);
        llRemoveInventory(llGetScriptName());
    }
}

