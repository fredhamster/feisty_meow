
// huffware script: time zone picker, by fred huffhines.
//
// allows the user to select a different time zone than the default.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

// constants...

// secret codes to control the time zone.
integer SECRET_TIME_ZONE_ADJUSTER_ID = -2091823;
string TIME_ZONE_ADJUST_COMMAND = "tzadjust";

// requires menutini v3.9 or better...
//////////////
// do not redefine these constants.
integer MENUTINI_HUFFWARE_ID = 10009;
    // the unique id within the huffware system for the jaunt script to
    // accept commands on.  this is used in llMessageLinked as the num parameter.
string HUFFWARE_PARM_SEPARATOR = "{~~~}";
    // this pattern is an uncommon thing to see in text, so we use it to separate
    // our commands in link messages.
string HUFFWARE_ITEM_SEPARATOR = "{|||}";
    // used to separate lists of items from each other when stored inside a parameter.
    // this allows lists to be passed as single string parameters if needed.
integer REPLY_DISTANCE = 100008;  // offset added to service's huffware id in reply IDs.
// commands available via menutini:
string SHOW_MENU_COMMAND = "#menu#";
    // the command that tells menutini to show a menu defined by parameters
    // that are passed along.  these must be: the menu name, the menu's title
    // (which is really the info to show as content in the main box of the menu),
    // the wrapped list of commands to show as menu buttons, the menu system
    // channel's for listening, and the key to listen to.
//
//////////////
// joins a list of sub-items using the item sentinel for the library.
string wrap_item_list(list to_wrap)
{ return llDumpList2String(to_wrap, HUFFWARE_ITEM_SEPARATOR); }
//
//////////////

integer random_channel() { return -(integer)(llFrand(40000) + 20000); }

simply_display_menu(string menu_name, string title, list buttons)
{
    integer menu_channel = random_channel();
    key listen_to = llGetOwner();
    llMessageLinked(LINK_THIS, MENUTINI_HUFFWARE_ID, SHOW_MENU_COMMAND,
        menu_name + HUFFWARE_PARM_SEPARATOR
        + title + HUFFWARE_PARM_SEPARATOR
        + wrap_item_list(buttons) + HUFFWARE_PARM_SEPARATOR
        + (string)menu_channel + HUFFWARE_PARM_SEPARATOR
        + (string)listen_to);
}

// handles the response message when the user chooses a button.
react_to_menu(string menu_name, string av_key, string which_choice)
{
    log_it("You selected a time zone offset of " + which_choice);
    llMessageLinked(LINK_SET, SECRET_TIME_ZONE_ADJUSTER_ID, TIME_ZONE_ADJUST_COMMAND, which_choice);
}

// processes a message from a link.  some of this must be handled
// by the driver script that handles our configuration.
handle_link_message(integer sender, integer num, string msg, key id)
{
    if ( (num != MENUTINI_HUFFWARE_ID + REPLY_DISTANCE)
        || (msg != SHOW_MENU_COMMAND) ) return;  // not for us.

//log_it("menu reply: sndr=" + (string)sender + " num=" + (string)num + " msg=" + msg + " id=" + (string)id);
    list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
    string menu_name = llList2String(parms, 0);
    string which_choice = llList2String(parms, 1);
    key av_key = llList2String(parms, 2);
    react_to_menu(menu_name, av_key, which_choice);
}

//////////////
// from hufflets...
//
integer debug_num = 0;

// a debugging output method.  can be disabled entirely in one place.
log_it(string to_say)
{
    debug_num++;
    // tell this to the owner.    
    llOwnerSay(llGetScriptName() + "[" + (string)debug_num + "] " + to_say);
    // say this on open chat, but use an unusual channel.
//    llSay(108, llGetScriptName() + "[" + (string)debug_num + "] " + to_say);
}

//
// end hufflets
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
        auto_retire();  // make sure newest addition is only version of script.
    }

    touch_start(integer touchers) {
        if (llDetectedKey(0) != llGetOwner()) return;  // only listen to owner for setting time zone.
        simply_display_menu("tz", "Select your time zone offset from GMT:\nGMT is 0, EST is -5, PST is -8, etc.",
            [ -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 ] );
    }

    link_message(integer sender, integer num, string msg, key id)
    { handle_link_message(sender, num, msg, id); }
}
