
// huffware script: rotanium rotato, by fred huffhines.
//
// causes the object to rotate according to the parameters set below.
// this can use herky-jerky timed rotation with llSetRot or it can use
// smooth rotation with llTargetOmega.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

integer RANDOMIZE_ROTATION = TRUE;

integer SMOOTH_ROTATION = TRUE;
    // if this is true, then the object will rotate smoothly rather than
    // being rotated by the timer.

float SMOOTH_TIMER_FREQUENCY = 14.0;
    // the smooth rotater doesn't need to hit the timer all that often.

float SMOOTH_CHANCE_FOR_ADJUSTING = 0.8;
    // we won't always change the smooth rotation, even though our timer
    // is pretty slow.

float SMOOTH_ROTATION_GAIN_MAX = 0.0490873852122;
    // the gain is how fast we will rotate in radians per second.
    // PI / 2 is about 90 degrees per second, which seems way too fast.
    // 0.196349540849 is about PI / 16, 0.0981747704244 is about PI / 32,
    // and 0.0490873852122 is about PI / 64.

float JERKY_TIMER_FREQUENCY = 0.50;
    // this is the fastest that llSetRot rotation can happen anyhow,
    // so we fire the timer at this rate.
    
float JERKY_CHANCE_FOR_ADJUSTING = 0.1;
    // this is the probability of changing the current direction.

vector current_add_in = <0.0, 0.0, 0.4>;
    // randomly assigned to if RANDOMIZE_ROTATION is true.

float current_gain = -0.05;
    // speed of smooth rotation; will randomly change if RANDOMIZE_ROTATION is true.

float MIN_ADDITION = 0.01;
    // smallest amount of change we will ever have.
float MAX_ADDITION = 7.0;
    // largest amount of change we will ever have.

// sets the gain and add in to random choices.
randomize_values()
{
    current_gain = randomize_within_range(0.001, SMOOTH_ROTATION_GAIN_MAX, TRUE);
    current_add_in = random_vector(MIN_ADDITION, MAX_ADDITION, TRUE);
}

// performs the timed rotation that has been configured for us.
rotate_as_requested()
{
    if (SMOOTH_ROTATION) {
        // our slack timer went off, so randomize the rotation if requested.
        if (RANDOMIZE_ROTATION && (llFrand(1.0) >= SMOOTH_CHANCE_FOR_ADJUSTING) )
            randomize_values();
        // make sure we are using the rotational values we were asked to.
        llTargetOmega(current_add_in, current_gain, 1.0);
    } else {
        // herky jerky rotation.
    
//hmmm: seeing that GetRot or GetLocalRot might be useful at different times.
        rotation curr_rot = llGetLocalRot();  // get our current state.
        vector euler_curr = llRot2Euler(curr_rot);  // turn into euler coords.
        euler_curr *= RAD_TO_DEG;  // convert to degrees.
        vector new_rot = euler_curr + current_add_in;  // add our adjustment in.
        new_rot *= DEG_TO_RAD; // convert to radians.
        rotation quat = llEuler2Rot(new_rot); // convert to quaternion
        llSetLocalRot(quat);  // rotate the object
//will the local work for a single prim?
//in the current case, we do want just the local rot to change.
    
        // change the rotation add-in if the mood strikes us.
        float starter = 0.420;  // where we start looking for change.
        float change_cap = starter + JERKY_CHANCE_FOR_ADJUSTING;
        float randomness = llFrand(1.000);
        if ( (randomness <= change_cap) && (randomness >= starter) ) {
            // time for a change in the rotation.
            if (RANDOMIZE_ROTATION)
                randomize_values();
        }
    }
}

//////////////
// start of hufflets...

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

// end hufflets
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
        auto_retire();
        // if needed, we will set our initial random rotation.
        if (RANDOMIZE_ROTATION) randomize_values();
        // do a first rotate, so we move right at startup.  otherwise we won't move
        // until after our first timer hits.
        rotate_as_requested();
        // now set the timer for our mode.
        if (SMOOTH_ROTATION) {
            llSetTimerEvent(SMOOTH_TIMER_FREQUENCY);
        } else {
            llSetTimerEvent(JERKY_TIMER_FREQUENCY);
        }
    }

    timer() { rotate_as_requested(); }
}

