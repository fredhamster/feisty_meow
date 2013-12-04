
// huffware script: simple follower script, by fred huffhines.
//
// this has been condensed from other pet scripts but still retains all the vitamins!
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

// global constants...

float HEIGHT_ABOVE_AVATAR = 0.3;  // how far above the av do we float.
float ROOM_FOR_ERROR = 0.1;  // how far away from the target locale can we roam.
float MAXIMUM_VELOCITY = 200.0;  // the fastest the object can zoom towards us.
float SENSOR_PERIOD = 1.0;  // how frequently we look for the avatar.
float SENSOR_RANGE =  64.0;  // the farthest away we will try to look for the avatar.

// global variables...

vector last_detected_position;  // where the avatar was last detected.
integer target_identifier;  // the id assigned when we registered our target.
integer last_physics_state;  // this remembers if physics should be enabled currently.
vector perch_position;  // place we inhabit near the owner.    

// returns the location where we should perch, given the target's location.
vector target_to_perch(key av, vector target)
{
    vector av_bounds = llGetAgentSize(av);
    return <target.x, target.y, target.z + av_bounds.z / 2.0 + HEIGHT_ABOVE_AVATAR>;
}

// returns the location of the target given where we're trying to aim at.
vector perch_to_target(key av, vector perch)
{
    vector av_bounds = llGetAgentSize(av);
    return <perch.x, perch.y, perch.z - av_bounds.z / 2 - HEIGHT_ABOVE_AVATAR>;
}
//hmmm: the two target and perch funcs both assume we don't want any lateral offset.
//      and currently they're only taking into account the avatar's size in the z direction.


// makes the pet completely stop travelling around and just sit there.
cease_movement()
{
    llStopMoveToTarget();
    llTargetRemove(target_identifier);
    full_stop();        
    llSetStatus(STATUS_PHYSICS, FALSE);
    last_physics_state = FALSE;
    // if we're not quite there, jump to the perch.
    if (llVecDist(perch_position, llGetPos()) > ROOM_FOR_ERROR) {
        // we're not close enough so make a flying leap.
//llOwnerSay("jumping to perch at " + (string)perch_position + ", was at " + (string)llGetPos());
        llSetPos(perch_position);  // make us very accurate until something changes.
    }
}

// tells the pet to target the avatar "av" at the location "pos".  this will assume
// the avatar is actually present in the same sim when it calculates the avatar's size.
// we use that size in calculating where to perch nearby the avatar.
move_toward_target(key av, vector destination)
{
    // first of all we'll remember where we're supposed to go.
    perch_position = target_to_perch(av, destination);
    // now see how far away we are from that.
    float distance = llVecDist(perch_position, llGetPos());
    if (distance < 1.0) {
        // we're close enough; stop moving for a bit.
//llOwnerSay("dist small enough to go non-phys: " + (string)distance);
        cease_movement();
        return;
    }
    // well, now we know that we need to move somewhere.  let's set up a
    // physics target and head that way.
    llSetStatus(STATUS_PHYSICS, TRUE);
    last_physics_state = TRUE;
    llTargetRemove(target_identifier);
    float time = distance / MAXIMUM_VELOCITY;
    // if we go too low, then SL will ignore the move to target request.
    if (time < 0.14) time = 0.14;
//llOwnerSay("tau=" + (string)time);
    target_identifier = llTarget(perch_position, ROOM_FOR_ERROR);
    llMoveToTarget(perch_position, time);
}

// startup the object.
initialize()
{
    llSetStatus(STATUS_PHYSICS, TRUE);
    last_physics_state = TRUE;
    llSetBuoyancy(1.0);
    llSetStatus(STATUS_PHANTOM, TRUE);
    llSensorRemove();
    // start looking for the owner.
    llSensorRepeat("", llGetOwner(), AGENT, SENSOR_RANGE, PI * 2, SENSOR_PERIOD);
}

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

default {
    state_entry() { if (llSubStringIndex(llGetObjectName(), "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default
{
    state_entry() { initialize(); }

    on_rez(integer param) { llResetScript(); }

    sensor(integer num_detected)
    {
        // save where we saw the av just now.
        last_detected_position = llDetectedPos(0);
        // move closer if we're not near enough to our beloved owner.
        move_toward_target(llGetOwner(), last_detected_position);
        // if we find that our rotation is incorrect, we'll fix that here.
        // we only reorient on the sensor period, since that's slower
        // than hitting our target.  plus we try to ensure we never get
        // out of place again.
        vector curr_rot = llRot2Euler(llGetRot());
        if ( (curr_rot.x != 0.0 ) || (curr_rot.y != 0.0) ) {
            // we are out of rotational goodness right now even.  fix that.
            llSetStatus(STATUS_PHYSICS, FALSE);
            llSetStatus(STATUS_ROTATE_X, FALSE);
            llSetStatus(STATUS_ROTATE_Y, FALSE);
            // save the z value before correcting the rotation.
            vector new_rot = ZERO_VECTOR;
            new_rot.z = curr_rot.z;
            llSetRot(llEuler2Rot(new_rot));
            llSetStatus(STATUS_PHYSICS, last_physics_state);
        }
    }

    no_sensor()
    {
        // we lost track of the avatar.  turn off phyics and wait.
        cease_movement();
    }

    at_target(integer number, vector targetpos, vector ourpos)
    {
        // wait until we see the av again before picking a new target.
///??seems bad        cease_movement();
    }
    
    not_at_target()
    {
    }
}
