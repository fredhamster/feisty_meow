
// huffware script: weapon fire, by fred huffhines.
//
// finds the first item in inventory and uses it as a bullet.  it also finds the first
// sound in inventory and uses that as the report noise for firing the bullet.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

// modifiable constants...

integer LEFT_HANDED = TRUE;

//float SPEED = 100.0;
float SPEED = 20.0;

float DELAY = 0.0;  // an enforced delay before the weapon can fire again.

float VOLUME = 0.2;

integer USE_ANIMATION = FALSE;  // if this is true, then the bearer is posed.

// global variables...

vector velocity;                          
vector position;                         
rotation rotation_vec;                       

integer have_permissions = FALSE;
integer armed = TRUE;

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

// performs the act of firing the bullet.
fire()
{
    if (armed) {
        rotation_vec = llGetRot();               
        velocity = llRot2Fwd(rotation_vec);           
        position = llGetPos();               
        position = position + velocity;               
        position.z += 0.75;                 
        velocity = velocity * SPEED;
        if (llGetInventoryNumber(INVENTORY_SOUND) > 0)
            llTriggerSound(llGetInventoryName(INVENTORY_SOUND, 0), VOLUME);
        integer i;
        for (i = 0; i < llGetInventoryNumber(INVENTORY_OBJECT); i++)
            llRezObject(llGetInventoryName(INVENTORY_OBJECT, i), position, velocity, rotation_vec, 2814);
        if (DELAY != 0.0) {
            armed = FALSE;
            llSetTimerEvent(DELAY);
        }
    }
}

default
{
    state_entry() { if (llSubStringIndex(llGetObjectName(),  "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default
{
    state_entry() {
        auto_retire();
        if (llGetInventoryNumber(INVENTORY_SOUND) > 0)
            llPreloadSound(llGetInventoryName(INVENTORY_SOUND, 0));
        if (!have_permissions) {
            llRequestPermissions(llGetOwner(),
                PERMISSION_TRIGGER_ANIMATION | PERMISSION_TAKE_CONTROLS);
        }
    }

    on_rez(integer param) { llResetScript(); }

    run_time_permissions(integer permissions) {
        if (permissions == PERMISSION_TRIGGER_ANIMATION | PERMISSION_TAKE_CONTROLS) {
            llTakeControls(CONTROL_ML_LBUTTON, TRUE, FALSE);
            if (USE_ANIMATION) {
                if (LEFT_HANDED) {
                    llStartAnimation("hold_L_bow");
                    llStopAnimation("aim_L_bow");
                } else {
                    llStartAnimation("hold_R_handgun");
                    llStopAnimation("aim_R_handgun");
                }
            }
            have_permissions = TRUE;
        }
    }

    attach(key attachedAgent) {
        if (attachedAgent != NULL_KEY) {
            llRequestPermissions(llGetOwner(),
                PERMISSION_TRIGGER_ANIMATION | PERMISSION_TAKE_CONTROLS);   
        } else {
            if (have_permissions) {
                llStopAnimation("hold_R_handgun");
                llStopAnimation("aim_R_handgun");
                llReleaseControls();
                llSetRot(<0,0,0,1>);
                have_permissions = FALSE;
            }
        }
    }

    control(key name, integer levels, integer edges)  {
        if ( ((edges & CONTROL_ML_LBUTTON) == CONTROL_ML_LBUTTON)
                && ((levels & CONTROL_ML_LBUTTON) == CONTROL_ML_LBUTTON) ) {
            fire();
        }
    }
    
    timer() {
        llSetTimerEvent(0.0);
        armed = TRUE;
    }
}
