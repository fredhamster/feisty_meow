
// huffware script: jaunting library, by fred huffhines, released under GPL-v3 license.
//
// this script is a library of useful teleportation and movement calls.  it should be added
// into an object along with other scripts that use the library.  the jaunting library
// responds to linked messages as commands.
// parts of this script are based on warpPos from "Teleporter Script v 3.0 by Asira Sakai",
// which was found at the LSL wiki.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

// global constants...

float PROXIMITY_REQUIRED = 0.1;
    // how close we must be to the target location to call the jaunt done.
    // we make this fairly low accuracy since we don't want jumps in space to immediately
    // be counted as failures.

// the most jumps the script will try to take.  the overall distance from the start
// to the end can be 10 * MAXIMUM_JUMPS_PER_CALL meters.
integer MAXIMUM_JUMPS_PER_CALL = 84;

// global variables...

vector last_posn;  // used to calculate jump distances.

// jaunting library link message API...
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
// commands available via the jaunting library:
string JAUNT_COMMAND = "#jaunt#";
    // command used to tell jaunt script to move object.  pass a vector with the location.
string FULL_STOP_COMMAND = "#fullstop#";
    // command used to bring object to a halt.
string REVERSE_VELOCITY_COMMAND = "#reverse#";
    // makes the object reverse its velocity and travel back from whence it came.
string SET_VELOCITY_COMMAND = "#setvelocity#";
    // makes the velocity equal to the vector passed as the first parameter.
string JAUNT_UP_COMMAND = "#jauntup#";
string JAUNT_DOWN_COMMAND = "#jauntdown#";
    // commands for height adjustment.  pass a float for number of meters to move.
string JAUNT_LIST_COMMAND = "#jauntlist#";
    // like regular jaunt, but expects a list of vectors as the first parameter; this list
    // should be in the jaunter notecard format (separated by pipe characters).
    // the second parameter, if any, should be 1 for forwards traversal and 0 for backwards.
//////////////

// returns what we consider to be safe to do in one big jump.
float distance_safe_to_jaunt() { return (float)(MAXIMUM_JUMPS_PER_CALL - 4) * 10.0; }

// tests whether the destination is safe for an object to enter.
integer safe_for_entry(vector where)
{
    if (outside_of_sim(where)) {
        // that's an obvious wrong idea; it tends to break us.
        return FALSE;
    }
    return TRUE;
}

// helper function for warp_across_list.  this adds one jump vector to the
// list of rules as long as the destination is interesting.
list process_warp_item(vector next_jump)
{
//log_it("mem: " + (string)llGetFreeMemory());
    // calculate the number of jumps needed.
    integer jumps = (integer)(llVecDist(next_jump, last_posn) / 10.0) + 1;
    last_posn = next_jump;  // record for next check.
    // set up our list which we'll replicate.
    list rules = [ PRIM_POSITION, next_jump ];
    // Try and avoid stack/heap collisions.
    if (jumps > MAXIMUM_JUMPS_PER_CALL - 1) jumps = MAXIMUM_JUMPS_PER_CALL;
    // add the rules repeatedly to get the effective overall jump done.
    integer count = 1;
    while ( (count = count << 1) < jumps)
        rules += rules;
    // magnify the rule list before really adding it.  this gets us to the proper
    // final number of jumps.
    return rules + llList2List(rules, (count - jumps) << 1 - 2, count);
}

// originally based on warpPos from lsl wiki but drastically modified.
// uses a set of transfer points instead of a single target.
list warp_across_list(list full_journey, integer forwards)
{
    // make sure the list didn't run out.
    if (llGetListLength(full_journey) == 0) return [];
    if (forwards) {
        // forwards traversal of the list.
        vector next_jump = (vector)llList2String(full_journey, 0);
        // shortcut the jumps if we're already there.
        if (next_jump == llGetPos())
            return warp_across_list(chop_list(full_journey, 1,
                llGetListLength(full_journey) - 1), forwards);
        // calculate our trajectory for the next jump and add in all subsequent jumps.
        return process_warp_item(next_jump)
            + warp_across_list(chop_list(full_journey, 1, llGetListLength(full_journey) - 1), forwards);
    } else {
        // reverse traversal of the list.
        vector next_jump = (vector)llList2String(full_journey, llGetListLength(full_journey) - 1);
        // shortcut the jumps if we're already there.
        if (next_jump == llGetPos())
            return warp_across_list(chop_list(full_journey, 0,
                llGetListLength(full_journey) - 2), forwards);
        // calculate our trajectory for the next jump and add in all subsequent jumps.
        return process_warp_item(next_jump)
            + warp_across_list(chop_list(full_journey, 0, llGetListLength(full_journey) - 2), forwards);
    }
}

// the main function that performs the jaunting process.
// now also supports adding a rotation into the mix to avoid paying the extra delay
// cost of calling llSetRot.
jaunt(list full_journey, integer forwards, string command_used,
    integer use_rotation, rotation rot, integer reply_to_request)
{
//log_it("jaunt: fullj=" + (string)full_journey + " forew=" + (string)forwards);
    // set up our global variables...
    last_posn = llGetPos();
    // calculate the trip and run it.
    list rot_add;
    if (use_rotation)
        rot_add = [ PRIM_ROTATION, rot ];
    llSetPrimitiveParams(warp_across_list(full_journey, forwards) + rot_add);
    if (reply_to_request) {
        // pick out the last target in the list based on the direction we're moving.
        integer last_indy = 0;
        if (forwards == TRUE) last_indy = llGetListLength(full_journey) - 1;
        vector last_jump = (vector)llList2String(full_journey, last_indy);
        // final judge of success here is how close we got to the target.
        integer landed_there = (llVecDist(llGetPos(), last_jump) <= PROXIMITY_REQUIRED);
        send_reply(LINK_THIS, [landed_there, llGetPos()], command_used);
    }
}

//////////////

// sets the object's speed to "new_velocity".
// if "local_axis" is TRUE, then it will be relative to the object's
// own local coordinates.
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

// attempts to bring the object to a complete stop.
full_stop()
{
    llSetForce(<0,0,0>, FALSE);
    set_velocity(<0,0,0>, FALSE);
}

// sends an object back along its trajectory.
reverse_velocity(integer local_axis)
{
    vector current_velocity = llGetVel();
    if (local_axis) {
        rotation rot = llGetRot();
        current_velocity /= rot;  // undo the rotation.
    }
    vector new_velocity = -2 * current_velocity;
    new_velocity *= llGetMass();
    llApplyImpulse(new_velocity, local_axis);
}

// teleports to the new "location", if possible.  does not change object's phantom / physical
// states.  this will do the jaunt in multiple steps if the distance from here to "location"
// is too large.
apportioned_jaunt(vector location, string command_used, integer use_rotation, rotation rot,
    integer should_send_reply)
{
    if (!safe_for_entry(location)) {
        // that's not good.  we should not allow the script to get broken.
        if (should_send_reply)
            send_reply(LINK_THIS, [FALSE, llGetPos()], command_used);
        return;
    }
    // go to position specified, by leapfrogs if too long.
    integer chunk_of_jump;
    integer MAX_CHUNKS = 200;  // multiplies the distance a single jaunt can cover.
    for (chunk_of_jump = 0; chunk_of_jump < MAX_CHUNKS; chunk_of_jump++) {
        vector interim_vec = location - llGetPos();
        float jump_dist = llVecDist(llGetPos(), location);
        integer reply_needed = TRUE;
        if (jump_dist > distance_safe_to_jaunt()) {
            // the entire distance cannot be jumped.  do the part of it that fits.
            float proportion_can_do = distance_safe_to_jaunt() / jump_dist;
            interim_vec *= proportion_can_do;
            reply_needed = FALSE;  // don't reply when this is not full jump.
        }
        interim_vec += llGetPos();  // bias jump back to where we are.
//log_it("jumping from " + (string)llGetPos() + " to " + (string)interim_vec);
        jaunt([llGetPos(), interim_vec], TRUE, command_used, use_rotation, rot,
            reply_needed && should_send_reply);
        float dist_now = llVecDist(llGetPos(), interim_vec);
        if (dist_now > PROXIMITY_REQUIRED) {
//log_it("failed to make interim jump, dist left is " + (string)dist_now);
            // bail out.  we failed to get as far as we thought we should.
            chunk_of_jump = MAX_CHUNKS + 10;
            if (!reply_needed) {
                // we need to send the reply we hadn't sent yet.
                if (should_send_reply)
                    send_reply(LINK_THIS, [FALSE, llGetPos()], command_used);
            }
        } else if (llVecDist(llGetPos(), location) <= PROXIMITY_REQUIRED) {
            // leave loop for a different reason; we got there.
            chunk_of_jump = MAX_CHUNKS + 10;
            if (!reply_needed) {
                // we need to send the reply we hadn't sent yet.
                if (should_send_reply)
                    send_reply(LINK_THIS, [TRUE, llGetPos()], command_used);
            }
        }
    }
}

// the entire distance embodied in the list of targets.  this is a little smart,
// in that if there's only one target, we assume we're going from "here" to that
// one target.
float total_jaunt_distance(list targets)
{
    if (!llGetListLength(targets)) return 0.0;
    // add in "this" location if they omitted it.
    if (llGetListLength(targets) < 2)
        targets = [ llGetPos() ] + targets;
    integer disindy;
    float total_dist = 0.0;
    vector prev = (vector)llList2String(targets, 0);
    for (disindy = 1; disindy < llGetListLength(targets); disindy++) {
        vector next = (vector)llList2String(targets, disindy);
        total_dist += llVecDist(prev, next);
        prev = next;
    }
    return total_dist;
}

// jaunts to a target via a set of intermediate locations.  can either go forwards
// through the list or backwards.
phantom_jaunt_to_list(list targets, integer forwards, string command_used,
    integer use_rotation, rotation rot)
{
    vector final_loc;
    if (forwards) final_loc = (vector)llList2String(targets, llGetListLength(targets) - 1);
    else final_loc = (vector)llList2String(targets, 0);

//hmmm: check each destination in list??
    if (!safe_for_entry(final_loc)) {
        // that's not good.  we should not allow the script to get broken.
        send_reply(LINK_THIS, [FALSE, llGetPos()], command_used);
        return;
    }
    
    // this turns off the physics property for the object, so that jaunt and
    // llSetPos will still work.  this became necessary due to havok4.
    integer original_phantomosity = llGetStatus(STATUS_PHANTOM);
    integer original_physicality = llGetStatus(STATUS_PHYSICS);
    if (original_physicality != FALSE) llSetStatus(STATUS_PHYSICS, FALSE);
    if (original_phantomosity != TRUE) llSetStatus(STATUS_PHANTOM, TRUE);

    integer send_reply_still = TRUE;  // true if we should send a reply when done.

    // iterate through our list of targets and either we will jaunt to the next
    // place directly or we will have to wrap a few destinations due to sim crossing.
    while (llGetListLength(targets) > 0) {
        vector next_loc;
        if (forwards) next_loc = (vector)llList2String(targets, 0);
        else next_loc = (vector)llList2String(targets, llGetListLength(targets) - 1);
        if (outside_of_sim(next_loc)) {
            log_it("bad jaunt path: first target is out of sim.");
            send_reply(LINK_THIS, [FALSE, llGetPos()], command_used);
            return;  // skip that badness.
        }
        
        // check how much total distance we have in the path that's left.  if it's under our
        // limit, then we'll just take it in one jump.
        float total_dist = total_jaunt_distance(targets);

        // if we're below the threshold, we'll just jump now.
        integer already_jumped = FALSE;
        if (total_dist < distance_safe_to_jaunt()) {
            jaunt(targets, forwards, command_used, use_rotation, rot, TRUE);
            targets = [];  // reset the list now.
            send_reply_still = FALSE;  // we have already sent the reply in jaunt().
            already_jumped = TRUE;  // don't do anything else.
        }
        if (!already_jumped) {
            vector next_plus_1 = ZERO_VECTOR;  // default cannot fail our "is inside sim" check.
            if (llGetListLength(targets) > 1) {
                if (forwards) next_plus_1 = (vector)llList2String(targets, 1);
                else next_plus_1 = (vector)llList2String(targets, llGetListLength(targets) - 2);
            }
            if (outside_of_sim(next_plus_1)) {
//hmmm: eventually find all the negative ones in a row and do them in combo, rather than
//      just giving up here and doing a jaunt to the rest..
                jaunt(targets, forwards, command_used, use_rotation, rot, TRUE);
                targets = [];  // reset the list now.
                send_reply_still = FALSE;  // we have already sent the reply in jaunt().
            } else {
                // we've passed the negativity test, so we can at least jump to the next place.
                
                // zap the next location, since we're about to jump there.
                integer zap_pos = 0;
                if (!forwards) zap_pos = llGetListLength(targets) - 1;
                targets = llDeleteSubList(targets, zap_pos, zap_pos);        
        
                // only bother jumping if we're not already there.
                if (llVecDist(next_loc, llGetPos()) > PROXIMITY_REQUIRED) {
                    apportioned_jaunt(next_loc, command_used, use_rotation, rot, FALSE);
                }
            }
        }
    }

    if (send_reply_still) {
        integer yippee = TRUE;  // assume we succeeded until we prove otherwise.
        if (llVecDist(final_loc, llGetPos()) > PROXIMITY_REQUIRED)
            yippee = FALSE;
        send_reply(LINK_THIS, [yippee, llGetPos()], command_used);
    }

    // return to prior characteristics.
    if (original_phantomosity != TRUE) llSetStatus(STATUS_PHANTOM, original_phantomosity);
    if (original_physicality != FALSE) llSetStatus(STATUS_PHYSICS, original_physicality);
}

// implements our API for jumping around.
handle_link_message(integer sender, integer huff_id, string msg, key id)
{
    // separate the list out
    list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
    if (msg == JAUNT_COMMAND) {
        // use list to string to avoid non-conversions.
        vector v = (vector)llList2String(parms, 0);
        rotation rot = <0.0, 0.0, 0.0, 1.0>;
        integer gave_rot = llGetListLength(parms) > 1;
        if (gave_rot) rot = (rotation)llList2String(parms, 1);  // ooh, they gave us a rotational value also.
//        log_it("gave rot? " + (string)gave_rot + " rot=" + (string)rot);
        phantom_jaunt_to_list([llGetPos(), v], TRUE, msg, gave_rot, rot);
    } else if (msg == FULL_STOP_COMMAND) {
        full_stop();
    } else if (msg == REVERSE_VELOCITY_COMMAND) {
        reverse_velocity(FALSE);
    } else if (msg == SET_VELOCITY_COMMAND) {            
        vector v = (vector)llList2String(parms, 0);
//log_it("jaunting lib received set velocity request for " + (string)v);
        set_velocity(v, FALSE);
    } else if (msg == JAUNT_UP_COMMAND) {
        phantom_jaunt_to_list([ llGetPos(), llGetPos() + <0.0, 0.0, llList2Float(parms, 0)> ],
            TRUE, msg, FALSE, ZERO_ROTATION);
    } else if (msg == JAUNT_DOWN_COMMAND) {
        phantom_jaunt_to_list([ llGetPos(), llGetPos() + <0.0, 0.0, -llList2Float(parms, 0)> ],
            TRUE, msg, FALSE, ZERO_ROTATION);
    } else if (msg == JAUNT_LIST_COMMAND) {
        string destination_list = llList2String(parms, 0);
        list targets = llParseString2List(destination_list, [HUFFWARE_ITEM_SEPARATOR], []);
        destination_list = "";
        integer forwards = TRUE;
        // snag the directionality for the list, if specified.
        if (llGetListLength(parms) > 1) forwards = llList2Integer(parms, 1);
        rotation rot = <0.0, 0.0, 0.0, 1.0>;
        integer gave_rot = llGetListLength(parms) > 2;
        if (gave_rot) rot = llList2Rot(parms, 2);  // ooh, they gave us a rotational value also.        
        phantom_jaunt_to_list(targets, forwards, msg, gave_rot, rot);
        targets = [];
    }
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
//    llSay(108, llGetScriptName() + "--" + (string)debug_num + ": " + to_say);
}
//////////////

// a simple version of a reply for a command that has been executed.  the parameters
// might contain an outcome or result of the operation that was requested.
send_reply(integer destination, list parms, string command)
{
    llMessageLinked(destination, JAUNT_HUFFWARE_ID + REPLY_DISTANCE, command,
        llDumpList2String(parms, HUFFWARE_PARM_SEPARATOR));
}

// returns TRUE if the value in "to_check" specifies a legal x or y value in a sim.
integer valid_sim_value(float to_check)
{
    if (to_check < 0.0) return FALSE;
    if (to_check >= 257.0) return FALSE;
    return TRUE;
}

integer outside_of_sim(vector to_check)
{
    return !valid_sim_value(to_check.x) || !valid_sim_value(to_check.y);
}

///////////////

// returns the portion of the list between start and end, but only if they are
// valid compared with the list length.  an attempt to use negative start or
// end values also returns a blank list.
list chop_list(list to_chop, integer start, integer end)
{
    integer last_len = llGetListLength(to_chop) - 1;
    if ( (start < 0) || (end < 0) || (start > last_len) || (end > last_len) ) return [];
    return llList2List(to_chop, start, end);
}

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

default {
    state_entry() { if (llSubStringIndex(llGetObjectName(), "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default
{
    state_entry() { auto_retire(); }
    
    link_message(integer sender, integer huff_id, string msg, key id) {
        if (huff_id != JAUNT_HUFFWARE_ID) return;  // not our responsibility.
        handle_link_message(sender, huff_id, msg, id);
    }
}

