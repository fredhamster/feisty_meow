
// huffware script: color smoove clock, by fred huffhines
//
// A clock with rotating color text, adjustable for your time zone.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

// constants you might want to modify...

integer TIME_ZONE = -5;
  // the time zone you want the clock to display, measured as an offset from GMT.

float TIMER_INTERVAL = 0.1;
  // how frequently timer is hit to adjust color and possibly change time text.

float MAX_RANDOM_COLOR_JUMP = 0.07;
  // how much the color might change in one timer hit for one R/G/B component.

// global variables...

// remembers the last time string we printed, since we want to be modifying the
// colors more frequently than we're changing the clock.
string global_time_string = "";

// track how many ticks have hit the timer.  only updates the time when a whole
// second has gone by.
integer global_tick_counter = 0;

// the current color for clock text; the default is just a starting point.
vector global_text_color = <0.3, 0.6, 0.8>;

// records whether the value is going up (positive) or down (negative).
float global_elevator_x = -1.0;
float global_elevator_y = 1.0;
float global_elevator_z = 1.0;

// do not change the following constants...

integer PST_TIME_ZONE = -8;  // this constant for the pacific time zone's offset from GMT.

// makes sure components of the color don't go out of range.  if they do, then
// the elevator direction is reversed.
normalize_color()
{
    if (global_text_color.x > 1.0) { global_text_color.x = 1.0; global_elevator_x *= -1.0; }
    if (global_text_color.x < 0.0) { global_text_color.x = 0.0; global_elevator_x *= -1.0; }
    if (global_text_color.y > 1.0) { global_text_color.y = 1.0; global_elevator_y *= -1.0; }
    if (global_text_color.y < 0.0) { global_text_color.y = 0.0; global_elevator_y *= -1.0; }
    if (global_text_color.z > 1.0) { global_text_color.z = 1.0; global_elevator_z *= -1.0; }
    if (global_text_color.z < 0.0) { global_text_color.z = 0.0; global_elevator_z *= -1.0; }
}

// show the current time string in our current color.
display_colored_text()
{
    llSetText(global_time_string, global_text_color, 1.0);
}

rotate_text_color()
{
    // gnarly version is totally random.  merely an example now.
//yuck; flashy    global_text_color = <llFrand(1.0), llFrand(1.0), llFrand(1.0)>;

    // better; rotates the colors as slow as you like, but somewhat randomly
    // for an interesting color glide effect.
    global_text_color.x += llFrand(MAX_RANDOM_COLOR_JUMP) * global_elevator_x;
    global_text_color.y += llFrand(MAX_RANDOM_COLOR_JUMP) * global_elevator_y;        
    global_text_color.z += llFrand(MAX_RANDOM_COLOR_JUMP) * global_elevator_z;
    
    normalize_color();  // make sure we didn't go out of bounds.
        
    display_colored_text();  // update to the new color.
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
    // state_entry() is an event handler, it executes whenever a state is entered.
    state_entry()
    {
        auto_retire();
        llSetTimerEvent(TIMER_INTERVAL);
    }

    timer()
    {
        // calculate our current rate of timer hits.
        float ticks_per_second = 1.0 / TIMER_INTERVAL;
        
        rotate_text_color();  // spin the text color every timer hit.
        
        // check whether the clock time has changed since last updated.
        global_tick_counter++;
        if (global_tick_counter < ticks_per_second) {
            return;
        }
        // yep, we need to update the text of the time.
        global_tick_counter = 0;  // reset for rollover.
        
        // get the number of seconds off of the clock on the wall...
        integer full_seconds = llFloor(llGetWallclock());

        // switch to the chosen time zone.
        full_seconds += -(PST_TIME_ZONE - TIME_ZONE) * 60 * 60;

        // a constant for how many seconds exist in a day.
        integer DAY_OF_SECONDS = 24 * 60 * 60;

        // correct any tendency of the seconds to be below zero or above the maximum
        // due to wacky time zones.
        while (full_seconds < 0) full_seconds += DAY_OF_SECONDS;
        while (full_seconds > DAY_OF_SECONDS) full_seconds -= DAY_OF_SECONDS;
        
        // calculate all the components of the time.
        integer seconds = full_seconds;
        integer minutes = llFloor(seconds / 60);
        seconds -= minutes * 60;
        integer hour = llFloor(minutes / 60);
        minutes -= hour * 60;
//llOwnerSay("fs=" + (string)full_seconds + " h=" + (string)hour + " m=" + (string)minutes);

        integer is_am = hour < 12;
        if (hour > 12) hour -= 12;  // convert down to 12 hour time.
        if (hour == 0) hour = 12;  // don't show a zero in 12 hour time.

        // prepare the time update message...
        string minutes_text = (string)minutes;
        if (minutes < 10) minutes_text = "0" + minutes_text;
        string seconds_text = (string)seconds;
        if (seconds < 10) seconds_text = "0" + seconds_text;

        string meridian_string = "am";
        if (!is_am) meridian_string = "pm";

        // update our time string now that we know all the pieces.        
        global_time_string = (string)hour + ":" + minutes_text + ":"
            + seconds_text + meridian_string;
        // flip the text up above the object to show the time.
        display_colored_text();
    }

}

//credits:
// parts from lsl wiki originally?
// Enables an object to display the time for the time zone of your choosing.
// Thanks to Ben Linden for insight into timers, time and text setting.
