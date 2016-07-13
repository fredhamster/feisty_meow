
// huffware script: jump good, by fred huffhines
// "jump good" is an homage to samurai jack.
//
// if you wear an object containing this script (and using the
// jump_good_jump_strength() value), then this script will make
// your jumps much higher than normal.  other values for the
// jump strength will vary the force on you accordingly.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

//////////////

// this section provides some example forces one might want to exert.
// all floating point numbers below are measured in meters per second squared,
// which is a measurement of acceleration.

float EARTH_ACCELERATION = 9.80665;
    // the acceleration due to gravity on earth.

float JUPITER_ACCELERATION = 25.93;
    // the acceleration on jupiter due to gravity.

float earth_jump_strength() { return EARTH_ACCELERATION - EARTH_ACCELERATION; }
    // normal gravity for the earth; no additional force added.

float jupiter_jump_strength()
{ return EARTH_ACCELERATION - (JUPITER_ACCELERATION - EARTH_ACCELERATION); }
    // this is what the jump force should be when approximating jupiter's gravity.

float jump_good_jump_strength() { return 8.4; }
    // samurai jack was probably able to jump this high after the tree dwellers
    // taught him how to jump good.

integer DEBUGGING = FALSE;
    // if this is true, then extra noise is made about state transitions.

float TIMER_PERIOD = 4.0;
    // how frequently to check our state.

//////////////

// this is the most important jump strength, since it's the one we'll use...

float jump_strength() { return jump_good_jump_strength(); }
  // the amount of force that you're "thrown" upwards with.  this is
  // relative to your mass.  the force of gravity pulls on objects with
  // a force of 9.80665, so if the jump strength exceeds that, the object
  // will never return to earth.

//////////////

// global variables...

key attached_key;  // key of the avatar wearing the object, if any avatar is.

integer _last_seen_flying;  // tracks the attached avatar's flight state.

integer enabled;  // records device's state; if false, then it's off.

//////////////

// makes the force appropriate to the mass of the overall object.
use_configured_force()
{
    float mass = llGetMass();
//log_it("mass is " + (string)mass);
    if (mass == 0.0) mass = 20000;
        // fake there being a good mass.
    vector force = <0,0, mass * jump_strength()>;
//log_it("using force of " + (string)force + ".");
    llSetForce(force, FALSE);
}

// resets force to zero to have normal gravity return.
use_zero_force() {
//log_it("using zero force.");
    
      llSetForce(<0,0,0>, FALSE); }

// changes the objects current velocity to match "new_velocity".
// if the "local_axis" is true, then it applies the velocity to the
// object's own axis.  if false, then it uses the global axis of
// the simulator.
set_velocity(vector new_velocity, integer local_axis)
{
    vector current_velocity = llGetVel();
         
    if (local_axis) {
        rotation rot = llGetRot();
        current_velocity /= rot;  // undo the rotation.
    }
    
    new_velocity -= current_velocity;
    new_velocity *= llGetMass();
    
    llApplyImpulse(new_velocity, local_axis);
}

// picks the right amount of force based on whether the object is
// attached to an avatar and whether the avatar is flying.
set_force_appropriately()
{        
    if (attached_key != NULL_KEY) {
        // make sure we're not flying now; jump force would be redundant.
        if (llGetAgentInfo(llGetOwner()) & AGENT_FLYING) {
            if (!_last_seen_flying) {
//log_it("avatar wasn't flying, is now.  setting zero force.");
                use_zero_force();
            }
            _last_seen_flying = TRUE;
        } else  {
//if (_last_seen_flying) {
//log_it("avatar was just flying, but is not now.  setting configured force.");
//}
            // we're not flying so keep our force set.
            _last_seen_flying = FALSE;
            use_configured_force();
            
        }
    } else {
        // there's no one attached, so just follow our plan with the force.
        use_configured_force();
    }
}

initialize_forces()
{
    enabled = FALSE;
    _last_seen_flying = FALSE;
    attached_key = NULL_KEY;
    llSetTimerEvent(TIMER_PERIOD);  // check avatar's state periodically.
    if (llGetAttached() != 0) {
        // there's an avatar attached right now!  get the stats right.
        attached_key = llGetOwner();
        if (llGetAgentInfo(llGetOwner()) & AGENT_FLYING) _last_seen_flying = TRUE;
        else _last_seen_flying = FALSE;
    }
    set_force_appropriately();
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

//////////////

// from hufflets...

integer debug_num = 0;

// a debugging output method.  can be disabled entirely in one place.
log_it(string to_say)
{
    debug_num++;
    // tell this to the owner.
    string addition;
    if (DEBUGGING) addition = llGetScriptName() + "[" + (string)debug_num + "] ";
    llOwnerSay(addition + to_say);
    // say this on an unusual channel for chat if it's not intended for general public.
//    llSay(108, llGetScriptName() + "[" + (string)debug_num + "] " + to_say);
    // say this on open chat that anyone can hear.  we take off the bling for this one.
//    llSay(0, to_say);
}

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
        auto_retire();
//log_it("state_entry");
        initialize_forces();
    }

    timer() {
        // we can't do anything without physics enabled.
        if (!llGetStatus(STATUS_PHYSICS))
            llSetStatus(STATUS_PHYSICS, TRUE);
        if (!llGetStatus(STATUS_PHANTOM))
            llSetStatus(STATUS_PHANTOM, FALSE);
        if (enabled) set_force_appropriately();
        else use_zero_force();
    }

    attach(key avatar)
    {
//log_it("into attach, key=" + (string)avatar);
        initialize_forces();
    }
    
    on_rez(integer parm) {
//log_it("on_rez");
        initialize_forces();
    }

    touch_start(integer count) {
        if (llDetectedKey(0) != llGetOwner()) return;  // not listening to that guy.
        if (llGetAttached() == 0) return;  // cannot change state if not attached.
        // flip the current state, since they clicked.
        if (enabled) {
            log_it("disabled gravity control.");
            enabled = FALSE;
            use_zero_force();
        } else {
            log_it("enabled gravity control.");
            enabled = TRUE;
            use_configured_force();
        }
    }
}

