
// huffware script: searchbert menus, by fred huffhines.
//
// manages the menu system for searchbert so the main script is free to do its thing.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

//notes
// + need to have the searchbert initiate anything here.  this cannot be free range and
//   running even if the searchbert isn't in a menuable mode.
// + is the above true?

// global constants...

string MAIN_MENU_NAME = "main";  // name we use in menu list manager for the main menu.
// boilerplate text that's shown at top of main menu.  we'll add the channel info later.
string MAIN_MENU_PREFIX = "Quick help: Say '";
string MAIN_MENU_MIDDLE = "#find XYZ' in chat to locate nearby objects with 'XYZ' in their names.\nSay '";
string MAIN_MENU_SUFFIX = "#reset' to stop showing a previous search.\n\n[Matches] shows recent search results.\n[Configure] changes options.\n[Reset] clears recent results.\n[Help] dispenses an instruction notecard.";

// main menu items...
string HELP_CHOICE = "Help";
string CONFIG_CHOICE = "Configure";
string MATCHES_CHOICE = "Matches";
string RESET_CHOICE = "Reset";

// configuration menu items...
string MAX_MATCHES_CHOICE = "Max Match";
string CHAT_CHANNEL_CHOICE = "Channel";

// menu indices...
integer MAIN_MENU_INDEX = 0;  // we always do main menu as zero.
integer CONFIG_MENU_INDEX = 1;  // choices for configurable items.
integer MAX_MATCHES_MENU_INDEX = 2;  // changing the number of matches.

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

// imported interfaces...

// menu list manager link message API.
//////////////
// do not redefine these constants.
integer MENU_LIST_MANAGER_HUFFWARE_ID = 10033;
    // the unique id within the huffware system for this sensor plugin script.
string HUFFWARE_PARM_SEPARATOR = "{~~~}";
    // this pattern is an uncommon thing to see in text, so we use it to separate
    // our commands in link messages.
string HUFFWARE_ITEM_SEPARATOR = "{|||}";
    // used to separate lists of items from each other when stored inside a parameter.
    // this allows lists to be passed as single string parameters if needed.
integer REPLY_DISTANCE = 100008;  // offset added to service's huffware id in reply IDs.
string MENU_LIST_MGR_RESET = "#ml-reset#";
    // throws out any current menus and gets ready to load a new set.
string MENU_LIST_MGR_ADD_MENU = "#ml-addmenu#";
    // puts a new menu into our list.  this requires 3 parameters: menu name,
    // menu title, menu button list.
string MENU_LIST_MGR_SHOW_MENU = "#ml-shomenu#";
    // a request that a particular menu be shown.  the first parameter is the menu index.
    // the second parameter is the avatar name.  the third parameter is the avatar key.
string MENU_LIST_MGR_GIVE_ITEM = "#ml-give#";
    // brings up a gift giving menu.  parms are avatar name and avatar key.
string MENU_LIST_MGR_MODIFY_MENU = "#ml-modmenu#";
string MENU_LIST_KEEP_WORD = "KEEP";
    // replaces a menu already in the list.  this requires 4 parameters: menu index, menu name,
    // menu title, menu button list.  if a parameter is the MENU_LIST_KEEP_WORD, then that field
    // is not changed.
string MENU_LIST_MGR_CHOICE_MADE_ALERT = "#ml-event-picked";
    // alerts the driver for this script that the owner has picked a choice.  the
    // parameters include: the text of the choice, the name of the menu, the avatar name,
    // the avatar key.
//
//////////////

// we snag just enough of this interface to masquerade as it...
// card configurator link message API:
//////////////
// do not redefine these constants.
integer CARD_CONFIGURATOR_HUFFWARE_ID = 10042;
    // the unique id within the huffware system for the card configurator script to
    // accept commands on.  this is used in llMessageLinked as the num parameter.
//////////////
string CARD_CONFIG_RECEIVED_ALERT = "#cfg-event-upd#";
    // this message is sent when the configurator has found some data updates or has finished
    // reading the configuration file.
//////////////

// global variables...

integer initialized_menus_yet = FALSE;
    // records if the menus have been set up.

// additional information that's provided by the client script.
integer TALKY_CHANNEL = 0;

//////////////

// sends a new menu to be placed in the menu manager.
add_menu(string menu_name, string menu_title, list button_list)
{
    if (llGetListLength(button_list) == 0) {
        // patch this degenerate list into a bogus one.
        button_list = [ "none" ];
    }
    llMessageLinked(LINK_ROOT, MENU_LIST_MANAGER_HUFFWARE_ID, MENU_LIST_MGR_ADD_MENU,
        menu_name + HUFFWARE_PARM_SEPARATOR
        + menu_title + HUFFWARE_PARM_SEPARATOR
        + wrap_item_list(button_list));
}

// stuffs in a new version of an old menu.
replace_menu(integer index, string menu_name, string menu_title, list button_list)
{
    llMessageLinked(LINK_ROOT, MENU_LIST_MANAGER_HUFFWARE_ID, MENU_LIST_MGR_MODIFY_MENU,
        (string)index + HUFFWARE_PARM_SEPARATOR
        + menu_name + HUFFWARE_PARM_SEPARATOR
        + menu_title + HUFFWARE_PARM_SEPARATOR
        + wrap_item_list(button_list));
}

// sets up the main menu options.  if "new_now" is false, then this replaces
// the main menu rather than expecting to add a new one.
establish_main_menu(integer new_now)
{
    // menu zero: this is the main menu.
    string menu_name = "main";
    string menu_title = MAIN_MENU_PREFIX + channel_string() + MAIN_MENU_MIDDLE
        + channel_string() + MAIN_MENU_SUFFIX;
    list menu_button_list = [ MATCHES_CHOICE, CONFIG_CHOICE, RESET_CHOICE, HELP_CHOICE ];
    if (new_now) {
        add_menu(menu_name, menu_title, menu_button_list);
    } else {
        replace_menu(MAIN_MENU_INDEX, menu_name, menu_title, menu_button_list);
    }
}

// sets up all of the static menus for searchbert.
prepare_all_menus()
{
    establish_main_menu(TRUE);  // set up the main menu as a new menu.

    // configuration menu provides tasty options for changing the way searchbert behaves.
    string menu_name = CONFIG_CHOICE;  // reusing menu button from main menu for the menu name.
    string menu_title = "Configurable Options...\n[" + MAX_MATCHES_CHOICE + "] selects number of items to find.\n[" + CHAT_CHANNEL_CHOICE + "] changes listening channel for commands.\n";
    list menu_button_list = [ MAX_MATCHES_CHOICE, CHAT_CHANNEL_CHOICE ];
    add_menu(menu_name, menu_title, menu_button_list);

    menu_name = MAX_MATCHES_CHOICE;  // reusing menu button from config menu.
    menu_title = "Choose a new maximum number of items to find:";
    menu_button_list = produce_nums(17);  // ultimate number of arms available (magic constant!).
    add_menu(menu_name, menu_title, menu_button_list);

}

// if we haven't initialized yet, we'll do it now.
maybe_really_setup_menus()
{
    if (!initialized_menus_yet) {
//log_it("needed initialization still!");
        prepare_all_menus();
        initialized_menus_yet = TRUE;
        llSetTimerEvent(0);  // stop timer.
        return;
    }
}

// deals with the timer elapsing.
handle_timer_hit()
{
    // see if this timer is for initialization purposes.
    maybe_really_setup_menus();
}

// processes a message requesting our services or updating our info.  or ignore it.
handle_link_message(integer sender, integer num, string msg, key id)
{
    if ( (num != MENU_LIST_MANAGER_HUFFWARE_ID + REPLY_DISTANCE)
            && (num != SEARCHBERT_MENUS_HUFFWARE_ID) )
        return;

    // make sure we're already initialized.
    maybe_really_setup_menus();

    list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);

    if (num == MENU_LIST_MANAGER_HUFFWARE_ID + REPLY_DISTANCE) {
        if (msg == MENU_LIST_MGR_CHOICE_MADE_ALERT) {
            // now deal with the implications of the menu they chose.
            process_menu_choice(llList2String(parms, 1), llList2String(parms, 2),
                llList2String(parms, 3), llList2String(parms, 0));
        }
        return;
    }
    
    // if we got to here, it must be for our main interface methods.
//log_it("got searchbert menu request: " + msg + ".  memory left=" + (string)llGetFreeMemory());
    if (msg == SM_CONFIGURE_INFO) {
        TALKY_CHANNEL = (integer)llList2String(parms, 0);
        establish_main_menu(FALSE);
    } else if (msg == SM_POP_MAIN_MENU_UP) {
        // show the main menu for the specified avatar.
        request_menu_popup(MAIN_MENU_INDEX, llList2String(parms, 0), llList2String(parms, 1));
    }

}

request_menu_popup(integer menu_index, string av_name, string av_key)
{
    llMessageLinked(LINK_ROOT, MENU_LIST_MANAGER_HUFFWARE_ID, MENU_LIST_MGR_SHOW_MENU,
        wrap_parameters([menu_index, av_name, av_key]));
//log_it("sent menu popup...  memory left=" + (string)llGetFreeMemory());
}

// generates a list of numbers up to and including the "max".
list produce_nums(integer max)
{
    list to_return;
    integer indy;
    for (indy = 1; indy <= max; indy++) {
        to_return += (string)indy;
    }
    return to_return;
}

// returns the appropriate extra text if the channel is not zero for open chat.
string channel_string()
{
    string add_in_channel = "";
    if (TALKY_CHANNEL != 0)
        add_in_channel = "/" + (string)TALKY_CHANNEL + " ";
    return add_in_channel;
}

// handles when a menu has been clicked on.
process_menu_choice(string menu_name, string av_name, string av_key, string which_choice)
{
//log_it("into process menu " + menu_name + " for " + av_name + " who chose " + which_choice);
    if (menu_name == MAIN_MENU_NAME) {
        if (which_choice == HELP_CHOICE) {
            // see if we can find a helper notecard.
            integer indy = find_basename_in_inventory("docs", INVENTORY_NOTECARD);
            if (indy < 0) {
                llSay(0, "We're very sorry, but there do not seem to be any documentation notecards available.  There may be a better version at eepaw shop (osgrid or SL).");
            } else {
                string invname = llGetInventoryName(INVENTORY_NOTECARD, indy);
                llGiveInventory(av_key, invname);
                llWhisper(0, "Here's a copy of the help file: " + invname);
            }
            return;
        } else if (which_choice == CONFIG_CHOICE) {
            request_menu_popup(CONFIG_MENU_INDEX, av_name, av_key);
            return;
        }
        // MATCHES_CHOICE falls through for forwarding.
        // so does RESET_CHOICE.
    } else if (menu_name == CONFIG_CHOICE) {
        if (which_choice == MAX_MATCHES_CHOICE) {
            // let them pick a new maximum number of matches to find.
            request_menu_popup(MAX_MATCHES_MENU_INDEX, av_name, av_key);
            return;
        } else if (which_choice == CHAT_CHANNEL_CHOICE) {
            llSay(0, "To change the chat channel, say '" + channel_string()
                + "#channel N' where N is your new channel number.");
            return;
        }
//more config options...        

    } else if (menu_name == MAX_MATCHES_CHOICE) {
        integer max = (integer)which_choice;
        if (max >= 1) {
            // a new number of search matches has been decided upon.
            send_data_burst(["max_matches", which_choice]);
            llSay(0, "Maximum number of matches is now set to " + (string)max + ".");
            return;
        }
    }

    // cases that aren't handled get forwarded to the client script.    
    llMessageLinked(LINK_ROOT, SEARCHBERT_MENUS_HUFFWARE_ID + REPLY_DISTANCE, SM_EVENT_MENU_CLICK ,
        wrap_parameters([menu_name, which_choice, av_name, av_key]));
}

// borrowed from card configurator.
// sends the currently held data out to whoever requested it.
send_data_burst(list to_send)
{
//log_it("sending " + (string)llGetListLength(to_send) + " items");
    llMessageLinked(LINK_THIS, CARD_CONFIGURATOR_HUFFWARE_ID + REPLY_DISTANCE, CARD_CONFIG_RECEIVED_ALERT,
        wrap_parameters(["yo-updated"] + to_send));
    to_send = [];  // reset any items held.
}

// set up all the parts of the externally configured bits.
initialize()
{
//log_it("initializing.  memory left=" + (string)llGetFreeMemory());
    llSleep(0.1);  // initial pause before chatting with menu manager.

    // reset the menu manager to get it ready for our new menus.
    llMessageLinked(LINK_THIS, MENU_LIST_MANAGER_HUFFWARE_ID, MENU_LIST_MGR_RESET, "");

    // snooze a bit to allow our helper to wake up.
    llSetTimerEvent(0.34);  // snooze until helper is ready.
    initialized_menus_yet = FALSE;
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

//////////////

// joins a list of parameters using the parameter sentinel for the library.
string wrap_parameters(list to_flatten)
{ return llDumpList2String(to_flatten, HUFFWARE_PARM_SEPARATOR); }

string wrap_item_list(list to_wrap)
{ return llDumpList2String(to_wrap, HUFFWARE_ITEM_SEPARATOR); }

//////////////

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

// returns the index of the first occurrence of "pattern" inside
// the "full_string".  if it is not found, then a negative number is returned.
integer find_substring(string full_string, string pattern)
{ return llSubStringIndex(llToLower(full_string), llToLower(pattern)); }

// returns TRUE if the "prefix" string is the first part of "compare_with".
integer is_prefix(string compare_with, string prefix)
{ return find_substring(compare_with, prefix) == 0; }

// looks for an inventory item with the same prefix as the "basename_to_seek".
integer find_basename_in_inventory(string basename_to_seek, integer inv_type)
{
    integer num_inv = llGetInventoryNumber(inv_type);
    if (num_inv == 0) return -1;  // nothing there!
    integer indy;
    for (indy = 0; indy < num_inv; indy++) {
        if (is_prefix(llGetInventoryName(inv_type, indy), basename_to_seek))
            return indy;
    }
    return -2;  // failed to find it.
}

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

// end hufflets.
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
        initialize();
    }
    
    on_rez(integer parm) { llResetScript(); }
    
    timer() { handle_timer_hit(); }

    link_message(integer sender, integer num, string msg, key id) {
        handle_link_message(sender, num, msg, id);
    }

}

