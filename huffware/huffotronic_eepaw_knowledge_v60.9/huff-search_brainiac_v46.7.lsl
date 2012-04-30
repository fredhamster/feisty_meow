﻿
// huffware script: huff-search brainiac, by fred huffhines
//
//    this script is one portion of a search system.  it is the brain for an object
// that can find nearby objects by a partial name match.  this script should be
// located in the root primitive.  it also requires an up-to-date jaunting library
// in the same primitive.  see the (hopefully) enclosed documentation for more details.
//    more implementation notes are stored at the back of the file.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

// global settings that can be very useful to change...

integer MAX_MATCHES = 17;
    // by default we try to match one object per rod so we can point at them.

float MAX_SPIRAL_RADIUS = 72.0;
    // the farthest that the object will travel from its home while searching.
    // this is an important factor in the searchbert's range of vision; the other is
    // the sensor range, below.
    // ensure that it has access to the lands within this range or it might get stuck.
    // you can also set the TRAVERSE_LANDS flag to false to make for safer searches when
    // surrounded by a lot of banned lands.

float SENSOR_MAX_RANGE = 96.0;
    // the maximum range that we try to sense objects at.  this is distinct from the spiral
    // radius because often the range of movement is constrained by land conditions, where
    // one might still want a large sensor radius to see as many things as possible in range.
    // note though that searches will keep matching the things nearest to them if neither
    // the position nor the angle nor object positions have changed.

float SPIRAL_LOOPS = 7;
    // how many loops there are in the spiral.  this is measured by how many times the
    // spiral cuts across the positive x-axis (that is, the zero angle vector) from
    // the center of the spiral to the radius (that is, from <0, 0, 0> to <radius, 0, 0>).

integer TOTAL_STEPS = 42;
    // how many positions within the spiral will the object travel to?  the first step
    // is step 0, which is the center of the spiral.  positions 1 through TOTAL_STEPS
    // spiral outward from the zero position, where the distance from the center at
    // TOTAL_STEPS should be MAX_SPIRAL_RADIUS or less.
    
integer MAX_SPIRAL_TRAVERSALS = 1;
    // how many different spirals are we allowed for the same search?  this will let the
    // search engine cover more ground than with just one spiral, to allow different
    // objects to be exposed.

integer HUFF_SEARCH_CHAT_CHANNEL = 0;
    // the channel where the object listens for commands.  the default is to use open
    // chat for this, but some people may want it more specific.

// this flag is important to keep as false for wright plaza and other areas with script traps
// (where the perms allow objects to enter, but then their scripts are stopped dead).
integer TRAVERSE_LANDS = FALSE;
    // if this is false, then the searchbert will stick to the land owner's land where it started.
    // but if it's true, then searchbert will cross into other people's lands too, but if there
    // are some weird border or permission conditions about object re-entry or scripts being able
    // to run, that can be quite problematic.

integer TRAVELLING_UPWARDS = FALSE;
    // if this is true, the searchbert will climb in the vertical direction also.

float MAX_UPWARDS_DISTANCE = 30.0;
    // the farthest away that the searchbert will fly during an upwards search.

float SEARCH_ROD_ANGULAR_SWEEP = PI_BY_TWO;
    // angular arc of the sensor cone, placeholder.

//////////////

// constants that are not configurable in a notecard...

integer DEBUGGING = FALSE;  // set to true to make the script noisier.

//////////////

// global constants that aren't as useful to change...

float MINIMUM_HEIGHT_ABOVE_GROUND = 0.14;
    // the closest we allow the searcher to get to the ground.  any lower and our search
    // rods might be dragging around under ground.

float FAST_TIMER_INTERVAL = 0.12;
    // the time between jumps when we're doing our search spiral.  this is the fastest
    // the object can spin around its spiral trajectory, but it's more limited than by
    // just this number; llSetPrimitiveParams is used for jaunting and it has a delay of
    // 0.2 seconds built into it.

integer NORMAL_TIMER_INTERVAL = 14;
    // how frequently we check for things to do, in seconds.

float STARTUP_TIME_ALLOWED = 50.0;  // number of seconds before declaring a timeout.

integer MAX_STEP_SNOOZES = 81;
    // number of timer hits to allow before giving up on jaunt.

float SNOOZE_BEFORE_RESET = 300;
    // number of seconds before a scan will automatically reset.  hopefully this is
    // enough time for the user to follow the trail to the detected object.
    
// imported interfaces below...

// huff-search pointer API:
//////////////
// do not redefine these constants.
integer HUFF_SEARCH_POINTER_HUFFWARE_ID = 10032;
    // the unique id within the huffware system for this script.
string HUFFWARE_PARM_SEPARATOR = "{~~~}";
    // this pattern is an uncommon thing to see in text, so we use it to separate
    // our commands in link messages.
string HUFFWARE_ITEM_SEPARATOR = "{|||}";
    // used to separate lists of items from each other when stored inside a parameter.
    // this allows lists to be passed as single string parameters if needed.
integer REPLY_DISTANCE = 100008;  // offset added to service's huffware id in reply IDs.
//////////////
string HUFF_SEARCH_RESET = "#reset#";
    // returns the script to its starting state.
string HUFF_SEARCH_POINT_PARTY = "#point_particles#";
    // aim at an object and show a particle stream leading to it.
string HUFF_SEARCH_JUST_POINT = "#just_point#";
    // aim at an object, but don't do any particles.
string HUFF_SEARCH_SENSEI = "#sensor#";
    // set up a sensor request for a search pattern.  pings will cause
    // the pattern to be sought in names of nearby objects.  the parameters are:
    // (1) the maximum range for the sensor, (2) the arc angle to use in sensing,
    // (3) the search pattern to look for in object names, (4) the maximum number
    // of matches to look for.
string HUFF_SEARCH_STOP_SENSE = "#stop_sensor#";
    // turn off the sensor but don't totally reset.
string HUFF_SEARCH_PING = "#ping#";
    // cause the searcher to actively sensor ping the targets.
string HUFF_SEARCH_MATCH_EVENT = "#match#";
    // fired at the root prim when matches are found for the search term.
    // the payload is a list of matched item pairs [item key, location].
//////////////

// the armature button pushing API.
// (we have subclassed the simple button pushing API for searchbert armature.)
//////////////
integer BUTTON_PUSHER_HUFFWARE_ID = 10029;
    // a unique ID within the huffware system for this script.
//////////////
string BUTTON_PUSHED_ALERT = "#btnp";
    // this event is generated when the button is pushed.  the number parameter will be
    // the huffware id plus the reply distance.  the id parameter in the link message will
    // contain the name of the button that was pushed.
//////////////
string CHECK_ARMS_BUTTON_NAME = "^checkarms";
    // this is the signal given to the armature script that it should check the
    // number of arms present on searchbert.  if the number is fine, it will
    // push the arms are good button back at the brainiac (using the button
    // push api plus reply distance).
string ARMS_ARE_GOOD_BUTTON_NAME = "^goodarmz";
    // the event sent back by the searchbert armature when all arms are ready
    // to go.
string PROBLEM_WITH_MY_THUMBS_BUTTON_NAME = "^ouch";
    // a problem was noticed with the number of arms and we could not fix it.
    // the brain needs to try reconfiguring again.
//////////////

// searchbert menus API.
//////////////
// do not redefine these constants.
integer SEARCHBERT_MENUS_HUFFWARE_ID = 10034;
    // the unique id within the huffware system for this script.
//////////////
string SM_CONFIGURE_INFO = "#sm-info#";
    // sets important information this script will use, such as (1) the channel for listening.
string SM_POP_MAIN_MENU_UP = "#sm-main#";
    // causes the main menu to be displayed.  this requires an avatar name and avatar key for the
    // target of the menu.
//////////////
string SM_EVENT_MENU_CLICK = "#sm-clik#";
    // the user has requested a particular menu item that this script cannot fulfill.  the
    // event is generated back to the client of this script for handling.  it will include (1) the
    // menu name in question, (2) the item clicked, (3) the avatar name, and (4) the avatar key.
//////////////

// card configurator link message API:
//////////////
// do not redefine these constants.
integer CARD_CONFIGURATOR_HUFFWARE_ID = 10042;
    // the unique id within the huffware system for the card configurator script to
    // accept commands on.  this is used in llMessageLinked as the num parameter.
//////////////
string BAD_NOTECARD_TEXT = "*badcrd*";
    // the sign that we hated the notecards we found, or there were none.
string FINISHED_READING_NOTECARDS = "**finished**";
    // the sign that we are done plowing through the card we found.
string BEGIN_READING_NOTECARD_COMMAND = "#read-cfg#";
    // requests that the configurator find a good notecard and read its contents.
    // it should send the contents via the alert below.  first parm is the signature and
    // second is the wrapped list of valid item prefixes.
string READ_PARTICULAR_NOTECARD_COMMAND = "#read-note#";
    // requests that the configurator find a good notecard and read its contents.
    // it should send the contents via the alert below.  first two parms are the same as
    // begin reading notecard, and the third parameter is the name of the specific notecard.
string CARD_CONFIG_RECEIVED_ALERT = "#cfg-event-upd#";
    // this message is sent when the configurator has found some data updates or has finished
    // reading the configuration file.
//////////////

// jaunting library API:
//////////////
// do not redefine these constants.
integer JAUNT_HUFFWARE_ID = 10008;
    // the unique id within the huffware system for the jaunt script to
    // accept commands on.  this is used in llMessageLinked as the num parameter.
//////////////
// commands available via the jaunting library:
string JAUNT_COMMAND = "#jaunt#";
    // command used to tell jaunt script to move object.  pass a vector with the location.
//string FULL_STOP_COMMAND = "#fullstop#";
    // command used to bring object to a halt.
//string REVERSE_VELOCITY_COMMAND = "#reverse#";
    // makes the object reverse its velocity and travel back from whence it came.
//string SET_VELOCITY_COMMAND = "#setvelocity#";
    // makes the velocity equal to the vector passed as the first parameter.
//string JAUNT_UP_COMMAND = "#jauntup#";
//string JAUNT_DOWN_COMMAND = "#jauntdown#";
    // commands for height adjustment.  pass a float for number of meters to move.
//string JAUNT_LIST_COMMAND = "#jauntlist#";
    // like regular jaunt, but expects a string in jaunt notecard format with vectors.
    // the second parameter, if any, should be 1 for forwards traversal and 0 for backwards.
//
//////////////

// global variables used in the script.

// configuration variables...
integer all_setup_finished = FALSE;  // have we read our configuration yet?
integer configuration_pending = FALSE;  // true if the setup process (config and arms) is still going on.
integer listening_handle = 0;  // tracks our handle for listening to commands.

// matching variables...
string global_target_name = "";  // the name of the object being sought.

list global_matches_found;  // a list of keys that match the specified search terms.
list global_positions_found;  // matches list of positions for the detected objects.

// jaunting variables...

vector global_home_posn;  // the location where the search bot is located.
vector last_safe_target;  // the last place we jaunted to that was safe.

integer next_step_snoozes_left;  // pauses allowed before deciding jaunt will not respond.

integer jaunt_responses_awaited = 0;  // true when a jaunt is pending.
integer last_jaunt_was_success = FALSE;  // result of jaunting received by message.

vector current_rotation = <0.0, 0.0, 0.0>;
  // the current rotation around each axis for object.

// spiral variables...

integer global_current_step = 0;  // the current position in the search spiral.
float spiral_start_angle = 0.0;  // how many radians to offset spiral by for this pass.
integer global_current_pass = 0;  // which spiral number are we working on?

integer tried_jump_homeward = FALSE;
    // when retracing to home, this records if we already tried the jump home that should
    // come in between the spiral jaunt.

integer retracing_steps = FALSE;
    // this is true when we are trying to get back home after finishing our spirals.

// object maintenance variables...

integer reset_for_next_timer = FALSE;  // true when next timer hit should do a reset.

integer running_a_search = FALSE;  // true if the object is seeking matches.

//////////////

// constants that should not be messed with.

float MAX_SLACK_DISTANCE = 0.01;  // how close we need to be to a target.

integer ALL_SEEKER_ALERT = -1;  // communicates with all search rods.

integer last_time_ordered_stop_sensing = 0;
    // tracks when we last tried to order the search rods to stop sensing matches.

// jaunting variables...

vector last_jaunt_target;
    // where we're currently headed.  this variable is mostly for record keeping, so we want
    // to update it whenever we jaunt someplace.

//////////////

// main functions for getting work done as the searchbert...

// pointing that doesn't use particles, but just aims a search rod.
aim_at_position(integer which_seeker, vector targetPosition) {
    list paramList = [targetPosition];
    request_from_seeker(which_seeker, llDumpList2String(paramList,
        HUFFWARE_PARM_SEPARATOR), HUFF_SEARCH_JUST_POINT);
}

// tells the rod at "which_seeker" link to do a sensor scan.
start_sensing(integer which_seeker, float max_range, float arc_angle, string search_pattern,
    integer matches_sought)
{
    list paramList = [max_range, arc_angle, search_pattern, matches_sought];
    request_from_seeker(which_seeker, llDumpList2String(paramList,
        HUFFWARE_PARM_SEPARATOR), HUFF_SEARCH_SENSEI);
}

// tells all of the seeker search rods to stop sensing matches.
stop_sensing()
{
    // make sure that we are not just blasting this order over and over.
    if (llGetUnixTime() != last_time_ordered_stop_sensing) {
        last_time_ordered_stop_sensing = llGetUnixTime();
        request_from_seeker(ALL_SEEKER_ALERT, "", HUFF_SEARCH_STOP_SENSE);
    }
}

// tell the seeker arms to find what we asked them about.
request_ping() { request_from_seeker(ALL_SEEKER_ALERT, "", HUFF_SEARCH_PING); }

//////////////

// teleports to the vector specified.  this invokes a method in the jaunting library
// which will return its result asynchronously.  thus we can't just expect that we
// have arrived at the target by the end of this function; we need to get back the
// IPC message in linked_message.
jaunt_to_target(vector target)
{
    if (!TRAVERSE_LANDS) {
        // this is an important restriction since we don't want to get trapped in weird
        // land perm screwups, like not being able to re-enter the land where we started,
        // if that parcel option is set.
        if (llGetLandOwnerAt(llGetPos()) != llGetLandOwnerAt(target)) {
            return;
        }
    }
    
    // reset our snoozer count, since we're doing a new jaunt now.
    next_step_snoozes_left += MAX_STEP_SNOOZES;
    jaunt_responses_awaited++;
    // record the current target.
    last_jaunt_target = target;

    float minimum_rod_angle_addition = 0.2;
    float maximum_rod_angle_addition = 10.8;
    // add small random amounts to the current rotation to expose more matches.
    current_rotation +=
        <(minimum_rod_angle_addition + llFrand(maximum_rod_angle_addition)) * DEG_TO_RAD,
        (minimum_rod_angle_addition + llFrand(maximum_rod_angle_addition)) * DEG_TO_RAD,
        (minimum_rod_angle_addition + llFrand(maximum_rod_angle_addition)) * DEG_TO_RAD>;
    rotation new_rot = llEuler2Rot(current_rotation);
//    log_it("new rot to " + (string)new_rot + " based on " + (string)current_rotation);
    llMessageLinked(LINK_THIS, JAUNT_HUFFWARE_ID, JAUNT_COMMAND,
        (string)target + HUFFWARE_PARM_SEPARATOR + (string)new_rot);
}

// provides the position on a spiral that has "loops" total loops (counted by
// how many times a line from the center to the outer "radius" is crossed).
// this provides an x and y offset from 0,0 for where the spiral should be if
// one were to walk it in "total_steps" (where the center is step 0).  this is
// given for the current "step" one is at.  the "start_angle" is the number of
// radians to start at for the spiral, to allow different areas to be traversed.
vector trace_spiral(float loops, float radius, integer total_steps, integer step,
    float start_angle)
{
    float total_radians = TWO_PI * loops;
    float angle_per_step = total_radians / (float)total_steps;
    float distance = radius / (float)total_steps * (float)step;
    float current_angle = start_angle + angle_per_step * (float)step;
    float current_z = 0;
    if (TRAVELLING_UPWARDS) {
        // add in the vertical distance for this step.
        current_z = MAX_UPWARDS_DISTANCE / total_steps * step;
    }
    return <distance * llCos(-current_angle), distance * llSin(-current_angle), current_z>;
}

// this describes the list of matches in local chat.
chat_about_matches()
{
    integer len = llGetListLength(global_matches_found);
    if (!len) {
        llSay(0, global_target_name + " not found within a distance of "
            + (string) (SENSOR_MAX_RANGE + MAX_SPIRAL_RADIUS) + " meters from here.");
        return;
    }
    string addition;
    if (len != 1) addition = "es";
    string match_description = (string)len + " match" + addition + " for search "
        + "pattern \'" + global_target_name + "\':\n";
    integer i;
    // only show a maximum number of matches as we have seeker objects.
    if (len > MAX_MATCHES) len = MAX_MATCHES;
    for (i = 0; i < len; i++) {
        if (llStringLength(match_description) > 350) {
            llSay(0, "\n" + match_description);
            match_description = "";
        } else if (i != 0) match_description += "\n";
        key targetKey = llList2Key(global_matches_found, i);
        vector targetPos = llList2Vector(global_positions_found, i);
        match_description += llKey2Name(targetKey)
            + " @ " + vector_to_string(targetPos)
            + " [" + (string) targetKey + "]";
    }
    llSay(0, "\n" + match_description);
}

// once we accumulate a set of matches, we want to show them off.
show_matches()
{
    llSetRot(llEuler2Rot(ZERO_VECTOR));  // set the object to point at the zero vector.    
    reset_rod(ALL_SEEKER_ALERT);  // clean up any current pointing first.
    chat_about_matches();  // say where the matches are.
    
    integer i;
    integer which_locater = 0;  // which pointer to use.
    // only show a maximum number of matches as we have seeker objects.
    integer len = llGetListLength(global_matches_found);
    if (len > MAX_MATCHES) len = MAX_MATCHES;
    for (i = 0; i < len; i++) {
        key targetKey = llList2Key(global_matches_found, i);
        vector targetPos = llList2Vector(global_positions_found, i);
        point_at_with_particles(which_locater++, targetKey, targetPos);
    }
}

// makes sure that a target we are given is above ground.  this used to do a
// lot more checking, but really being above ground is the most important thing
// for us to check at this level of the code.
vector clean_target(vector to_clean)
{
    vector to_return = to_clean;
    float ground_height = llGround(to_return - llGetPos());
//log_it("ground height here: " + (string)ground_height);

    // we'll adjust the basic ground height by adding in the object's height.
    list bounds = llGetBoundingBox(llGetKey());
//hmmm: we have found a bug in opensim in that the bounding box only covers the root prim.
// this needs to be reported.
    vector min = llList2Vector(bounds, 0);
    vector max = llList2Vector(bounds, 1);
//log_it("calcd: min=" + (string)min + " max=" + (string)max);

//real math    float distance_to_add = llVecDist(<0, 0, min.z>, <0, 0, max.z>) / 2.0;
//below is bogus math for opensim currently.
    float distance_to_add = llVecDist(<0, 0, min.z>, <0, 0, max.z>) * 1.1;
    
//log_it("distance_to_add: " + (string)distance_to_add);
    if (to_return.z - MINIMUM_HEIGHT_ABOVE_GROUND <= ground_height + distance_to_add) {
        // patch up the height to be above ground.
        to_return.z = ground_height + distance_to_add + MINIMUM_HEIGHT_ABOVE_GROUND;
    }
    return to_return;
}

// shows all the matches by pointing our seekers at them and emitting a targeted
// trail of particles.  this method should only be called after the final jaunt
// back to home has occurred.
show_off_what_was_found()
{
    // show the positions of what was found.
    show_matches();
    llSetTimerEvent(SNOOZE_BEFORE_RESET);
    reset_for_next_timer = TRUE;
}

// stops the device from travelling around and asking for help from the search rods.
integer finish_spiral_traversal()
{
//log_it("got to finish spiral...");
    stop_sensing();  // turn off all sensors.

    // are we close enough to home to declare victory?
    if (llVecDist(llGetPos(), global_home_posn) > MAX_SLACK_DISTANCE) {
        if (!tried_jump_homeward) {
            // there are enough here; we completed early.
            tried_jump_homeward = TRUE;
//log_it("finish spiral sees us as too far from home, jaunting there.");
            jaunt_to_target(global_home_posn);
            return FALSE;
        } else {
            // we already tried a jump home.  dang, let's try spiraling.
//log_it("finish spiral failed jaunt home, continuing spiral.");
            tried_jump_homeward = FALSE;  // reset that we tried this.
            return TRUE;
        }
    }

//log_it("finish spiral success, showing off matches");
    
    // this object has done enough spiraling.
    llSetTimerEvent(NORMAL_TIMER_INTERVAL);  // turn off jump timer.
    running_a_search = FALSE;  // reset our state since the search is done.
    show_off_what_was_found();
    return FALSE;
}

// our timer handling function; this will move the search engine to the next place on
// the current spiral (or to the start of the next spiral) when it is called.
// if we're not currently running a search, then it will just periodically update the
// sub-prims to make sure they have the latest versions.
handle_timer()
{
    if (!all_setup_finished) {
        complain_about_config(!configuration_pending);
        return;  // should not get here.
    }
    
    if (reset_for_next_timer) {
        // this timer intends a reset so we don't just keep pointing forever.
        llSay(0, "Timed Out: resetting matches now.");
        reset_search();
        return;
    }

    if (!running_a_search) {
//log_it("exiting timer since not searching.");
        return;
    }

//llSay(0, "next search move!");
            
    // make sure we don't allow too many pauses while awaiting a jaunt completion.
    if (next_step_snoozes_left-- <= 0) {
        log_it("waiting to arrive failed!  ran out of snoozes trying to get to "
            + (string)last_jaunt_target);
    } else if (jaunt_responses_awaited) {
        // not ready to do next step yet.
        return;
    }
    if (next_step_snoozes_left < 0) {
//log_it("got below zero in next step snoozes!");
        next_step_snoozes_left = 0;
    }
    
    if (llGetListLength(global_matches_found) < MAX_MATCHES) {
        // tell the sensor arms to look for stuff right here.
        request_ping();
    }

    // make sure we're not already done finding enough objects.
    if (llGetListLength(global_matches_found) >= MAX_MATCHES) retracing_steps = TRUE;
    
    // check whether we're trying to get home again, rather than moving forward.
    if (retracing_steps) {
//log_it("retrace steps in timed move.");
        integer keep_going = finish_spiral_traversal();
        if (!keep_going) return;
    }

    // see if the object has reached the end of its tether and should report
    // home with results.  we do this by taking a spiral pass outwards from home
    // and a return pass to get back to home.  if we've used up all our spiral
    // traversals, then it's time to stay home and show the matches.
    integer done_with_this_spiral = FALSE;
    if (global_current_pass % 2 == 0) {
        // even passes are outward bound ones.  we're done when we've gotten to TOTAL_STEPS.
        if (global_current_step++ > TOTAL_STEPS) done_with_this_spiral = TRUE;
    } else {
        // odd passes are for returning to home.  those are done when they hit zero.
        if (global_current_step-- <= 0) done_with_this_spiral = TRUE;
    }
    
    if (done_with_this_spiral) {
        // now evaluate whether we're totally done or just need to move to next spiral,
        // whether inward or outward.
        if (++global_current_pass < 2 * MAX_SPIRAL_TRAVERSALS) {
            // just step to the next spiral.
//log_it("moving to next spiral");
            if (global_current_pass % 2 == 0) {
                // even passes are outward bound ones.
                global_current_step = 0;
                // add some different angular rotation to get more matches for this spiral.
                spiral_start_angle += TWO_PI / (MAX_SPIRAL_TRAVERSALS + 1);
            } else {
                // odd passes are for returning to home.
                global_current_step = TOTAL_STEPS;
            }
//hmmm: not currently resetting that angle for next search.
        } else {
//log_it("done with spiral mode, now retracing steps.");
            // we finished the search pattern.
            retracing_steps = TRUE;
            global_current_pass--;  // try spiraling back.
            global_current_step = TOTAL_STEPS;
            return;
        }
    }        

    // normal activity here--pick the next place on the spiral to look for objects.        
    vector new_posn = trace_spiral(SPIRAL_LOOPS, MAX_SPIRAL_RADIUS,
        TOTAL_STEPS, global_current_step, spiral_start_angle);
//log_it("next step " + (string)global_current_step + " to " + (string)new_posn);        
    // go to that location now, or rather, when the jaunting library gets the message.
    vector new_target = clean_target(new_posn + global_home_posn);
    jaunt_to_target(new_target);
}

// makes sure we are ready to run.  TRUE is returned if we're good.
integer test_health()
{
    if (!configuration_pending && !all_setup_finished) {
        llSay(0, "Configuration is not good yet; retrying.");
        return FALSE;
    }
    return TRUE;
}

// deals with the jaunter telling us about a completed jump.
process_jaunt_response(string str)
{
    jaunt_responses_awaited--;  // one less response being awaited.
    if (jaunt_responses_awaited < 0) {
        if (DEBUGGING)
            log_it("erroneously went below zero for jaunt responses!");
        jaunt_responses_awaited = 0;
    }
    // unpack the reply.
    list parms = llParseString2List(str, [HUFFWARE_PARM_SEPARATOR], []);
    last_jaunt_was_success = (integer)llList2String(parms, 0);
    vector posn = (vector)llList2String(parms, 1);
//log_it("got a reply for a jaunt request, success=" + (string)last_jaunt_was_success + " posn=" + (string)posn);
    if (last_jaunt_was_success) {
        last_safe_target = posn;
    } else {
        // we had a problem getting to the expected destination, so go to the last place we were
        // completely safely able to reach.  we should have no problem returning there, since that's
        // where we should have been coming from when we failed to reach the intended destination.
//log_it("bkwd retracing to " + (string)last_safe_target);
        jaunt_to_target(last_safe_target);
    }
}

reset_search()
{
    reset_for_next_timer = FALSE;

    llSetTimerEvent(NORMAL_TIMER_INTERVAL);  // back to slow timer hits.
    reset_rod(ALL_SEEKER_ALERT);  // reset the pointers to camber position.
    llSetRot(llEuler2Rot(ZERO_VECTOR));  // set the object to point at the zero vector.

    // set the home position to wherever we happen to be right now.
    global_home_posn = clean_target(llGetPos());
    llSetPos(global_home_posn);  // get us off the ground.
    last_safe_target = global_home_posn;

    global_current_step = 0;
    global_current_pass = 0;
    global_matches_found = [];
    global_positions_found = [];
    next_step_snoozes_left = 4;  // reset to small num, since this is additive.
    reset_for_next_timer = FALSE;
    running_a_search = FALSE;
    tried_jump_homeward = FALSE;
    retracing_steps = FALSE;
    jaunt_responses_awaited = 0;
    current_rotation = <0.0, 0.0, 0.0>;
    // fix a global 'constant' that can't be pre-initiatlized in LSL.
    SEARCH_ROD_ANGULAR_SWEEP = (PI_BY_TWO / 4.0);
        // PI_BY_TWO / 4 is 22.25 degrees, which when turned into a sensor cone
        // will be a range of 45 degrees around the zero vector.
    
}

initialize()
{
    llSay(0, "Initializing... this may take a few seconds.");
    auto_retire();
    llSleep(0.2);  // we ensure part of the claim above by waiting for sub-scripts to start.
    request_configuration("");
}

// complete the initialization once we know our configuration.
finish_initializing()
{
    all_setup_finished = TRUE;  // cancel the time-out checker.
    configuration_pending = FALSE;

    // in case we already have some arms, clean up their pointing states.
    reset_search();
    // set the position of rotation back to zero.
    llSetRot(<0.0, 0.0, 0.0, 1.0>);
    // listen for commands from our chat channel.
    listening_handle = llListen(HUFF_SEARCH_CHAT_CHANNEL, "", NULL_KEY, "");

    llSay(0, "Running and ready to search.  Touch for more instructions.");

    llSetTimerEvent(NORMAL_TIMER_INTERVAL);    
}

// this points the rods at all angles we possibly can to get the best coverage.
// if show_particles is true, then a demo mode is used that points at the positions
// that the rods are aiming at.
skew_rod_angles(integer show_particles)
{
    reset_rod(ALL_SEEKER_ALERT);
        // point everyone straight up for the moment.        
    float radius = 4.0;  // arbitrary distance of target we're aiming at.
    integer which_child;
    integer total_searchers = llGetNumberOfPrims() - 1;
    float angle_per_step = TWO_PI / (float)total_searchers;
    // use a circle for the x and y coordinates.
    for (which_child = 0; which_child < total_searchers; which_child++) {
        vector spinner = <
            radius * llCos((float)which_child * angle_per_step),
            radius * llSin((float)which_child * angle_per_step),
            radius * llCos((float)which_child * angle_per_step / 2.0)>;
        vector target_location = llGetPos() + spinner;
        if (!show_particles) {
            aim_at_position(which_child, target_location);
        } else {
            point_at_with_particles(which_child, NULL_KEY, target_location);
        }
    }
}

// if "noisy" is true, this says out loud that the searchbert is busy.
integer check_if_search_pending(integer noisy)
{
    if (running_a_search) {
        if (noisy) llSay(0, "A search is still pending.  Please wait for that to finish.");
        return TRUE;  // still working on a search.
    }
    return FALSE;  // not busy.
}

// processes a message coming back from a search rod or from the jaunter library.
handle_link_message(integer which, integer num, string msg, key id)
{
    if ( (num == CARD_CONFIGURATOR_HUFFWARE_ID + REPLY_DISTANCE)
            && (msg == CARD_CONFIG_RECEIVED_ALERT) ) {
        // the first element of the list will still be the notecard name.
        consume_configuration(llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []));
        return;
    }
    
    if ( (num == BUTTON_PUSHER_HUFFWARE_ID +  REPLY_DISTANCE)
            && (msg == BUTTON_PUSHED_ALERT) ) {
        if (id == ARMS_ARE_GOOD_BUTTON_NAME) {
            finish_initializing();
        } else if (id == PROBLEM_WITH_MY_THUMBS_BUTTON_NAME) {
            complain_about_config(TRUE);
        }
        return;
    }
    
    if (num == JAUNT_HUFFWARE_ID + REPLY_DISTANCE) {
        if (msg == JAUNT_COMMAND) {
            process_jaunt_response(id);
        }
        return;
    }
    
    list parms;
    
    if ( (num == SEARCHBERT_MENUS_HUFFWARE_ID + REPLY_DISTANCE) && (msg == SM_EVENT_MENU_CLICK) ) {
        parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
        process_menu_choice(llList2String(parms, 0), llList2String(parms, 2), llList2String(parms, 3),
            llList2String(parms, 1));
        return;
    }
    
    if (num != HUFF_SEARCH_POINTER_HUFFWARE_ID + REPLY_DISTANCE) return;  // not for us.
    if (msg != HUFF_SEARCH_MATCH_EVENT) return;  // also not for us.
        
    // make sure this is not already redundant, before doing a costly list search.
    if (llGetListLength(global_matches_found) >= MAX_MATCHES) {
        // we have enough already.
        stop_sensing();
        return;
    }
//hmmm: searchbert arms are still backwards.
    
//log_it("unpacking " + (string)llStringLength(id) + " byte string.");
    // fluff out the list back from the encoded string.
    parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
//log_it("received " + (string)llGetListLength(parms) + " element list from string.");

    while (llGetListLength(parms) > 1) {
        // unpack the match that one of our seekers found for us.
        key found = (string)llList2String(parms, 0);
        if (find_in_list(global_matches_found, found) < 0) {
            vector posn = (vector)llList2String(parms, 1);
            // that key wasn't already listed, so we can drop it in the list.
            global_matches_found += found;
            global_positions_found += posn;
        }
        // truncate the list by removing those two pieces we already handled.
        parms = llDeleteSubList(parms, 0, 1);
        if (llGetListLength(global_matches_found) >= MAX_MATCHES) {
            // we can bail out of the list processing now; we have enough already.
            parms = [];
        }
    }
}

// uses the configuration item as the value of one of our variables, if we can figure out
// the name involved.
apply_variable_definition(string var, string value)
{
//log_it("[" + var + "=" + value + "]");
    if (var == "max_matches") {
        MAX_MATCHES = (integer)value;
    } else if (var == "max_spiral_radius") {
        MAX_SPIRAL_RADIUS = (float)value;
    } else if (var == "sensor_max_range") {
        SENSOR_MAX_RANGE = (float)value;
    } else if (var == "huff_search_chat_channel") {
        // we have to hook up our ears to the new channel here.
        if (listening_handle) llListenRemove(listening_handle);
        HUFF_SEARCH_CHAT_CHANNEL = (integer)value;
        listening_handle = llListen(HUFF_SEARCH_CHAT_CHANNEL, "", NULL_KEY, "");
    } else if (var == "traverse_lands") {
        TRAVERSE_LANDS = (integer)value;
    } else if (var == "spiral_loops") {
        SPIRAL_LOOPS = (integer)value;
    } else if (var == "total_steps") {
        TOTAL_STEPS = (integer)value;
    } else if (var == "max_spiral_traversals") {
        MAX_SPIRAL_TRAVERSALS = (integer)value;
    } else if (var == "search_rod_angular_sweep") {
        SEARCH_ROD_ANGULAR_SWEEP = (float)value;
    } else if (var == "travelling_upwards") {
        TRAVELLING_UPWARDS = (integer)value;
    } else if (var == "max_upwards_distance") {
        MAX_UPWARDS_DISTANCE = (float)value;
    } else {
        if (DEBUGGING)
            log_it("unknown variable '" + var + "' tried to define value = " + value);
    }
}

// we have to consume the configuration in digestible chunks, since the pieces may
// be too large for sending in link messages.
consume_configuration(list config_chunk)
{
    string notecard_name = llList2String(config_chunk, 0);
    if (notecard_name == BAD_NOTECARD_TEXT) {
        complain_about_config(FALSE);
        return;
    } else if (notecard_name == FINISHED_READING_NOTECARDS) {
        // we're ready to make use of our new configuration now.
        if (DEBUGGING)
            log_it("Configuration has been read, need validation of limbs.");
        configuration_pending = FALSE;  // no longer waiting for notecard config.
        // configure the menu system to know what channel to talk about.
        llMessageLinked(LINK_THIS, SEARCHBERT_MENUS_HUFFWARE_ID, SM_CONFIGURE_INFO,
            wrap_parameters([HUFF_SEARCH_CHAT_CHANNEL]));
        // now check our arms to make sure they're all ready.
        llMessageLinked(LINK_THIS, BUTTON_PUSHER_HUFFWARE_ID, BUTTON_PUSHED_ALERT, CHECK_ARMS_BUTTON_NAME);
        return;
    }
    
    integer sandy;
    // scan the configuration items two at a time, but skip the notecard name at slot zero.
    for (sandy = 1; sandy < llGetListLength(config_chunk); sandy += 2) {
        string var = llList2String(config_chunk, sandy);
        string value = llList2String(config_chunk, sandy + 1);
        apply_variable_definition(var, value);
    }
    // clear the list out now that we've eaten its contents.
    config_chunk = [];
}

//////////////

// sends a message to the "which_seeker" search object.  the "parameters" should be a
// string-ized list of parameters.  the "command" is the specific action requested.
request_from_seeker(integer which_seeker, string parameters, string command)
{
    integer linkNumber = which_seeker + 2;
    if (which_seeker == ALL_SEEKER_ALERT) linkNumber = LINK_ALL_OTHERS;
    llMessageLinked(linkNumber, HUFF_SEARCH_POINTER_HUFFWARE_ID, command, parameters);
}

// aim the specified seeker rod at the object with the key and position.
point_at_with_particles(integer which_seeker, key targetId, vector targetPosition)
{
    list paramList = [targetId, targetPosition];
    request_from_seeker(which_seeker, llDumpList2String(paramList, HUFFWARE_PARM_SEPARATOR),
        HUFF_SEARCH_POINT_PARTY);
}

// stop the particle stream running at the specified search rod.
reset_rod(integer which_seeker) { request_from_seeker(which_seeker, "", HUFF_SEARCH_RESET); }

// support for reading configuration from notecards...

string CARD_CONFIGURATOR_SIGNATURE = "#searchbert";
    // the notecard signature we use for our configuration.

// tries to load a searchbert configuration notecard with the name specified.  if it's blank,
// then any config notecard will do.
request_configuration(string notecard_name)
{
    if (DEBUGGING) log_it("have hit request_configuration.");
    all_setup_finished = FALSE;
    configuration_pending = TRUE;
    // figure out whether we have a pre-chosen configuration or not.
    if (llStringLength(notecard_name) == 0) {
        // start reading the configuration from whatever card.
        llMessageLinked(LINK_THIS, CARD_CONFIGURATOR_HUFFWARE_ID,
            BEGIN_READING_NOTECARD_COMMAND, CARD_CONFIGURATOR_SIGNATURE + HUFFWARE_PARM_SEPARATOR
            + wrap_item_list([]));
    } else {
        // get the specific one they wanted.
        if (DEBUGGING) log_it("Reading configuration from notecard: " + notecard_name);
        llMessageLinked(LINK_THIS, CARD_CONFIGURATOR_HUFFWARE_ID,
            READ_PARTICULAR_NOTECARD_COMMAND, CARD_CONFIGURATOR_SIGNATURE + HUFFWARE_PARM_SEPARATOR
            + wrap_item_list([]) + HUFFWARE_PARM_SEPARATOR
            + notecard_name);
    }

    // make sure we complete this in time.
    llSetTimerEvent(STARTUP_TIME_ALLOWED);
}

// this is used when we've totally failed to start up properly.
// if "arms_problem" is true, then the issue is considered to be that the seeker arms
// are missing.
complain_about_config(integer arms_problem)
{
    configuration_pending = FALSE;  // it's failed, so we no longer pause for it.
    string explanation = "something prevented me from putting my seeker arms back on";
    if (!arms_problem) explanation = "either no notecard had the prefix '"
        + CARD_CONFIGURATOR_SIGNATURE + "'\n"
        + "or this sim is very busy and timed out";
    // we hated the notecards we found, or there were none, or our arms are still ripped off.
    llOwnerSay("Sorry...  " + explanation + ".\nI will restart now to try again.");
    llSleep(4);
    llResetScript();
}

// handles when a menu has been clicked on.
process_menu_choice(string menu_name, string av_name, string av_key, string which_choice)
{
    if (menu_name == "main") {
        if (which_choice == "Matches") {
            // if there are any current matches, describe them again.
            if (llGetListLength(global_matches_found)) show_off_what_was_found();
            else llSay(0, "There are no current matches.");
            return;
        } else if (which_choice == "Reset") {
            carefully_reset_search_list(av_key);
            return;
        }
    }
//other things will come...    
    
    if (DEBUGGING)
        log_it("non-implemented menu: " + menu_name + "/" + which_choice + " for " + av_name);
}

// reset the search list and get ready for a new search.
carefully_reset_search_list(key id)
{
    if (check_if_search_pending(FALSE)) {
        // only allow reset while running if it's the owner.
        if (id != llGetOwner()) {
            check_if_search_pending(TRUE);  // be noisy now.
            return;  // we're busy.
        }
        llOwnerSay("Stopping active search.");
        retracing_steps = TRUE;
        return;
    }
    llSay(0, "Resetting matches.");
    reset_search();
}

//////////////
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

integer debug_num = 0;
// a debugging output method.  can be disabled entirely in one place.
log_it(string to_say)
{
    debug_num++;
    // tell this to the owner.    
    llWhisper(0, llGetScriptName() + "[" + (string)debug_num + "] " + to_say);
    // say this on an unusual channel for chat if it's not intended for general public.
//    llSay(108, llGetScriptName() + "[" + (string)debug_num + "] " + to_say);
    // say this on open chat that anyone can hear.  we take off the bling for this one.
//    llSay(0, to_say);
}

// joins a list of parameters using the parameter sentinel for the library.
string wrap_parameters(list to_flatten)
{ return llDumpList2String(to_flatten, HUFFWARE_PARM_SEPARATOR); }

string wrap_item_list(list to_wrap)
{ return llDumpList2String(to_wrap, HUFFWARE_ITEM_SEPARATOR); }

// returns the index of the first occurrence of "pattern" inside
// the "full_string".  if it is not found, then a negative number is returned.
integer find_substring(string full_string, string pattern)
{ return llSubStringIndex(llToLower(full_string), llToLower(pattern)); }

// returns TRUE if the "prefix" string is the first part of "compare_with".
integer is_prefix(string compare_with, string prefix)
{ return find_substring(compare_with, prefix) == 0; }

// encases a list of vectors in the expected character for the jaunting library.
string wrap_vector_list(list to_wrap)
{
    integer len = llGetListLength(to_wrap);
    integer i;
    string to_return;
    for (i = 0; i < len; i++) {
        if (i > 0) to_return += "|";
        to_return += llList2String(to_wrap, i);
    }
    return to_return;
}

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

integer float_precision = 2;  // number of digits used when printing floats.

//hmmm: in hufflets yet???
string float_to_string(float to_print)
{
    string to_return = (string)to_print;
    // find out where the decimal point is in the string.
    integer decimal_point_posn = llSubStringIndex(to_return, ".");
    if (decimal_point_posn < 0) return to_return;
    return llGetSubString(to_return, 0, decimal_point_posn + float_precision);
}

//hmmm: in hufflets yet???
string vector_to_string(vector to_print)
{
    string to_return = "<";
    to_return += float_to_string(to_print.x);
    to_return += ", ";
    to_return += float_to_string(to_print.y);
    to_return += ", ";
    to_return += float_to_string(to_print.z);
    to_return += ">";
    return to_return;
}

// end hufflets.
//////////////

default {
    state_entry() { if (llSubStringIndex(llGetObjectName(), "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default {
    state_entry() { initialize(); }
    
    state_exit() { llSetTimerEvent(0); }
    
    on_rez(integer parm) { state default; }
    
    timer() { handle_timer(); }

    link_message(integer which, integer num, string str, key id)
    { handle_link_message(which, num, str, id); }

    changed(integer change) {
        if (change & CHANGED_INVENTORY) {
            llSleep(1.4);  // snooze to allow other things to react first.
            request_configuration("");  // see if there's a card update.
        }
    }

    listen(integer chan, string name, key id, string msg) {
        if (msg == "#reset") {
            carefully_reset_search_list(id);
        } else if (is_prefix(msg, "#find ")) {
            // needs arms for this trick.
            if (!test_health()) state default;
            // try to locate the object the user has specified.
            if (check_if_search_pending(TRUE)) return;  // we're busy.
            reset_search();  // clean out prior state.
            global_target_name = llDeleteSubString(msg, 0, 5);  // Delete "#find " from msg
            running_a_search = TRUE;  // record that we're searching.
            // point all the rods in as many different directions as we can.
            skew_rod_angles(FALSE);
            // look for any objects near each seeker and in the direction it's pointing.
            // we double the arc we might need to try to get more matches.
            start_sensing(ALL_SEEKER_ALERT, SENSOR_MAX_RANGE,
                SEARCH_ROD_ANGULAR_SWEEP, global_target_name, MAX_MATCHES);
            // start stepping through our search spirals.
            llSetTimerEvent(FAST_TIMER_INTERVAL);
            llSay(0, "Searching for " + global_target_name);
        } else if (is_prefix(msg, "#initialize")) {
            llResetScript();
        } else if (is_prefix(msg, "#skew")) {
            llSetTimerEvent(SNOOZE_BEFORE_RESET);
            reset_for_next_timer = TRUE;
            skew_rod_angles(TRUE);
        } else if (is_prefix(msg, "#channel")) {
            HUFF_SEARCH_CHAT_CHANNEL = (integer)llDeleteSubString(msg, 0, 7);
            llSay(0, "Changed listening channel to " + (string)HUFF_SEARCH_CHAT_CHANNEL + ".");
            llMessageLinked(LINK_THIS, SEARCHBERT_MENUS_HUFFWARE_ID, SM_CONFIGURE_INFO,
                wrap_parameters([HUFF_SEARCH_CHAT_CHANNEL]));
            apply_variable_definition("huff_search_chat_channel", (string)HUFF_SEARCH_CHAT_CHANNEL);
        }
    }
    
    touch_start(integer num) {
        // make sure we have already been set up.
        if (!test_health()) state default;

        if (check_if_search_pending(TRUE)) return;
        
        // send a request to the menu script for the users' clicks...
        integer indy;
        for (indy = 0; indy < num; indy++) {
            // request new menu popup for each av that clicked.
            llMessageLinked(LINK_THIS, SEARCHBERT_MENUS_HUFFWARE_ID, SM_POP_MAIN_MENU_UP, 
                wrap_parameters([llDetectedName(indy), llDetectedKey(indy)]));
        }
    }
}

/////////
// original attributions:
// Special Particle Sensor "Brain" Script
// Written by Christopher Omega
// Tasks: Listen to the owner, Parse the owner's message, Signal individual locators
// to reset, Or point at a certain object within 96 meters of the apparatus.
/////////

// note by fred huffhines:
//   much of this script was originally written by the attributed authors above.  however,
//   i have spent a ton of time improving this pair of scripts...  (the brain and the
//   search pointer).  i've added pattern matching (rather than needing to know exact
//   names), added the spiral search traversal method to improve search behavior and
//   get more results, improved the particle streams, and other bits.  i just recently added
//   configurability from notecards for searchbert options and a menuing system to provide
//   help and runtime control over some of the options.  wheeee, it's been a lot of fun.
//      --fred.
