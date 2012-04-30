
// huffware script: timed expiration, by fred huffhines.
//
// this ensures that an object only lives a set amount of time before de-rezzing.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

float PRODUCT_LIFETIME = 0.9;  // time allowed before the object terminates, in minutes.
    // since we now make the object temporary, this has to be less than a minute
    // for our own dying scene to take place.  otherwise the object just disappears.

float EXPIRY_VOLUME = 0.25;
    // we have found that this volume level is generally good, since you don't want
    // the object screaming like crazy as it evaporates.


//////////////
// huffware script: auto-retire, by fred huffhines, version 1.9.
// distributed under BSD-like license.
//   partly based on the self-upgrading scripts from markov brodsky and jippen faddoul.
// the function auto_retire() should be added *inside* a version numbered script that
// you wish to give the capability of self-upgrading.
//   this script supports a notation for versions embedded in script names where a 'v'
// is followed by a number in the form "major.minor", e.g. "grunkle script by ted v8.2".
// when the containing script is dropped into an object with a different version, the
// most recent version eats any existing ones.
//   keep in mind that this code must be *copied* into your script you wish to add
// auto-retirement capability to.
//
// example usage of the auto-retirement script:
//
// default {
//    state_entry() {
//        auto_retire();  // make sure newest addition is only version of script.
//    }
// }
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
                if ((float)inv_version_string < (float)version_string) {
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
    if (llSubStringIndex(to_chop_up, " ") < 0) return [];  // no space found, not a valid name to work on.
        
    string basename = to_chop_up;  // script name with no version attached.
    
    integer posn;
    // minimum script name is 2 characters plus version.
    for (posn = llStringLength(to_chop_up) - 1;
        (posn >= 2) && (llGetSubString(to_chop_up, posn, posn) != " ");
        posn--) {
        // find the space.  do nothing else.
    }
    if (posn < 2) return [];  // no space found.
    string full_suffix = llGetSubString(to_chop_up, posn, -1);
    // ditch the space character for our numerical check.
    string chop_suffix = llGetSubString(full_suffix, 1, llStringLength(full_suffix) - 1);
    // strip out a 'v' if there is one.
    if (llGetSubString(chop_suffix, 0, 0) == "v")
        chop_suffix = llGetSubString(chop_suffix, 1, llStringLength(chop_suffix) - 1);
    // if valid floating point number and greater than zero, that works for our version.
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

// how many times the timer is hit per minute.
float TICKS_PER_MINUTE = 20.0;

float ticks_to_live;  // how long this particular object has left.

show_time_left()
{
//    integer secs_left = (integer)(60.0 * ticks_to_live / TICKS_PER_MINUTE);
//    llWhisper(14, (string)secs_left + " seconds before expiration.");
}

default
{
    state_entry() { if (llSubStringIndex(llGetObjectName(),  "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default
{
    state_entry() {
        llParticleSystem([]);  // kill any old particles running.
        auto_retire();
//llWhisper(0, "starting up " + llGetObjectName());
        ticks_to_live = PRODUCT_LIFETIME * TICKS_PER_MINUTE;
        llSetTimerEvent(60.0 / TICKS_PER_MINUTE);  // check avatar's state periodically.
        // make sure this is a temporary object.
        llSetPrimitiveParams([PRIM_TEMP_ON_REZ, TRUE]);
        show_time_left();
    }

    timer() {
        // make sure we don't wait too long to reset our temporariness.
        integer time_now = llGetUnixTime();
        // check whether we should stay alive or not.
        if (--ticks_to_live <= 0.0) {
//            llWhisper(0, llGetObjectName() + " is terminating now.");
            string first_sound = llGetInventoryName(INVENTORY_SOUND, 0);
            if (first_sound != "")
                llTriggerSound(first_sound, EXPIRY_VOLUME);
            string first_pic = llGetInventoryName(INVENTORY_TEXTURE, 0);
            if (first_pic != "")
                llMakeExplosion(20, 1.0, 5, 3.0, 1.0, first_pic, ZERO_VECTOR);
            llDie();
            // resetting this since if we're here, we don't want to keep blowing up.
            ticks_to_live = PRODUCT_LIFETIME * TICKS_PER_MINUTE;
        } else {
            show_time_left();
        }
    }
    
    on_rez(integer param) {
        // make sure we start over when this is a new object.
        llResetScript();
    }
    
    touch_start(integer total_number)
    {
        ticks_to_live += TICKS_PER_MINUTE;  // add a minute to its life.
//        llWhisper(0, "Object gets another minute to live.");
    }

}
