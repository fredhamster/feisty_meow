
// huffware script: mediatron control, by fred huffhines.
//
// controls the mediatron device and manages menus for the user.
// when buttons are clicked, this is the script that arranges for things to
// happen.  this script also processes the spoken commands from the user.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

// configurable constants...

integer MEDIATRON_CHANNEL = 1008;
    // fairly arbitrary channel for listening.

integer ONLY_OWNER = FALSE;
    // only listen to the owner's chats?

vector TEXT_COLOR = <0.5, 0.7, 0.92>;
    // the color that the current chapter is displayed in.

string LATEX_DRIVER_URL = "http://universalbuilds.com/tib/gif.php?l=";
    // the URL for turning tex code into tibetan and english bitmaps.

// global constants that really should stay constant...

////////
// link ids for flinging messages at them.
//----these are useless until the OAR link number bug is fixed.
//integer ROOT_PRIM_LINK = 1;  // by SL conventions.
//integer MAIN_VIEWSCREEN_LINK = 2;  // the next prim after the root is our viewer.
//integer NEXT_BUTTON_LINK = 3;
//integer PREVIOUS_BUTTON_LINK = 4;
//integer UHHH_BUTTON_LINK = 5;
//integer MENU_BUTTON_LINK = 6;
//integer BACKGROUND_PLATE_LINK = 7;  // the blank whitish plank underneath the main viewscreen.
////////

integer DEFAULT_CHANNEL = 1008;  // this must remain unchanged so we can dynamically modify help.

// commands that can begin the line of text spoken on our special channel.
// these are also used for interchange with sub-prims, such as when a button is pushed.
string TIBETAN_CHUNK = "tib";
string ENGLISH_CHUNK = "en";
string NEW_LINE_CMD = "line";
string SHOW_BUFFER_CMD = "go";

string NEXT_PAGE_CMD = "next";
string PREVIOUS_PAGE_CMD = "prev";

string RESET_MEDIATRON_CMD = "Reset";
string CLEAR_MEDIATRON_CMD = "Clear";

string OPEN_MENU_CMD = "menu";
string CHAPTER_PICKER_CMD = "chapter";

string CHANNEL_CHANGE_CMD = "channel";
string DRIVER_WEB_SITE_CMD = "urldriver";
string LOAD_WEB_PAGE_CMD = "loadweb";

string TEXT_COLOR_CHANGE_CMD = "textcolor";

string HELP_CMD = "Help";

// config / control menus that we support.
string STATUS_ITEM = "Status";
string AUTHORIZE_ITEM = "Authorize";
string TEXT_COLOR_ITEM = "Text Color";
string CHANNEL_ITEM = "Channel";
string WEB_SITE_ITEM = "Driver URL";

// API for the viewscreen blitter library...
//////////////
integer VIEWSCREEN_BLITTER_HUFFWARE_ID = 10027;
    // unique ID for the viewscreen services.
string HUFFWARE_PARM_SEPARATOR = "{~~~}";
    // this pattern is an uncommon thing to see in text, so we use it to separate
    // our commands in link messages.
string HUFFWARE_ITEM_SEPARATOR = "{|||}";
    // used to separate lists of items from each other when stored inside a parameter.
    // this allows lists to be passed as single string parameters if needed.
integer REPLY_DISTANCE = 100008;  // offset added to service's huffware id in reply IDs.
//
string SHOW_URL_COMMAND = "#shurl";
    // requests the viewscreen to show a particular URL as its texture.
string RESET_VIEWSCREEN_COMMAND = "#shrz";
    // resets the viewscreen script to the default state.
string SHOW_TEXTURE_COMMAND = "#shtex";
    // displays a texture on the prim.  the first parameter is the texture key or name.
//////////////

// API for moving between the active notecards.
//////////////
// do not redefine these constants.
integer SLATE_READER_HUFFWARE_ID = 10028;
    // the unique id within the huffware system for the noteworthy script to
    // accept commands on.  this is used in llMessageLinked as the num parameter.
//////////////
string RESET_SLATE_READER_COMMAND = "#rsslt";
    // causes the notecard information to be forgotten and the script restarted.
string SR_GET_INFORMATION_COMMAND = "#infy";
    // used by clients to ask for information about the current number of notecards
    // available, and their names.  this information is sent back on the huffware ID
    // plus the reply distance.  first parm is the number, and the rest are the names.
string SR_PLAY_CARD_COMMAND = "#playvo";
    // picks a particular notecard for reading and send the notecard's contents in a
    // series of link messages, using this command and the reply distance.  there are
    // two parameters: an integer for the notecard number to read (from 0 through the
    // number of notecards - 1) and the link number to send the messages to.
//////////////

// the button pushing API.
//////////////
integer BUTTON_PUSHER_HUFFWARE_ID = 10029;
    // a unique ID within the huffware system for this script.
//////////////
string BUTTON_PUSHED_ALERT = "#btnp";
    // this event is generated when the button is pushed.  the number parameter will be
    // the huffware id plus the reply distance.  the id parameter in the link message will
    // contain the name of the button that was pushed.
//////////////

// menutini link message API...
//////////////
// do not redefine these constants.
integer MENUTINI_HUFFWARE_ID = 10009;
    // the unique id within the huffware system for the jaunt script to
    // accept commands on.  this is used in llMessageLinked as the num parameter.
string SHOW_MENU_COMMAND = "#menu#";
    // the command that tells menutini to show a menu defined by parameters
    // that are passed along.  these must be: the menu name, the menu's title
    // (which is really the info to show as content in the main box of the menu),
    // the wrapped list of commands to show as menu buttons, the menu system
    // channel's for listening, and the key to listen to.
    // the reply will include: the menu name, the choice made and the key for
    // the avatar.
//////////////

// global variables...

string buffer_text;  // the buffer being accumulated for display.

integer page_number;  // current page shown: ranges from zero to number of notecards - 1.

integer maximum_notecards;  // the maximum notecards that can be shown.

list notecard_choices;  // the chapters that can be chosen.

integer listening_handle;  // tracks our listening so we can change channels.

set_viewscreen_texture()
{
    // get the video texture they want to use for the parcel.
    key texture_key = llList2Key(llParcelMediaQuery([PARCEL_MEDIA_COMMAND_TEXTURE]), 0);
    if (texture_key == NULL_KEY) {
        // there wasn't one set yet.  we can't do much about that here.  we'll use the
        // last inventory texture instead, which usually will be our proper image.
        texture_key = llGetInventoryKey(llGetInventoryName(INVENTORY_TEXTURE,
            llGetInventoryNumber(INVENTORY_TEXTURE) - 1));
        if (texture_key == NULL_KEY) return;  // now we're hosed; there's no texture.
    }
    // set the main viewer prim to the chosen texture...
    llMessageLinked(LINK_SET, VIEWSCREEN_BLITTER_HUFFWARE_ID,
        SHOW_TEXTURE_COMMAND, (string)texture_key);
}

// checks the parcel's media texture to make sure it's set.  if it's not, we reset it to
// our favorite version.
fix_parcel_media_texture()
{
    // get the video texture they want to use for the parcel.
    key texture_key = llList2Key(llParcelMediaQuery([PARCEL_MEDIA_COMMAND_TEXTURE]), 0);
    if (texture_key == NULL_KEY) {
        // there wasn't one set yet, so we need to fix that.  we'll use the last inventory
        // texture, which usually will be our proper image.  this is a distinct chore from
        // setting the viewscreen's texture, since we don't know that we have the capability
        // to change the parcel media texture yet.
        texture_key = llGetInventoryKey(llGetInventoryName(INVENTORY_TEXTURE,
            llGetInventoryNumber(INVENTORY_TEXTURE) - 1));
        if (texture_key != NULL_KEY) {
            llParcelMediaCommandList([PARCEL_MEDIA_COMMAND_TEXTURE, texture_key]);
            if (llGetLandOwnerAt(llGetPos()) != llGetOwner()) {
                log_it("There is a problem with the parcel media: "
                    + llGetObjectName() + " is not owned by the parcel owner.");
            } else {
                log_it("No media texture found for video on this parcel.  Using default texture: "
                    + (string)texture_key);
            }
        } else {
            log_it("The object does not seem to contain any textures; we need at "
                + "least one for a default viewscreen image.");
        }
    }
}

// asks the slate reader to find out what cards there are for us.
reload_notecards()
{
    // now we find out what's really present so we can offer a menu.
    llMessageLinked(LINK_THIS, SLATE_READER_HUFFWARE_ID,
        SR_GET_INFORMATION_COMMAND, "");
}

// fixes the viewscreen face and parcel media texture, if needed, and gets the mediatron
// ready for commands.
initialize_mediatron()
{
    fix_parcel_media_texture();
    set_viewscreen_texture();
    llSetText("", TEXT_COLOR, 0.0);
    page_number = -1;
    reload_notecards();

    // listening for commands from the user.    
    listening_handle = llListen(MEDIATRON_CHANNEL, "", NULL_KEY, "");

    llSay(0, "Now listening for commands on channel /" + (string)MEDIATRON_CHANNEL);
}

// locates the space boundaries between words and returns just a list of the words,
// with no spaces remaining.
list break_on_spaces(string msg)
{
    if (!llStringLength(msg)) return [];  // no length means no list.
    integer indy;
    list to_return;
    for (indy = 0; indy < llStringLength(msg); indy++) {
        integer space_posn = llSubStringIndex(msg, " ");
        if (space_posn < 0) {
            // no spaces left, so use the whole string.
            to_return += [ msg ];
            indy = 2 * llStringLength(msg);
        } else {
            // we found a space.  chop out the first part to add to the list and
            // keep the second part for further scanning.
            to_return += [ llGetSubString(msg, 0, space_posn - 1) ];
            // we also trash any additional spaces that were after the one we found.
            msg = llStringTrim(llGetSubString(msg, space_posn + 1, -1), STRING_TRIM);
            indy = -1;  // start over at top of string.
        }
    }
    return to_return;
}

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

// handles when a button on the device has been clicked.
process_button_clicks(string msg, key id)
{
    if (msg != BUTTON_PUSHED_ALERT) return;  // uhhhh, not sure what that was.
    if (id == NEXT_PAGE_CMD) {
        hear_voices(FALSE, "self", llGetOwner(), id);
    } else if (id == PREVIOUS_PAGE_CMD) {
        hear_voices(FALSE, "self", llGetOwner(), id);
    } else if (id == CHAPTER_PICKER_CMD) {
        list pruned_choices;
        string list_as_block;
        integer indy;
        for (indy = 0; indy < llGetListLength(notecard_choices); indy++) {
            string curr_name = prune_notecard_name(llList2String(notecard_choices, indy));
            pruned_choices += [ curr_name ];
            list_as_block += (string)(indy + 1) + ". " + curr_name;
            if (indy < llGetListLength(notecard_choices) - 1) list_as_block += "\n";
        }
        simply_display_menu(CHAPTER_PICKER_CMD, "Pick the Chapter to display...\n" + list_as_block,
            pruned_choices);
    } else if (id == OPEN_MENU_CMD) {
        simply_display_menu(OPEN_MENU_CMD, "TibSlate Mediatron Control Menu",
            [ HELP_CMD, STATUS_ITEM, AUTHORIZE_ITEM, TEXT_COLOR_ITEM, CHANNEL_ITEM,
                WEB_SITE_ITEM, CLEAR_MEDIATRON_CMD, RESET_MEDIATRON_CMD ]);
    }
}

string bool_string_for_int(integer truth)
{
    if (truth) return "true"; else return "false";
}

tell_not_authorized()
{
    llSay(0, "The operation you have chosen can only be performed by the owner.");
}

// handles the response message when the user chooses a button.
react_to_menu(string msg, key id)
{
    list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
    string menu_name = llList2String(parms, 0);
    string which_choice = llList2String(parms, 1);
    key av_key = llList2String(parms, 2);
    if (ONLY_OWNER && (av_key != llGetOwner())) {
        tell_not_authorized();
        return;
    }
//log_it("you clicked " + which_choice + " item for av " + av_key);

    if (menu_name == CHAPTER_PICKER_CMD) {
        // find the index of the choice if we can, and make the choice be the new page.
        page_number = find_similar_in_list(notecard_choices, which_choice);
        play_chapter();
        return;
    } else if (menu_name == OPEN_MENU_CMD) {
        // show the main control menu.
        if (which_choice == STATUS_ITEM) {
            llSay(0, "Status...\n\tMemory Free " + (string)llGetFreeMemory()
                + "\n\tListening Channel " + (string)MEDIATRON_CHANNEL
                + "\n\tOwner Only? " + bool_string_for_int(ONLY_OWNER));
            return;
        } else if (which_choice == AUTHORIZE_ITEM) {
            if (av_key != llGetOwner()) {
                // this option isn't allowed for anyone but the owner, ever.
                tell_not_authorized();
                return;
            }
            simply_display_menu(AUTHORIZE_ITEM, "Authorization required to use device...",
                [ "Any User", "Only Owner" ]);
            return;
        } else if (which_choice == TEXT_COLOR_ITEM) {
            llSay(0, "To change the text color, say\n\t/"
                + (string)MEDIATRON_CHANNEL + " " + TEXT_COLOR_CHANGE_CMD
                + " <r, g, b>\nwhere r, g, and b are the red, green and blue components of the color.\n"
                + "Note that r, g & b can range from 0 to 1 inclusive.");
            return;
        } else if (which_choice == CHANNEL_ITEM) {
            llSay(0, "To change the channel of the viewer, say\n\t/"
                + (string)MEDIATRON_CHANNEL + " " + CHANNEL_CHANGE_CMD
                + " N\nwhere N is a number for the new channel.");
            return;
        } else if (which_choice == WEB_SITE_ITEM) {
            llSay(0, "Current driver web URL: " + LATEX_DRIVER_URL
                + "\nTo replace this site, say:\n\t/"
                + (string)MEDIATRON_CHANNEL + " " + DRIVER_WEB_SITE_CMD
                + " newURL\nNote that this site must be capable of accepting LaTex code and\n"
                + "converting that into simulator compatible bitmaps.");
            return;
        } else {
            // handle all the others by just passing this on to the hear voices function.
            // if it's a known command, we handle it.  otherwise we bounce it off the couch.
            hear_voices(FALSE, "self", llGetOwner(), which_choice);
            return;
        }
    // the remainder of these handle the menu button's menu options...
    } else if (menu_name == AUTHORIZE_ITEM) {
        // they chose an option for authorization, so see if it was for only the owner.
        ONLY_OWNER = is_prefix(which_choice, "only");
        llSay(0, "now, Owner Only = " + bool_string_for_int(ONLY_OWNER));
        return;
    }

log_it("i do not understand item " + which_choice + " in "+ menu_name);
}

// processes the responses from the slate reader.
handle_slate_items(string msg, key id)
{
    if (msg == SR_PLAY_CARD_COMMAND) {
        // we have gotten some data from our reader.
        hear_voices(FALSE, "self", llGetOwner(), id);
    } else if (msg == SR_GET_INFORMATION_COMMAND) {
        list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
        maximum_notecards = llList2Integer(parms, 0);
//log_it("got info response, num notes=" + (string)maximum_notecards);
        notecard_choices = llList2List(parms, 1, -1);
    }
}

// strips off any numerical info to make a more useful notecard name.
string prune_notecard_name(string note_name)
{
    // we look at the name and remove any numbers in front, so we can avoid cluttering up the
    // user facing text with the numbers that are used for ordering the notecards.
    list rip_these = [ "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", " ", "." ];
    while (llListFindList(rip_these, [ llGetSubString(note_name, 0, 0) ]) >= 0) {
        // found a number or space or period.  rip it off.
        note_name = llDeleteSubString(note_name, 0, 0);
    }
    return note_name;
}

show_chapter_name()
{
    string note_name = prune_notecard_name(llList2String(notecard_choices, page_number));
    llSetText("Page #" + (string)(page_number + 1)
        + " -- " + note_name, TEXT_COLOR, 1.0);
}

// shows the contents of the current page's notecard on the viewscreen.
play_chapter()
{
    show_chapter_name();
    llMessageLinked(LINK_THIS, SLATE_READER_HUFFWARE_ID,
        SR_PLAY_CARD_COMMAND, wrap_parameters([page_number, LINK_THIS]));
}

// tell the viewscreen to display this possible tibetan.
// spoken reports whether it's a user speaking or not.  if spoken is false, then
// the voice is internal from getting notecard info or button clicks.
hear_voices(integer spoken, string name, key id, string msg)
{
    msg = llStringTrim(msg, STRING_TRIM);  // toss extra spaces.
    if (msg == CLEAR_MEDIATRON_CMD) {
        // wipe out anything on the screen.
        set_viewscreen_texture();
        buffer_text = "";  // toss out any partial buffer also.
        if (spoken) llSay(0, "Clearing viewscreen.");
        page_number = -1;  // reset page to the spot before the first.
        llSetText("", TEXT_COLOR, 0.0);
        return;
    } else if ( (msg == RESET_MEDIATRON_CMD) && (id == llGetOwner()) ) {
        llMessageLinked(LINK_SET, VIEWSCREEN_BLITTER_HUFFWARE_ID,
            RESET_VIEWSCREEN_COMMAND, "");
        llMessageLinked(LINK_THIS, SLATE_READER_HUFFWARE_ID,
            RESET_SLATE_READER_COMMAND, "");
        if (spoken) llSay(0, "Resetting scripts.");
        llResetScript();
    } else if (is_prefix(msg, CHANNEL_CHANGE_CMD)) {
        integer new_channel = (integer)llGetSubString(msg, llStringLength(CHANNEL_CHANGE_CMD), -1);
        llSay(0, "New channel is " + (string)new_channel);
        MEDIATRON_CHANNEL = new_channel;
        llListenRemove(listening_handle);
        listening_handle = llListen(MEDIATRON_CHANNEL, "", NULL_KEY, "");
        return;
    } else if (is_prefix(msg, TEXT_COLOR_CHANGE_CMD)) {
        vector new_col = (vector)llGetSubString(msg, llStringLength(TEXT_COLOR_CHANGE_CMD), -1);
        llSay(0, "New text color is " + (string)new_col);
        TEXT_COLOR = new_col;
        show_chapter_name();
        return;
    } else if (is_prefix(msg, DRIVER_WEB_SITE_CMD)) {
        string new_url = llGetSubString(msg, llStringLength(DRIVER_WEB_SITE_CMD), -1);
        llSay(0, "New driver URL for LaTex: " + (string)new_url);
        LATEX_DRIVER_URL = new_url;
        return;
    } else if (is_prefix(msg, NEXT_PAGE_CMD)) {
        page_number++;
        if (page_number >= maximum_notecards) {
            page_number = 0;
        }
        play_chapter();
        return;
    } else if (is_prefix(msg, PREVIOUS_PAGE_CMD)) {
        page_number--;
        if (page_number < 0) {
            page_number = maximum_notecards - 1;
        }
        show_chapter_name();
        llMessageLinked(LINK_THIS, SLATE_READER_HUFFWARE_ID,
            SR_PLAY_CARD_COMMAND, wrap_parameters([page_number, LINK_THIS]));
        return;
    } else if (is_prefix(msg, HELP_CMD)) {
        provide_help();
        return;
    } else if (is_prefix(msg, LOAD_WEB_PAGE_CMD)) {
        string new_url = llGetSubString(msg, llStringLength(LOAD_WEB_PAGE_CMD), -1);
        llSay(0, "Loading web site: " + (string)new_url);
        llMessageLinked(LINK_SET, VIEWSCREEN_BLITTER_HUFFWARE_ID, SHOW_URL_COMMAND, new_url);
        buffer_text = "";  // clear the buffer out.
    }

    // now we try to process the screen buffer commands which we suspect
    // are in the text that was spoken.
    list sentence = break_on_spaces(msg);
    string first_word = llList2String(sentence, 0);
//log_it("first word is: " + first_word);
    string remainder;
    integer indo;
    for (indo = 1; indo < llGetListLength(sentence); indo++) {
        if (indo != 1) remainder += " ";
        remainder += llList2String(sentence, indo);
    }
//log_it("remainder is: " + remainder);

    if (first_word == TIBETAN_CHUNK) {
        if (spoken) llSay(0, "Added Tibetan: " + remainder);
        buffer_text += "{\\tib " + remainder + "}{ }";        
    } else if (first_word == ENGLISH_CHUNK) {
        if (spoken) llSay(0, "Added English: " + remainder);
        buffer_text += "{" + remainder + " }";        
    } else if (first_word == NEW_LINE_CMD) {
        if (spoken) llSay(0, "Inserted blank line.");
        buffer_text += "{\\\\}";
    } else if (first_word == SHOW_BUFFER_CMD) {
        if (spoken) llSay(0, "Displaying buffered screen.");
        // reset text label since that's no longer our page.
        if (spoken) llSetText("", TEXT_COLOR, 0.0);
        string url = LATEX_DRIVER_URL + buffer_text;
        llMessageLinked(LINK_SET, VIEWSCREEN_BLITTER_HUFFWARE_ID, SHOW_URL_COMMAND, url);
        buffer_text = "";  // clear the buffer out.
    } else {
        // treat this as english.
//mostly redundant!
        if (spoken) llSay(0, "Added English: " + first_word + " " + remainder);
        buffer_text += "{" + first_word + " " + remainder + "}";
        // if we're assuming this is english, we will plug in a blank line afterwards.
        // otherwise a text document will look right awful.
        buffer_text += "{\\\\}";
    }
}

// hands out any notecards with help in their names.
provide_help()
{
    integer count = llGetInventoryNumber(INVENTORY_NOTECARD);
    integer indy;
    list help_cards = [];
    for (indy = 0; indy < count; indy++) {
        string note_name = llGetInventoryName(INVENTORY_NOTECARD, indy);
        if (find_substring(note_name, "help") >= 0)
            help_cards += [ note_name ];
    }

    if (llGetListLength(help_cards)) llOwnerSay("Here is the built-in help file...");
    else llOwnerSay("There are currently no help notecards; sorry.");
    // iterate on helper cards and give them out.
    count = llGetListLength(help_cards);
    for (indy = 0; indy < count; indy++) {
        string note_name = llList2String(help_cards, indy);
        llGiveInventory(llGetOwner(), note_name);
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

//////////////

// joins a list of parameters using the parameter sentinel for the library.
string wrap_parameters(list to_flatten)
{ return llDumpList2String(to_flatten, HUFFWARE_PARM_SEPARATOR); }

// joins a list of sub-items using the item sentinel for the library.
string wrap_item_list(list to_wrap)
{ return llDumpList2String(to_wrap, HUFFWARE_ITEM_SEPARATOR); }

// returns TRUE if the "prefix" string is the first part of "compare_with".
integer is_prefix(string compare_with, string prefix)
{ return (llSubStringIndex(llToLower(compare_with), llToLower(prefix)) == 0); }

// returns the index of the first occurrence of "pattern" inside
// the "full_string".  if it is not found, then a negative number is returned.
integer find_substring(string full_string, string pattern)
{ return llSubStringIndex(llToLower(full_string), llToLower(pattern)); }

// locates the string "text" as a partial match in the list to "search_in".
integer find_similar_in_list(list search_in, string text)
{ 
    integer len = llGetListLength(search_in);
    integer i; 
    for (i = 0; i < len; i++) { 
        if (find_substring(llList2String(search_in, i), text) >= 0)
            return i; 
    } 
    return -1;
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
        initialize_mediatron();
    }
    
    listen(integer chan, string name, key id, string msg) {
        if (chan != MEDIATRON_CHANNEL) return;  // how are we even hearing that?
        if (ONLY_OWNER && (id != llGetOwner())) return;  // unauthorized talker.
        hear_voices(TRUE, name, id, msg);
    }
    
    on_rez(integer start_parm) {
        llResetScript();
    }
    
    changed(integer change) {
        if (change & CHANGED_INVENTORY) {
            reload_notecards();
        }
    }
    
    link_message(integer sender, integer num, string msg, key id) {
        if (num == BUTTON_PUSHER_HUFFWARE_ID + REPLY_DISTANCE) {
            process_button_clicks(msg, id);
            return;
        }
        if (num == MENUTINI_HUFFWARE_ID + REPLY_DISTANCE) {
            react_to_menu(msg, id);
        }

        if (num != SLATE_READER_HUFFWARE_ID + REPLY_DISTANCE) return;  // not for us.
        handle_slate_items(msg, id);
    }
}

