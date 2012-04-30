
// huffware script: axis rider, by fred huffhines.
//
// causes an object to move up and down on the z axis, although that and
// other parameters can be modified.
//
//   this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
//   do not use it in objects without fully realizing you are implicitly accepting that license.
//

// these two control how far from the original position the object may travel.
float MIN_POSITION = -0.3;
float MAX_POSITION = 1.0;
// these specify which axis to modify and compare on.
integer USE_X_AXIS = FALSE;
integer USE_Y_AXIS = FALSE;
integer USE_Z_AXIS = TRUE;
// these are the limits on how much the object can move during one timer click.
float MIN_POSITION_ADJUSTMENT = 0.05;
float MAX_POSITION_ADJUSTMENT = 0.07;

// jitter is the capability for the object to not be aligned on a straight
// up and down axis.
integer JITTER_EFFECT = TRUE;
    // if this is true, then the rider will move unevenly at various angles.
float CHANCE_FOR_JITTER = 0.60;
    // this is the probability of changing the current direction (0.0 to 1.0).
vector MIN_ANGULAR_POSITION = <0.0, 0.0, 0.0>;  // minimum jitter amount, in degrees.
vector MAX_ANGULAR_POSITION = <1.0, 1.0, 4.0>;  // maximum jitter amount, in degrees.

float TIMER_FREQUENCY = 0.20;
    // this is the fastest that prim changes can happen anyhow,
    // so we fire the timer at this rate.

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

// from hufflets...

integer debug_num = 0;

// a debugging output method.  can be disabled entirely in one place.
log_it(string to_say)
{
    debug_num++;
    // tell this to the owner.    
    llOwnerSay(llGetScriptName() + "--" + (string)debug_num + ": " + to_say);
    // say this on open chat, but use an unusual channel.
//    llSay(108, llGetScriptName() + "--" + (string)debug_num + ": " + to_say);
}

// returns a number at most "maximum" and at least "minimum".
// if "allow_negative" is TRUE, then the return may be positive or negative.
float randomize_within_range(float minimum, float maximum, integer allow_negative)
{
    if (minimum > maximum) {
        // flip the two if they are reversed.
        float temp = minimum; minimum = maximum; maximum = temp;
    }
    float to_return = minimum + llFrand(maximum - minimum);
    if (allow_negative) {
        if (llFrand(1.0) < 0.5) to_return *= -1.0;
    }
    return to_return;
}

// returns a random vector where x,y,z will be between "minimums" and "maximums"
// x,y,z components.  if "allow_negative" is true, then any component will
// randomly be negative or positive.
vector random_bound_vector(vector minimums, vector maximums, integer allow_negative)
{
    return <randomize_within_range(minimums.x, maximums.x, allow_negative),
        randomize_within_range(minimums.y, maximums.y, allow_negative),
        randomize_within_range(minimums.z, maximums.z, allow_negative)>;
}

// returns a vector whose components are between minimum and maximum.
// if allow_negative is true, then they can be either positive or negative.
vector random_vector(float minimum, float maximum, integer allow_negative)
{
    return random_bound_vector(<minimum, minimum, minimum>,
        <maximum, maximum, maximum>, allow_negative);
}

// returns TRUE if a is less than b in any component.
integer vector_less_than(vector a, vector b)
{ return (a.x < b.x) || (a.y < b.y) || (a.z < b.z); }

// returns TRUE if a is greater than b in any component.
integer vector_greater_than(vector a, vector b)
{ return (a.x > b.x) || (a.y > b.y) || (a.z > b.z); }

// returns a list with two components; a new vector and a boolean.
// the new vector starts from "starting_point".  it will have a vector
// between "minimum_addition" and "maximum_addition" added to it.
// if it is over the "minimum_allowed" or the "maximum_allowed", then
// it is reset to whichever it would have crossed over.  two booleans
// are also returned to indicate when the lower and upper limits were
// exceeded (in that order).
list limit_and_add(vector starting_point,
    vector minimum_allowed, vector maximum_allowed,
    vector minimum_addition, vector maximum_addition)
{
    integer too_low = FALSE;
    integer too_high = FALSE;
    vector new_location = starting_point;
    vector addition = random_bound_vector(minimum_addition, maximum_addition, FALSE);
//log_it("start=" + (string)starting_point + " addin=" + (string)addition);
    new_location += addition;
    if (vector_less_than(new_location, minimum_allowed)) {
        too_low = TRUE;
        new_location = minimum_allowed;
    } else if (vector_greater_than(new_location, maximum_allowed)) {
        too_high = TRUE;
        new_location = maximum_allowed;
    }
    return [ new_location, too_low, too_high ];
}

//////////////

// variables used during the script.

vector rez_position;
    // set at time of object rez from object's current location.

vector rez_rotation;
    // set at time of object rez from object's current rotation.

vector current_position_addin = <0.0, 0.0, 0.0>;
    // the amount that we're adding to the rider's position right now.

vector current_rotation_addin = <0.0, 0.0, 0.0>;
    // randomly assigned to if JITTER_EFFECT is true.

vector current_direction = <1.0, 1.0, 1.0>;
    // we start out by adding to all axes.

// provides a random vector that could be negative or positive for
// any of the values.  the range is given by the two constants
// MIN_ADDITION and MAX_ADDITION.
vector rando_vector()
{
    return random_bound_vector(MIN_ANGULAR_POSITION * DEG_TO_RAD,
        MAX_ANGULAR_POSITION * DEG_TO_RAD, TRUE);
}

default {
    state_entry() { if (llSubStringIndex(llGetObjectName(), "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default
{
    state_entry()
    {
        auto_retire();
        rez_position = llGetPos();
        rez_rotation = llRot2Euler(llGetRot());
//vector calc_rot = rez_rotation * RAD_TO_DEG;
//log_it("rotation at start is " + (string)calc_rot);
        llSetTimerEvent(TIMER_FREQUENCY);
    }
    
    on_rez(integer parm) { llResetScript(); }

    timer()
    {
        if (USE_X_AXIS) {
             list add_result = limit_and_add(current_position_addin,
                <MIN_POSITION, current_position_addin.y, current_position_addin.z>,
                <MAX_POSITION, current_position_addin.y, current_position_addin.z>,
                <current_direction.x * MIN_POSITION_ADJUSTMENT, 0.0, 0.0>,
                <current_direction.x * MAX_POSITION_ADJUSTMENT, 0.0, 0.0>);
            current_position_addin = llList2Vector(add_result, 0);
            integer too_low = llList2Integer(add_result, 1);
            integer too_high = llList2Integer(add_result, 2);
            if (too_low) current_direction.x = 1.0;
            else if (too_high) current_direction.x = -1.0;
        }
        if (USE_Y_AXIS) {
             list add_result = limit_and_add(current_position_addin,
                <current_position_addin.x, MIN_POSITION, current_position_addin.z>,
                <current_position_addin.x, MAX_POSITION, current_position_addin.z>,
                <0.0, current_direction.y * MIN_POSITION_ADJUSTMENT, 0.0>,
                <0.0, current_direction.y * MAX_POSITION_ADJUSTMENT, 0.0>);
            current_position_addin = llList2Vector(add_result, 0);
            integer too_low = llList2Integer(add_result, 1);
            integer too_high = llList2Integer(add_result, 2);
            if (too_low) current_direction.y = 1.0;
            else if (too_high) current_direction.y = -1.0;
        }
        if (USE_Z_AXIS) {
             list add_result = limit_and_add(current_position_addin,
                <current_position_addin.x, current_position_addin.y, MIN_POSITION>,
                <current_position_addin.x, current_position_addin.y, MAX_POSITION>,
                <0.0, 0.0, current_direction.z * MIN_POSITION_ADJUSTMENT>,
                <0.0, 0.0, current_direction.z * MAX_POSITION_ADJUSTMENT>);
            current_position_addin = llList2Vector(add_result, 0);
            integer too_low = llList2Integer(add_result, 1);
            integer too_high = llList2Integer(add_result, 2);
            if (too_low) current_direction.z = 1.0;
            else if (too_high) current_direction.z = -1.0;
        }
        
//logic below for randomness is a bit odd.
        // change the jitter position if we get a chance.
        float starter = 0.420;  // where we start looking for change.
        float change_cap = starter + CHANCE_FOR_JITTER;
        float randomness = llFrand(1.000);
        if ( (randomness <= change_cap) && (randomness >= starter) ) {
            // time for a change in the rotation.
            if (JITTER_EFFECT) {
                current_rotation_addin = rando_vector();
            }
        }

        llSetPrimitiveParams([
            PRIM_POSITION, rez_position + current_position_addin,
            PRIM_ROTATION, llEuler2Rot(rez_rotation + current_rotation_addin)
            ]);

    }
}

