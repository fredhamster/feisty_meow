
// huffware script: huff-search pointer, by fred huffhines
//
// note: parts of this script were written by the attributed authors below.
//
// this script is one portion of a search system.  the pointer is meant to be in a child prim.
// it is told what to point at by the root prim.
//
// the newer version of this supports a search command that locates objects in the
// direction specified.
//
// original attributions: started life as "Particle Script 0.4, Created by Ama Omega, 3-7-2004"
// some code in this script is also from: Christopher Omega.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

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

// global variables...

// SENSOR_TYPE_ALL: a constant that tells the sensor to look for (ACTIVE|PASSIVE|AGENT).
integer SENSOR_TYPE_ALL = 7;

// AXIS_* constants, represent the unit vector 1 unit on the specified axis.
vector AXIS_UP = <0.0, 0.0, 1.0>;
vector AXIS_LEFT = <0.0, 1.0, 0.0>;
vector AXIS_FWD = <1.0, 0.0, 0.0>;

integer MAX_LIST_LEN = 17;
    // the maximum matches we hang onto, to avoid using too many resources.
    // this should be no longer than the number of search arms created by
    // the brain script, but it can be less if fewer matches are needed.

///////////////

// global variables...

string search_pattern;
    // the pattern that we are hoping to find from our sensor hits.

list global_matches_found;
    // a list of keys that match the specified search terms.

integer our_link_number = 0;
    // set to the number for our link if we ever see ourselves as a link greater than 1.
    // this only happens when we have been made a sub-prim and are definitely not the root prim,
    // so it is our key for whether the prim should die if it becomes unlinked.

// Mask Flags - set to TRUE to enable
integer glow = TRUE;            // Make the particles glow
integer bounce = FALSE;          // Make particles bounce on Z plane of object
integer interpColor = TRUE;     // Go from start to end color
integer interpSize = TRUE;      // Go from start to end size
integer wind = FALSE;           // Particles effected by wind
integer followSource = TRUE;    // Particles follow the source
integer followVel = TRUE;       // Particles turn to velocity direction

// Choose a pattern from the following:
// PSYS_SRC_PATTERN_EXPLODE
// PSYS_SRC_PATTERN_DROP
// PSYS_SRC_PATTERN_ANGLE_CONE_EMPTY
// PSYS_SRC_PATTERN_ANGLE_CONE
// PSYS_SRC_PATTERN_ANGLE
integer pattern = PSYS_SRC_PATTERN_DROP;

// Select a target for particles to go towards
// "" for no target, "owner" will follow object owner 
//    and "self" will target this object
//    or put the key of an object for particles to go to
key target = "owner";

// useful particle parameters.
float age = 14;                  // Life of each particle
float startAlpha = 1.0;           // Start alpha (transparency) value
float endAlpha = 1.0;           // End alpha (transparency) value
vector startSize = <0.2, 0.2, 0.2>;     // Start size of particles 
vector endSize = <0.8, 0.8, 0.8>;       // End size of particles (if interpSize == TRUE)

// colors are now assigned dynamically per search.
vector startColor;  // Start color of particles <R,G,B>
vector endColor;  // End color of particles <R,G,B> (if interpColor == TRUE)

// unused particle parameters.
float maxSpeed = 2;            // Max speed each particle is spit out at
float minSpeed = 2;            // Min speed each particle is spit out at
string texture;                 // Texture used for particles, default used if blank
vector push = <0.0, 0.0, 0.0>;          // Force pushed on particles

// System parameters
float rate = 0.08;            // burst rate to emit particles, zero is fastest.
float radius = 1;          // Radius to emit particles for BURST pattern
integer count = 1;        // How many particles to emit per BURST 
float outerAngle = 1.54;    // Outer angle for all ANGLE patterns
float innerAngle = 1.55;    // Inner angle for all ANGLE patterns
vector omega = <0,0,0>;    // Rotation of ANGLE patterns around the source
float life = 0;             // Life in seconds for the system to make particles

// Script variables
integer precision = 2;          //Adjust the precision of the generated list.

integer running_particles = FALSE;  // is the particle system running?

///integer start_parm;  // set from the on_rez parameter.

string float2String(float in)
{
    return llGetSubString((string)in, 0, precision - 7);
}

create_particles()
{
    list system_content;
    integer flags = 0;
    if (target == "owner") target = llGetOwner();
    if (target == "self") target = llGetKey();
    if (glow) flags = flags | PSYS_PART_EMISSIVE_MASK;
    if (bounce) flags = flags | PSYS_PART_BOUNCE_MASK;
    if (interpColor) flags = flags | PSYS_PART_INTERP_COLOR_MASK;
    if (interpSize) flags = flags | PSYS_PART_INTERP_SCALE_MASK;
    if (wind) flags = flags | PSYS_PART_WIND_MASK;
    if (followSource) flags = flags | PSYS_PART_FOLLOW_SRC_MASK;
    if (followVel) flags = flags | PSYS_PART_FOLLOW_VELOCITY_MASK;
    if (target != "") flags = flags | PSYS_PART_TARGET_POS_MASK;

    // original recipe searchbert pointer...
//    startColor = <0.92, 0.79412, 0.66863>;    // Start color of particles <R,G,B>
//    endColor = <0.0, 1.0, 0.7>;      // End color of particles <R,G,B> (if interpColor == TRUE)
    // new randomized version for colors.  the current aesthetic here is to start
    // with a relatively dark color and end on a relatively light color.
    startColor = <randomize_within_range(0.14, 0.95, FALSE),
        randomize_within_range(0.14, 0.95, FALSE),
        randomize_within_range(0.14, 0.95, FALSE)>;
    endColor = <randomize_within_range(0.28, 0.95, FALSE),
        randomize_within_range(0.28, 0.95, FALSE),
        randomize_within_range(0.28, 0.95, FALSE)>;

    system_content = [ PSYS_PART_MAX_AGE, age,
        PSYS_PART_FLAGS,flags,
        PSYS_PART_START_COLOR, startColor,
        PSYS_PART_END_COLOR, endColor,
        PSYS_PART_START_SCALE, startSize,
        PSYS_PART_END_SCALE, endSize, 
        PSYS_SRC_PATTERN, pattern,
        PSYS_SRC_BURST_RATE, rate,
        PSYS_SRC_ACCEL, push,
        PSYS_SRC_BURST_PART_COUNT, count,
        PSYS_SRC_BURST_RADIUS, radius,
        PSYS_SRC_BURST_SPEED_MIN, minSpeed,
        PSYS_SRC_BURST_SPEED_MAX, maxSpeed,
//      PSYS_SRC_INNERANGLE,innerAngle,
//      PSYS_SRC_OUTERANGLE,outerAngle,
        PSYS_SRC_OMEGA, omega,
        PSYS_SRC_MAX_AGE, life,
//      PSYS_SRC_TEXTURE, texture,
        PSYS_PART_START_ALPHA, startAlpha,
        PSYS_PART_END_ALPHA, endAlpha
    ];
    if (target != NULL_KEY) {
        system_content += [ PSYS_SRC_TARGET_KEY, target ];
    }

    llParticleSystem(system_content);
    running_particles = TRUE;
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

// SetLocalRot
// In a linked set, points a child object to the rotation.
// @param rot The rotation to rotate to.
SetLocalRot(rotation rot)
{
    if(llGetLinkNumber() > 1)
    {
        rotation locRot = llGetLocalRot();
        locRot.s = -locRot.s; // Invert local rot.
        
        rotation parentRot = locRot * llGetRot();
        parentRot.s = -parentRot.s; // Invert parent's rot.
    
        llSetLocalRot(rot * parentRot);
    }
}

// Gets the rotation to point the specified axis at the specified position.
// @param axis The axis to point. Easiest to just use an AXIS_* constant.
// @param target The target, in region-local coordinates, to point the axis at.
// @return The rotation necessary to point axis at target.
rotation getRotToPointAxisAt(vector axis, vector target)
{
    return llGetRot() * llRotBetween(axis * llGetRot(), target - llGetPos());
}

// aims in the direction of the target.
aim_at(vector targetPos)
{
    SetLocalRot(getRotToPointAxisAt(AXIS_UP, targetPos));
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

// point_at
// Points up axis at targetPos, and emits a particle system at targetKey.
// @param targetKey The UUID of the target to emit particles to.
// @param targetPos The poaition of the target in region-local coordinates.
point_at(key targetKey, vector targetPos)
{
    aim_at(targetPos);
    target = targetKey;
    create_particles();
}

// variables that are established by a search and used periodically in the timer.
float max_range = 0.0;
float arc_angle = 0.0;
float sensor_interval = 0.0;

reset_sensors()
{
    radius = 1;
    llSetTimerEvent(0.0);
    llSensorRemove();
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
    // say this on an unusual channel for chat if it's not intended for general public.
//    llSay(108, llGetScriptName() + "[" + (string)debug_num + "] " + to_say);
    // say this on open chat that anyone can hear.  we take off the bling for this one.
//    llSay(0, to_say);
}

// returns TRUE if the "pattern" is found in the "full_string".
integer matches_substring(string full_string, string pattern)
{ return (find_substring(full_string, pattern) >= 0); }

// returns the index of the first occurrence of "pattern" inside
// the "full_string".  if it is not found, then a negative number is returned.
integer find_substring(string full_string, string pattern)
{ return llSubStringIndex(llToLower(full_string), llToLower(pattern)); }

// returns TRUE if the "prefix" string is the first part of "compare_with".
integer is_prefix(string compare_with, string prefix)
{ return find_substring(compare_with, prefix) == 0; }

// returns a number at most maximum and at least minimum.
// if "allow_negative" is TRUE, then the return may be positive or negative.
float randomize_within_range(float minimum, float maximum, integer allow_negative)
{
    float to_return = minimum + llFrand(maximum - minimum);
    if (allow_negative) {
        if (llFrand(1.0) < 0.5) to_return *= -1.0;
    }
    return to_return;
}

// makes sure that we record the current link number if it's higher than 1; this
// is how we know that we're a sub-prim.
record_link_num_if_useful()
{
    if (llGetLinkNumber() > 1) our_link_number = llGetLinkNumber();
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
        llParticleSystem([]);
        running_particles = FALSE;
        SetLocalRot(<0.0, 0.0, 0.0, 1.0>);
        global_matches_found = [];
        record_link_num_if_useful();
    }
    on_rez(integer parm) {
        record_link_num_if_useful();
        state default;
    }
    link_message(integer sender, integer num, string command, key parameters) {
        if (num != HUFF_SEARCH_POINTER_HUFFWARE_ID) return;  // not for us.
        if (llGetLinkNumber() <= 1) return;  // do nothing as root prim.
        if (command == HUFF_SEARCH_RESET) {
            // returns to the normal state of the object.
            reset_sensors();
            SetLocalRot(<0, 0, 0, 1>);
            llParticleSystem([]);
            running_particles = FALSE;
            global_matches_found = [];
        } else if (command == HUFF_SEARCH_POINT_PARTY) {
            // aim at an object and show a particle stream leading to it.
            reset_sensors();
            list parsedParameters = llParseString2List(parameters, [HUFFWARE_PARM_SEPARATOR], []);
            key targetKey = (key)llList2String(parsedParameters, 0);
            vector targetPos = (vector)llList2String(parsedParameters, 1);
            point_at(targetKey, targetPos);
        } else if (command == HUFF_SEARCH_JUST_POINT) {
            // aim at an object, but don't do any particles.
            reset_sensors();
            list parsedParameters = llParseString2List(parameters, [HUFFWARE_PARM_SEPARATOR], []);
            vector targetPos = (vector)llList2String(parsedParameters, 0);
            aim_at(targetPos);
        } else if (command == HUFF_SEARCH_SENSEI) {
            // set up a sensor request for a search pattern.  pings will cause
            // the pattern to be sought in names of nearby objects.
            reset_sensors();
            global_matches_found = [];  // reset any previous matches.
            list parsedParameters = llParseString2List(parameters, [HUFFWARE_PARM_SEPARATOR], []);
            max_range = (float)llList2String(parsedParameters, 0);
            arc_angle = (float)llList2String(parsedParameters, 1);
            search_pattern = llToLower((string)llList2String(parsedParameters, 2));
            MAX_LIST_LEN = (integer)llList2String(parsedParameters, 3);
if (!MAX_LIST_LEN) {
MAX_LIST_LEN = 17; 
log_it("failed to get list length param");
}
        } else if (command == HUFF_SEARCH_STOP_SENSE) {
            // turn off the sensor but don't totally reset yet.
            reset_sensors();
            global_matches_found = [];
        } else if (command == HUFF_SEARCH_PING) {
            // do a little particle emission while searching, just to let them know
            // where we've been.
            target = "self";
            radius = 5;
            if (!running_particles) create_particles();
            // they want to check for objects right here, right now...
            llSensor("", NULL_KEY, SENSOR_TYPE_ALL, max_range, arc_angle);
        }
    }
    changed(integer change) {
        if (change & CHANGED_LINK) {
            // we have been linked or unlinked or sat upon.
            if ( (our_link_number > 1) && (llGetLinkNumber() == 0) ) {
                // this is now a single prim linked to nothing.
                llDie();
            } else if (our_link_number <= 1) {
                // we had no link number recorded, so let's track this if needed.
                record_link_num_if_useful();
            }
        }
    }
    sensor(integer num_detected) {
        if (llGetLinkNumber() <= 1) return;  // do nothing as root prim.
        if (llGetListLength(global_matches_found) > MAX_LIST_LEN) {
            // we have enough matches already.  stop adding more.
            return;
        }
        
        list parms = [];  // the full set of matches we found.
        integer i;  // loop variable.
        integer matches_found = 0;
        key root_key = llGetLinkKey(1);
        for (i = 0; i < num_detected; i++) {
            key targetKey = llDetectedKey(i);
            string target_name = llDetectedName(i);
            if ( (targetKey != root_key)
                    // we don't want to report our own object.
                && matches_substring(target_name, search_pattern)
                    // check whether the current target matches our search pattern.
                && (find_in_list(global_matches_found, targetKey) < 0) ) {
                    // make sure we haven't already reported this one.
                    
//if (matches_substring(target_name, "searchbert v")) {
//log_it("somehow got past the is it myself check with a no-answer, my key=" + llGetKey() + " their key=" + targetKey);
//}

                // store the match now.  even if we don't like where it's located
                // (as in, outside our sim), we still don't want to keep matching it
                // and looking at it.
                global_matches_found += targetKey;

                // the name matched the search pattern...  but make sure the
                // location is worth reporting; if it's outside the sim, we will
                // not be able to name it or point at it properly.
                vector location = llDetectedPos(i);
                if (!outside_of_sim(location)) {
                    // it's a match that's inside the sim.  send it along.
                    parms += [ targetKey, location ];
                    matches_found++;  // we got one!
//log_it("match added: " + (string)targetKey);
                    // shorten lists of matches so we don't overload the brain.
                    if (llGetListLength(parms) >= 8) {
                        llMessageLinked(LINK_ROOT,
                            HUFF_SEARCH_POINTER_HUFFWARE_ID + REPLY_DISTANCE,
                            HUFF_SEARCH_MATCH_EVENT,
                            llDumpList2String(parms, HUFFWARE_PARM_SEPARATOR));
                        matches_found = 0;
                        parms = [];
                    }
                }
            }
        }
        if (matches_found) {
//log_it("sending " + (string)matches_found + " matches in message...");
            // send message about matches back to parent.
            llMessageLinked(LINK_ROOT, HUFF_SEARCH_POINTER_HUFFWARE_ID + REPLY_DISTANCE,
                HUFF_SEARCH_MATCH_EVENT,
                llDumpList2String(parms, HUFFWARE_PARM_SEPARATOR));
        }
    }
}
