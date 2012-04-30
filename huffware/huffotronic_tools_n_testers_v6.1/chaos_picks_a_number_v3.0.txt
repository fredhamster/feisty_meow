
// huffware script: chaos picks a number, by fred huffhines.
//     (was formerly called "raffles asks chaos a question")
//
// produces a random number between 0 and the MAXIMUM_VALUE defined below.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

integer MAXIMUM_VALUE = 100;
    // this is the highest number the generator will produce.
    // the range is from zero, a very poor pick, to the number above
    // (the best pick), inclusive.

///////////////

// from hufflets...

// locates the string "text" in the list to "search_in".
integer find_in_list(list search_in, string text)
{ 
    integer len = llGetListLength(search_in);
    integer i; 
    for (i = 0; i < len; i++) { 
        if (llList2String(search_in, i) == text) 
            return i; 
    } 
    return -1;
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

// variables used in the script...

list participants = [];
    // the people who have already received a number in this drawing.

integer highest_picked = 0;
    // the highest picked so far.

string winner_so_far = "";
    // the name of the person who got the highest number to this point.

integer lottery_finished = FALSE;
    // true when the lottery has been completed.

initialize()
{
    highest_picked = 0;
    participants = [];
    winner_so_far = "";
    lottery_finished = FALSE;
    llSay(0, "ready to draw random numbers from 0 to "
        + (string)MAXIMUM_VALUE + "...");
    llListen(0, "", llGetOwner(), "");
}

// reports the winner, but adds "prefix" to the front of our text and
// will say 'so far' if "provisional" flag is true.
call_winner(string prefix, integer provisional)
{
    string disclaimer = "";  // extra text if not final result.
    if (provisional) disclaimer = "so far ";
    string winner = winner_so_far;
    if (!llStringLength(winner)) winner = "no one(!)";
    llSay(0, prefix + "winner " + disclaimer + "is " + winner
        + " who picked " + (string)highest_picked + ".");
}

announce_final_winner()
{
    call_winner("The **final** ", FALSE);
}

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
        initialize();
    }
    
    on_rez(integer param) { llResetScript(); }

    touch_start(integer total_number)
    {
        integer which_picker;
        for (which_picker = 0; which_picker < total_number; which_picker++) {
            if (llDetectedKey(which_picker) == llGetOwner()) {
                llOwnerSay("as owner you can say\n'#winner' to hear the current winner,\n'#final' to close the drawing and,\n'#reset lottery' to start a new drawing.");
                call_winner("the ", TRUE);
                return;
            }
            
            if (lottery_finished) {
                // just report that this one's finished.
                announce_final_winner();
                return;
            }
            
            // if we got to here, this is a normal person (not the owner) who
            // might still be allowed to participate in an active drawing.
            
            // get a random number between 0 and almost up to the max.
            float rando = llFrand(MAXIMUM_VALUE);
            // convert to int, which makes the range from zero to the max inclusive.
            integer int_rando = llRound(rando);
            string letter = "";  // default is no extra letter.
            // pick the right article for english.
            if ( (int_rando == 8)
                || (int_rando == 11)
                || (int_rando == 18)
                || (int_rando / 10 == 8) ) {
                // these cases like an "an" instead of an "a".
                // e.g., an eleven, an eight, an eighty-three, an eighteen.
                letter = "n";
            }

            // make sure this person hasn't already played.
            string name = llDetectedName(which_picker);
            if (find_in_list(participants, name) >= 0) {
                call_winner(name + " already picked a number,\nand the ", TRUE);
                return;
            }

            // add the new participant to the list.
            participants += [ name ];
                // list kludge for better memory usage.
            llSay(0, name + " drew a" + letter + " " + (string)int_rando + ".");
            if (highest_picked < int_rando) {
                llSay(0, name + " has the new highest number!!!");
                highest_picked = int_rando;
                winner_so_far = name;
            } else if (highest_picked == int_rando) {
                call_winner("oh, so close; " + name + " tied with the ", FALSE);
            } else {
                call_winner("that's not quite enough " + name + ", since\nthe ", TRUE);
            }
        }
    }
    
    listen(integer channel, string name, key id, string msg) {
        if (msg == "#reset lottery") {
            llSay(0, "resetting the drawing now...");
            if (winner_so_far != "")
                call_winner("forgetting previous ", FALSE);
            initialize();
        } else if (msg == "#winner") {
            call_winner("the ", TRUE);
        } else if (msg == "#final") {
            lottery_finished = TRUE;
            announce_final_winner();
        }
    }
}

