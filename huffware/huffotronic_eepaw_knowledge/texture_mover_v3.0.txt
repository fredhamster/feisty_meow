
// huffware script: texture mover, by fred huffhines
//
// moves a texture across the object, either by smooth animation or brute force offsetting.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

// constants...

integer USE_TEXTURE_ANIMATION = TRUE;
    // by default, we use texture animation which is a lower-lag way to
    // make the texture move.  if this is false, then we use a timed update
    // to move the texture instead.
    // note: these modes are actually really different.  the texture animation
    // is included here because i wanted to smooth out some objects that used
    // texture movement.

// used in both types...

float TIMER_INTERVAL = 0.2;  // how fast do we move texture?

// for timed movement...

vector OFFSET_MOVEMENT = ZERO_VECTOR;  // no movement.
//vector OFFSET_MOVEMENT = <0.0, -0.02, 0.0>;
    // how much to move the texture by in x,y,z directions.

//float OFFSET_ROTATION = 0.0;  // in degrees.
float OFFSET_ROTATION = -1.0;  // in degrees.

// for texture animations...

float MOVER_TIMER_DIVISOR = 30.0;
    // makes the movement comparable to timed method.

float ROTATER_TIMER_DIVISOR = 7.0;
    // makes the rotation comparable to timed method.

// iterative movement of texture.
move_texture(vector offset)
{
    vector current = llGetTextureOffset(ALL_SIDES);
    current += offset;
    if (current.x > 1.0) current.x = -1.0;
    if (current.x < -1.0) current.x = 1.0;
    if (current.y > 1.0) current.y = -1.0;
    if (current.y < -1.0) current.y = 1.0;
    llOffsetTexture(current.x, current.y, ALL_SIDES);
}

// iterative rotation of texture.  "spin" is measured in degrees.
spin_texture(float spin)
{
    float rot = llGetTextureRot(ALL_SIDES);  // get our current state.
    rot += spin * DEG_TO_RAD;  // add some rotation.
    llRotateTexture(rot, ALL_SIDES); //rotate the object   
}

initialize_texture_mover()
{
    if (!USE_TEXTURE_ANIMATION) {
        // we're stuck with the timed update style for movement.
        llSetTimerEvent(TIMER_INTERVAL);
        // turn off previous animation.
        llSetTextureAnim(0, ALL_SIDES, 0, 0, 0, 0, 0);
    } else {
        // we can just set the texture movement here and be done with it.
        integer x_frames = 1;
        integer y_frames = 1;
        llSetTimerEvent(0);  // we don't use a timer.

//hmmm: how do we combine rotation and offsets?  currently mutually exclusive.

        if (OFFSET_MOVEMENT != ZERO_VECTOR) {
            float timer_interval = TIMER_INTERVAL / MOVER_TIMER_DIVISOR;
log_it("getting x_frames=" + (string)x_frames
+ " y_frames=" + (string)y_frames
+ " timer_intvl=" + (string)timer_interval);

            llSetTextureAnim(ANIM_ON | LOOP | SMOOTH, ALL_SIDES,
                x_frames, y_frames,
                0, 100, timer_interval);
        } else if (OFFSET_ROTATION != 0.0) {
            float timer_interval = TIMER_INTERVAL / ROTATER_TIMER_DIVISOR;
            // we're actually not using the rotation at all here, except for
            // the sign.  that seems pretty busted.
            if (OFFSET_ROTATION < 0) timer_interval *= -1.0;
            llSetTextureAnim(ANIM_ON | LOOP | SMOOTH | ROTATE, ALL_SIDES,
                0, 0,
                0, TWO_PI, timer_interval);
        }
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
    // say this on an unusual channel for chat if it's not intended for general public.
//    llSay(108, llGetScriptName() + "[" + (string)debug_num + "] " + to_say);
    // say this on open chat that anyone can hear.  we take off the bling for this one.
//    llSay(0, to_say);
}
//
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

default {
    state_entry() { if (llSubStringIndex(llGetObjectName(), "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default
{
    state_entry() {
        auto_retire();
        initialize_texture_mover();
    }

    timer() {
        // the timed approach is the only one so far that allows both a
        // movement of the texture and a rotation.
        if (OFFSET_MOVEMENT != ZERO_VECTOR) move_texture(OFFSET_MOVEMENT);
        if (OFFSET_ROTATION != 0.0) spin_texture(OFFSET_ROTATION);
    }
}

