
//huffware script: swiveller, modified by fred huffhines.

//original docs:
// This script is as end-user friendly as possible. Moving the swing will make it reset to it's new position and orientation. Occasionally JUST rotating it won't work, so in that case you will need to nudge it sideways just a little (1mm will do). This is a quirk of LSL and it intermittent.
// Put this script in the pivot - the thing the swing swings around (it doesn't have to be a long rod). That needs to be the root of the prim set that actually swings - if you want/need a frame that needs to be a separate item. SL doesn't yet allow us hierarchical linking.
//
// fred's modifications are licensed by:
//   this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
//   do not use it in objects without fully realizing you are implicitly accepting that license.
//

// configuration variables -- these control how the swing behaves.

integer is_swinging = FALSE;  // if set to FALSE, then the swing is not running when rezzed.

float SWING_PERIOD = 0.4;
    // the delay between changing the swing's position.  decrease this number to increase
    // speed of swinging.

integer SWING_STEPS = 12;
    // The total number of steps in the swing's path.  More steps makes for a
    // smoother swing.  More steps (alone) means slower swinging also--time for
    // a complete swing cycle is SWING_STEPS * swing_period.

integer SWING_ANGLE_DEGREES = 10;
    // How far from the vertical the swing will move.
//This is in both directions?

/////

integer swing_step = 1;
    // the current swing position.
    
float swingRad;

vector normal;

rotation Inverse(rotation r) { return <-r.x, -r.y, -r.z, r.s>; }

rotation GetParentRot() { return Inverse(llGetLocalRot()) * llGetRot(); }

SetLocalRot(rotation x) { llSetRot(x*Inverse(GetParentRot())); }


//////////////

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
    state_entry()
    {
        normal = llRot2Euler(llGetRot());
        swingRad = SWING_ANGLE_DEGREES * DEG_TO_RAD;
        llSetTouchText("Swing");
    }
    touch_start(integer num)
    {
        if (is_swinging) {
            is_swinging = FALSE;
            llSetTouchText("Swing");
        } else {
            is_swinging = TRUE;
            llSetTouchText("Stop swing");
            llSetTimerEvent(SWING_PERIOD);
        }
    }
    timer()
    {
        float stepOffset = (float)swing_step / SWING_STEPS * TWO_PI;
        if (swing_step > SWING_STEPS) swing_step = 1;
        if (!is_swinging && (swing_step>=SWING_STEPS || swing_step==SWING_STEPS/2)) {
            llSetTimerEvent(0.0);
            SetLocalRot(llEuler2Rot(<normal.x, normal.y,
                normal.z + swingRad*llSin(stepOffset)>));
        } else {
            SetLocalRot(llEuler2Rot(<normal.x, normal.y,
                normal.z + swingRad*llSin(stepOffset)>));
            swing_step++;
        }
    }
    moving_end()
    {
        normal = llRot2Euler(llGetRot());
    }
}
