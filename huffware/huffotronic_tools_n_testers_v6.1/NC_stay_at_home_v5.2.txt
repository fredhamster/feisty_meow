
// huffware script: stay at home, by fred huffhines
//
// this script attempts to keep an object at home, which is to say, near where
// it was rezzed.  it is not perfect, and if the object quickly crosses a region
// boundary, the object may get away.  there is an automatic termination option
// for cases where the object is taken away and cannot get back; otherwise the
// object will bleat its position out at regular intevals.
// this script mainly makes sense for physical objects, since non-physical
// objects are not as prone to being pushed around by other avatars.  in addition,
// it really makes the most sense for an object that allows anyone to move it.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

// constants to be adjusted as needed...

float MAXIMUM_ROAMING_RANGE = 12.0;
    // this is the farthest that we let them drag our object away, in meters.

integer AUTO_TERMINATE = FALSE;
    // if the object crosses a region boundary (exits sim, enters new one), then
    // it will zap itself if this flag is set to true.  it will also go poof if
    // it reaches escape velocity and travels off world.

integer KEEP_IT_PHYSICAL = TRUE;
    // if this is true, then the object's state as physical is continually
    // refreshed.

integer ROTATE_TO_TARGET = FALSE;
    // if this is true, the object will try to rotate to a set position.

integer ONLY_ROTATE_FIXED_AXES = FALSE;
    // if this is true and rotate to target is true, then we will only try to
    // set the rotation on the axes that are specified as "fixed" below.

vector TARGETED_ROTATION = <0.0, 0.0, 0.0>;
    // if the control rotation flag is true, the object will try to reattain
    // this value for its rotation.

// if these are set to true, the no rotation is allowed around the axis.
// note that this decision is totally independent of whether the object has
// a targeted rotation or not.
integer FIXATE_X_AXIS = FALSE;
integer FIXATE_Y_AXIS = FALSE;
integer FIXATE_Z_AXIS = FALSE;

integer MAXIMUM_FAILED_JUMPS = 200;
    // if we are having to make this many attempts to get home, something is
    // really wrong.  treat it like leaving the region; totally awol.

float POSITION_CHECKING_INTERVAL = 8.0;
    // how frequently the object will check where it is, in seconds.

float PANIC_EVERY_N_HOURS = 4.0;
    // this is the number of hours between panic mewling at the owner, if the object
    // did not get told to self-terminate upon leaving the region.

integer DEBUGGING = FALSE;
    // if this is true, then the object will tell about it's problems when it
    // goes off world or into another sim.  note that an object that is not
    // set to auto-terminate is considered important enough to tell the owner;
    // if you don't want to hear from lost objects, always turn on auto-terminate
    // and turn off debugging.

// these mysterious values control how the object rotates to its target.
float ROT_STRENGTH = 1.0;
float ROT_DAMPING = 0.25;
//these values don't work that great, but neither did any other ones.
//  the rotlookat stuff seems overpowered by the physical nature of an object.

// global variables...

vector home_place;
    // where the object starts out inside the sim.  we will try to get back
    // to here if we exceed our maximum distance from home.

vector home_region;
    // which sim we start out in, in terms of the region corners.  if this changes,
    // the object has left the sim.

integer homeward_jump_attempts = 0;
    // tracks how many jumps home we've tried to make.

integer last_alerted = 0;
    // when the script last told the user about having gone missing.
    // if this is zero, we presumably haven't been lost recently.

// requires jaunting library v10.5 or greater.
//////////////
// do not redefine these constants.
integer JAUNT_HUFFWARE_ID = 10008;
    // the unique id within the huffware system for the jaunt script to
    // accept commands on.  this is used in llMessageLinked as the num parameter.
string HUFFWARE_PARM_SEPARATOR = "{~~~}";
    // this pattern is an uncommon thing to see in text, so we use it to separate
    // our commands in link messages.
string HUFFWARE_ITEM_SEPARATOR = "{|||}";
    // used to separate lists of items from each other when stored inside a parameter.
    // this allows lists to be passed as single string parameters if needed.
integer REPLY_DISTANCE = 100008;  // offset added to service's huffware id in reply IDs.
//////////////
// commands available via the jaunting library:
string JAUNT_COMMAND = "#jaunt#";
    // command used to tell jaunt script to move object.  pass a vector with the location.
string JAUNT_LIST_COMMAND = "#jauntlist#";
    // like regular jaunt, but expects a list of vectors as the first parameter; this list
    // should be in the jaunter notecard format (separated by pipe characters).
    // the second parameter, if any, should be 1 for forwards traversal and 0 for backwards.
//
//////////////
// joins a list of parameters using the parameter sentinel for the library.
string wrap_parameters(list to_flatten)
{ return llDumpList2String(to_flatten, HUFFWARE_PARM_SEPARATOR); }
//
// encases a list of vectors in the expected character for the jaunting library.
string wrap_vector_list(list to_wrap)
{ return llDumpList2String(to_wrap, HUFFWARE_ITEM_SEPARATOR); }
//////////////

// asks the jaunting library to take us to the target using a list of waypoints.
request_jaunt(list full_journey, integer forwards)
{
//    jaunt_responses_awaited++;
    llMessageLinked(LINK_THIS, JAUNT_HUFFWARE_ID, JAUNT_LIST_COMMAND,
        wrap_vector_list(full_journey) + HUFFWARE_PARM_SEPARATOR + (string)forwards);
}

// jaunts back to our home location.
attempt_to_go_home()
{
    // jump back to home.
    request_jaunt([llGetPos(), home_place], TRUE);
}

// we panic at the user this frequently, in seconds.
float panic_interval() { return PANIC_EVERY_N_HOURS * 60.0 * 60.0; }

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
        // we just woke up here.  remember where we started out.
        home_place = llGetPos();
        home_region = llGetRegionCorner();
        if (AUTO_TERMINATE) {
            // make the object go kerflooey if it leaves the world entirely.
            llSetStatus(STATUS_DIE_AT_EDGE, TRUE);
        }
        // disallow rotation if we were requested to.
        llSetStatus(STATUS_ROTATE_X, !FIXATE_X_AXIS);
        llSetStatus(STATUS_ROTATE_Y, !FIXATE_Y_AXIS);
        llSetStatus(STATUS_ROTATE_Z, !FIXATE_Z_AXIS);
        
        if (ROTATE_TO_TARGET) {
//hmmm: maybe still offer gradual form as an option also?
            
            // set our rotational target to the configured position.
//            llRotTarget(llEuler2Rot(TARGETED_ROTATION), 0.01);
            // if we're supposed to rotate, we need to start that movement
            // towards our target rotation.
//            llRotLookAt(llEuler2Rot(TARGETED_ROTATION), ROT_STRENGTH, ROT_DAMPING);
        }
        // start the timer that checks on our position.
        llSetTimerEvent(POSITION_CHECKING_INTERVAL);
    }

    on_rez(integer parm) {
        llResetScript();  // a cop-out; just reroutes rezzing into a script reset.
    }
    
    timer() {

        if (ROTATE_TO_TARGET) {
            vector real_rot_target = TARGETED_ROTATION;
            vector curr_rot = llRot2Euler(llGetRot());
            
            if (ONLY_ROTATE_FIXED_AXES) {
                // set our rotation to the target, but only for fixed axes.
                if (!FIXATE_X_AXIS) real_rot_target.x = curr_rot.x;
                if (!FIXATE_Y_AXIS) real_rot_target.y = curr_rot.y;
                if (!FIXATE_Z_AXIS) real_rot_target.z = curr_rot.z;
            }

            // see if we even need to do any rotation.
            if (curr_rot != real_rot_target) {
                // yes, we need to move back to the preferred rotation for at least one axis.
                llSetStatus(STATUS_PHYSICS, FALSE);  // physics property gets fixed in next block.
                llSetRot(llEuler2Rot(real_rot_target));
            }
        }

        if (KEEP_IT_PHYSICAL) {
            // remind the object that it's physical, or set it if it wasn't.  sometimes
            // a physical object can get jinxed when bouncing into other people's lands
            // and lose its status as a physical object.  we correct for that here.
            llSetStatus(STATUS_PHYSICS, TRUE);
        }

        integer time_to_panic = FALSE;  // set to true if we should panic now.

        // check whether the object is still in the appropriate region.
        integer region_okay = (home_region == llGetRegionCorner());

        // check to see if we're watching for a specific time.
        if (last_alerted != 0) {
            // ooh, we've already hit panicky conditions in the past.  let's see if
            // it's time to tell the owner about it.
            if (panic_interval() <= llAbs(llGetUnixTime() - last_alerted)) {
                // time to tell them about this, oh yes.
                time_to_panic = TRUE;
            }
        } else {
            // evaluate whether we are in the right condition (right sim and
            // without being boxed in).
            if (!region_okay || (homeward_jump_attempts > MAXIMUM_FAILED_JUMPS)) {
                // this is not a good situation.  we will now set the last alerted
                // timer so as to force a panic message next time through the timer
                // loop.
                last_alerted = llAbs(llGetUnixTime() - (integer)panic_interval() - 1000);
                // decide when to fire the timer next.
                if (AUTO_TERMINATE) {
                    llSetTimerEvent(0.1);  // fire very soon to zap.
                } else {
                    llSetTimerEvent(POSITION_CHECKING_INTERVAL * 5);
                        // slow the checking down a bit, since we don't expect to
                        // magically get back to our sim (although it could happen!).
                }
            }
        }

        // time to panic just means that we should tell the owner about our problematic
        // location, or auto-terminate if configured that way.
        if (time_to_panic) {
            // well, we've got to say something about this dire situation.
            string reason = "trapped by unfriendly turf";
            if (!region_okay) reason = "crossed region boundary";
            if (AUTO_TERMINATE) {
                // enforce the rules; we left the building and we have no ticket back.
                if (DEBUGGING) {
                    llOwnerSay(reason + " -- object death -- was in "
                        + llGetRegionName() + " at " + (string)llGetPos());
                }
                llDie();
            }
            // whew, we ducked that deadly issue.  let the owner know that we
            // don't like this place.
            llOwnerSay(reason + "; please come get me:\nlast in "
                + llGetRegionName() + " at " + (string)llGetPos());
            last_alerted = llGetUnixTime();  // reset, since we just alerted.
            return;  // don't bother trying to move anywhere.
        }
        
        if (region_okay && (last_alerted != 0) ) {
            // hmmm, did we recover our position?
            if (homeward_jump_attempts <= MAXIMUM_FAILED_JUMPS) {
                // we didn't expend too many jumps, so try going home again.
                last_alerted = 0;
                // also go back to our normal timing cycle.
                llSetTimerEvent(POSITION_CHECKING_INTERVAL);
            }
        }
        
        if (!region_okay || (homeward_jump_attempts > MAXIMUM_FAILED_JUMPS)
            || (last_alerted != 0) ) {
            // we said our piece, if any.  we can't be fuddling around here.
            return;
        }

        // we're still in the right region, so check how far we are from the starting
        // place.  we'll try to move back if someone dragged us too far away from it.
        float distance = llVecDist(llGetPos(), home_place);
        if (distance > MAXIMUM_ROAMING_RANGE) {
            // this is not good; we need to get back to our starting point.
            attempt_to_go_home();
            homeward_jump_attempts++;
        } else {
            // reset our counter, since we are close enough to home.
            homeward_jump_attempts = 0;
        }
    }
}

