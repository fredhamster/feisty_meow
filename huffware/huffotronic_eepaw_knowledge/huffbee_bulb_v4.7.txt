
// huffware script: "huffbee bulb", by fred huffhines
//
// inspired by the higbee bulb script available in open source, but provides a few
// different functions.  this accepts a click to turn on and off, but it also listens
// to a special command to hear requests from linked objects.  this also shuts off
// at dusk and turns on at dawn.
// this script uses the party culiar script to get the particle system created,
// and hence relies on a notecard to define the lamp's particles.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

integer DUSK_CHECKING_PERIOD = 240;
    // check for dusk this frequently.  in seconds, so 240 = 4 minutes.

integer SECRET_LIGHT_BULB_ILLUMINATI_CODE = -14058;
    // a semi-secret code that is used in a linked message when some other script
    // wants the lamp to turn on or off.

float BULB_GLOW_LEVEL = 0.5;  // how much glow should the bulb have when it's lit?

integer CHECK_FOR_DUSK = TRUE;  // true if this lamp should be driven by daylight.

integer PARTICLE_FIX_COG = 7;  // every N timer hits, we'll fix our particles.

// party culiar link message API.
//////////////
integer PARTYCULIAR_HUFFWARE_ID = 10018;
    // a unique ID within the huffware system for this script.
string HUFFWARE_PARM_SEPARATOR = "~~~";
    // three tildes is an uncommon thing to have otherwise, so we use it to separate
    // our commands in linked messages.
//
string PARTYCULIAR_POWER_COMMAND = "#powrpcl";
    // tells the particle system to either turn on or off.  the single parameter is
    // either "on", "1", "true", or "off", "0", "false".
//////////////

// globals.

integer particle_cog = 0;

//////////////

// asks the particle system script for a favor, given by the command and the
// parameters.
send_partyculiar_request(string cmd, string params)
{ llMessageLinked(LINK_THIS, PARTYCULIAR_HUFFWARE_ID, cmd, params); }

// shuts down the lamp.  there is no coming back from this method,
// since it resets the script.
turn_off_lamp()
{
    turn_off_particles();
    llResetScript();
}

turn_off_particles() { send_partyculiar_request(PARTYCULIAR_POWER_COMMAND, "off"); }

turn_on_particles() { send_partyculiar_request(PARTYCULIAR_POWER_COMMAND, "on"); }

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
    state_entry()
    {
        auto_retire();
        turn_off_particles();
        // reset the bulb to a fairly dark color with no glow and no light.
        llSetPrimitiveParams([
            PRIM_COLOR, ALL_SIDES, <0.42, 0.42, 0.42>, 0.5,
            PRIM_FULLBRIGHT, ALL_SIDES, FALSE,
            PRIM_GLOW, ALL_SIDES, 0.0,
            PRIM_POINT_LIGHT, FALSE, <0, 0, 0>, 1.0, 10.0, 0.25
            ]);        
        llSetTimerEvent(DUSK_CHECKING_PERIOD);
    }
    on_rez(integer parm) {
        state default;
    }
    timer() {
        vector sun = llGetSunDirection();
        if (CHECK_FOR_DUSK && (sun.z <= 0) )
            state PowerUp;  // turn on at dusk.
    }
    touch_start(integer total_number) {
        state PowerUp;
    }

    // handle requests to ignite the lamp.    
    link_message(integer sender, integer num, string str, key id) {
        // we only support one message really, which is to turn on/off.
        if (num == SECRET_LIGHT_BULB_ILLUMINATI_CODE) {
            state PowerUp;  // we know we aren't lit yet, so get that way.
        }
    }
}
        
state PowerUp
{
    state_entry() {
        // light the bulb, with a full color / full bright assault on
        // the sense, using real light and a glow effect.
        llSetPrimitiveParams([
            PRIM_COLOR, ALL_SIDES, <1, 1, 1>, 0.5,
            PRIM_FULLBRIGHT, ALL_SIDES, TRUE,
            PRIM_GLOW, ALL_SIDES, BULB_GLOW_LEVEL,
            PRIM_POINT_LIGHT, TRUE, <.8, .8, .3>, 1.0, 20.0, .9
            ]);
        turn_on_particles();
        llSetTimerEvent(DUSK_CHECKING_PERIOD);
    }
    on_rez(integer parm) {
        // if we rezzed in this state, that's a mistake.  the lamp is intended
        // to start in a dark state.
        turn_off_lamp();
    }
    timer() {
        particle_cog++;
        if (particle_cog >= PARTICLE_FIX_COG) {
            // periodically refresh the particle system.
            turn_on_particles();
            particle_cog = 0;  // reset cog count.
        }
        // also periodically check if we should shut off.
        vector sun = llGetSunDirection();
        if (CHECK_FOR_DUSK && (sun.z > 0) ) {
            // turn off at dawn.
            turn_off_lamp();
        }
    }
    
    touch_start(integer total_number)
    {
        turn_off_lamp();
    }
    
    link_message(integer sender, integer num, string str, key id) {
        // we only support one message really, which is to turn on/off.
        if (num == SECRET_LIGHT_BULB_ILLUMINATI_CODE) {
            turn_off_lamp();  // in this state, we are already lit, so get real dark.
        }
    }
}
