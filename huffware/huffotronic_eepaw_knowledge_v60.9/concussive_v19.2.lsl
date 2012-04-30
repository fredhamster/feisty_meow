
// huffware script: concussive (aka blow em up), by fred huffhines
//
// provides a script for a missile that will start tracking the last position it
// hit something at and will start a sensor probe to see if there are avatars there.
// if there are, it will blow up and try to take avatars and active objects with it.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

// hmmm: unimplemented ideas...
//  buddy list; ids to not blow up besides the owner.
//    maybe that's just a team setup.
//    probably better in a notecard?


// configurable parameters...

float sense_distance = 23.0;
    // the distance from the bisconation device within which to search for happy targets.

float sensory_addition = 14.0;
    // the amount of distance that can be added in the case of
    // a failure to sense any avatars or items.

float push_magnitude = 2147483646.0;  //maxint - 1, dealing with svc-2723.
    // how much to push the targets that we have located.

integer MAXIMUM_HITS = 2;
    // the most times the object is allowed through its sensor loops.

float MAX_VEL_ADJUST = 20.0;
    // the maximum amount we would randomly add to the object's velocity after a
    // failure to sense any items.

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

//borrowed from jump good
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

//////////////

//////////////
//borrowed from jaunt script...
///////////////

// returns the portion of the list between start and end, but only if they are valid compared with
// the list length.  an attempt to use negative start or end values also returns a blank list.
list chop_list(list to_chop, integer start, integer end)
{
    integer last_len = llGetListLength(to_chop) - 1;
    if ( (start < 0) || (end < 0) || (start > last_len) || (end > last_len) ) return [];
    return llList2List(to_chop, start, end);
}

// the most jumps the script will try to take.  the overall distance from the start
// to the end can be 10 * MAXIMUM_JUMPS meters.
integer MAXIMUM_JUMPS = 100;

// used to calculate jump distances.
vector last_posn;

// helper function for warp_across_list.  this adds one jump vector to the
// list of rules as long as the destination is interesting.
list process_warp_item(vector next_jump)
{
//tell_owner("last posn " + (string)last_posn);
//tell_owner("next warp " + (string)next_jump);
    // calculate the number of jumps needed.
    integer jumps = (integer)(llVecDist(next_jump, last_posn) / 10.0) + 1;
    // calculate the offset needed for crossing sim boundaries.
///    adjust_offset(next_jump);
    last_posn = next_jump;  // record for next check.
    // set up our list which we'll replicate.
    list rules = [ PRIM_POSITION, next_jump ];
    // Try and avoid stack/heap collisions.
    if (jumps > MAXIMUM_JUMPS) jumps = MAXIMUM_JUMPS;
    // add the rules repeatedly to get the effective overall jump done.
    integer count = 1;
    while ( (count = count << 1) < jumps) rules += rules;
    // magnify the rule list before really adding it.  this gets us to the proper
    // final number of jumps.
    return rules + llList2List(rules, (count - jumps) << 1, count);
}

// originally based on warpPos from lsl wiki but drastically modified.
// uses a set of
// transfer points instead of a single target.
list warp_across_list(list full_journey, integer forwards)
{
//tell_owner("entry to warp across list, list size=" + (string)llGetListLength(full_journey));    
    // make sure the list didn't run out.
    if (llGetListLength(full_journey) == 0) return [];
    if (forwards) {
        // forwards traversal of the list.
        vector next_jump = (vector)llList2String(full_journey, 0);
        // shortcut the jumps if we're already there.
        if (next_jump == llGetPos())
            return warp_across_list(chop_list(full_journey, 1, llGetListLength(full_journey) - 1), forwards);
        // calculate our trajectory for the next jump and add in all subsequent jumps.
        return process_warp_item(next_jump)
            + warp_across_list(chop_list(full_journey, 1, llGetListLength(full_journey) - 1), forwards);
    } else {
        // reverse traversal of the list.
        vector next_jump = (vector)llList2String(full_journey, llGetListLength(full_journey) - 1);
        // shortcut the jumps if we're already there.
        if (next_jump == llGetPos())
            return warp_across_list(chop_list(full_journey, 0, llGetListLength(full_journey) - 2), forwards);
        // calculate our trajectory for the next jump and add in all subsequent jumps.
        return process_warp_item(next_jump)
            + warp_across_list(chop_list(full_journey, 0, llGetListLength(full_journey) - 2), forwards);
    }
}

// the main function that performs the jaunting process.
jaunt(list full_journey, integer forwards)
{
    // set up our global variables...
    last_posn = llGetPos();
    // calculate the trip and run it.
    llSetPrimitiveParams(warp_across_list(full_journey, forwards));
    // failsafe to patch up any math issues...
    integer last_indy = 0;
    if (forwards == TRUE) last_indy = llGetListLength(full_journey) - 1;
    
    // pick out the last target in the list based on the direction we're moving.
    vector last_jump = (vector)llList2String(full_journey, last_indy);
    integer max_attempts = 3;  // a rough guess at most adjustments we'd ever need.
    while ( (llVecDist(llGetPos(), last_jump) > .001) && (max_attempts-- > 0) ) {
////llWhisper(0, "touch up jump from " + (string)llGetPos());
        llSetPos(last_jump);
    }
}

//end borrowed from jaunt
//////////////

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

//////////////

// globals that record parameters during runtime.

vector last_victim = <0.0, 0.0, 0.0>;
    // the last place we collided or sensed something and decided to check out.

vector previous_velocity;
    // the rate the object was going when it started to hit something.

// the steps taken when the bullet itself dies.
dying_scene()
{
    // make fire appear again.
    llMakeExplosion(20, 1.0, 5, 3.0, 1.0, "fire", ZERO_VECTOR);
    // replay the explosion noise, for good measure
    llTriggerSound("Explosion", 1.0);
    // make the smoke visible.  it would technically be faster than the sound, but
    // the call also pauses the script.
    llMakeExplosion(20, 1.0, 5, 3.0, 1.0, "Smoke", ZERO_VECTOR);
    // gone!
    llDie();
}

fling_at(vector location)
{
    if (location == <0.0, 0.0, 0.0>) return;
    vector proper_direction = location - llGetPos();
    set_velocity(100 * proper_direction, FALSE);
}

jaunt_to_location(vector location)
{
    if (location == <0.0, 0.0, 0.0>) return;
    // this turns off the physics property for the object, so that jaunt and
    // llSetPos will still work.  this became necessary due to havok4.
    llSetStatus(STATUS_PHYSICS, FALSE);
    llSetStatus(STATUS_PHANTOM, TRUE);
    // go to position specified.
    jaunt([llGetPos(), location], TRUE);
    // return to prior characteristics.
    llSetStatus(STATUS_PHANTOM, FALSE);
    llSetStatus(STATUS_PHYSICS, TRUE);

//simplistic version.
////    llSetPos(location);
}

blow_up_item(key to_whack, integer type)
{
    if (to_whack == NULL_KEY) {
        llOwnerSay("wtf?  passed a null key in blow up item.");
        return;
    }
    list details = llGetObjectDetails(to_whack, [ OBJECT_POS ]);
    vector current_victim = llList2Vector(details, 0);
///    fling_at(current_victim);
    jaunt_to_location(current_victim - <0.0, 0.0, 0.042>);
    if (type & AGENT) {
        llOwnerSay("targeting: " + llKey2Name(to_whack));
        // flame first.
        llMakeExplosion(20, 1.0, 5, 3.0, 1.0, "fire", ZERO_VECTOR);
        // reset the last victim for a *real* victim.
        last_victim = current_victim;
    }
//fling_at(last_victim);//trying new approach.
    // then a huge push.
    llPushObject(to_whack, push_magnitude * llRot2Up(llGetRot()), ZERO_VECTOR, FALSE);
    // then the noise, if the victim is important enough.
    if (type & AGENT) {
        // make a glad noise.  sound is slower, right?
llSetStatus(STATUS_PHYSICS, FALSE);//guess temp.
        llTriggerSound("Explosion", 1.0);
llSetStatus(STATUS_PHYSICS, TRUE);//guess temp.
    }
}

reset_to_last_hit()
{
   jaunt_to_location(last_victim);  // start back where we first collided.
    // onward at nearly the same rate, but backwards.
    vector adjusted = <llFrand(MAX_VEL_ADJUST), llFrand(MAX_VEL_ADJUST),
        llFrand(MAX_VEL_ADJUST)>;
    set_velocity(-previous_velocity + adjusted, FALSE);
    last_victim = <0.0, 0.0, 0.0>;  // allow collisions to start being noticed again.
 }

integer blow_up_avatars(integer total_number)
{
//    fling_at(last_victim);
    jaunt_to_location(last_victim);

//if (last_victim != <0.0, 0.0, 0.0>) {
//full_stop();
//// check where it actually thinks we hit.
//integer i;
//for (i = 0; i < 20; i++) 
//llMakeExplosion(20, 1.0, 5, 3.0, 1.0, "fire", ZERO_VECTOR);   
//}
    
    integer found_avatar = FALSE;

    // now whack the avatars.    
    integer i;
    for (i = 0; i < total_number; i++) {
        // if we hit an avatar, slam them away.
        key curr_key = llDetectedKey(i);
        if (curr_key != NULL_KEY) {
            // why would the key ever be null?  yet it seems we have seen that.
            integer type = llDetectedType(i);
            // make sure the target is an avatar and it's NOT the owner.
            if (curr_key != llGetOwner()) {
                // blow them up regardless of what type they are.
                blow_up_item(curr_key, type);
                // set that we found an avatar if the type is right.
                if (type & AGENT) found_avatar = TRUE;
            }
        }
    }

    if (!found_avatar) {
        reset_to_last_hit();
        return FALSE;  // don't die right now.  we didn't achieve our objective.
    }

    dying_scene();
    return TRUE;  // i'm not sure we'll ever see this statement...
}

integer enabled = FALSE;    // true if bullet is ready to operate.

integer hits_used = 0;  // number of hits consumed so far on sensors.

prepare_bullet()
{    
///    llSensorRemove();

//    llSetDamage(5000);  // hang onto that.
//what do they mean by "task will be killed"?

    // pick the object characteristics that seem to work best.
    llSetStatus(STATUS_PHYSICS, TRUE);
    llSetStatus(STATUS_PHANTOM, FALSE);

    hits_used = 0;
    last_victim = <0.0, 0.0, 0.0>;
}

limit_obnoxiousness()
{
    hits_used++;
    if (hits_used > MAXIMUM_HITS) dying_scene();
}

default
{
    state_entry() { if (llSubStringIndex(llGetObjectName(),  "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default {
    state_entry() {
        auto_retire();
        llPreloadSound("Explosion");
        prepare_bullet();
    }

    on_rez(integer start_param) {
        // make this a bit more specific so we can work on it.
        if (start_param == 2814) {
            prepare_bullet();
            enabled = TRUE;
        }
    }
    
    collision_start(integer total_number)
    {
        if (!enabled) return;
        // initialize our boom position if it hasn't been yet.
        if (last_victim == <0.0, 0.0, 0.0>) {
            previous_velocity = llGetVel();
            last_victim = llGetPos();
//            reverse_velocity(FALSE);
            // sense up any bogies.
            llSensor("", NULL_KEY, AGENT | ACTIVE, sense_distance, 2.0 * PI);
        }
    }

    sensor(integer total_number) {
        if (!enabled) return;
        limit_obnoxiousness();
        // act on who we sensed around us.
        integer found_one = blow_up_avatars(total_number);
        // did we find an avatar?  that makes us explode with pleasure.
        if (!found_one) {
            // start looking again.
            llSensor("", NULL_KEY, AGENT | ACTIVE, sensory_addition + sense_distance, 2.0 * PI);
        } else {
            // we had seen one, so croak.
            dying_scene();
        }
    }

    no_sensor() {
        if (!enabled) return;
        limit_obnoxiousness();
        // try again until we find a victim.
        reset_to_last_hit();
        llSensor("", NULL_KEY, AGENT | ACTIVE, 2.0 * sensory_addition
            + sense_distance, 2.0 * PI);            
    }
}

