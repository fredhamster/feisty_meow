
// huffware script: texture shower, by fred huffhines.
//
// displays a texture from its inventory when the name is spoken in chat.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//


// global constants...

string SHOWER_SIGNATURE = "#texture_shower";
    // the notecard must begin with this as its first line for it to be
    // recognized as our configuration card.

// global variables...

string global_notecard_name;  // name of our notecard in the object's inventory.
integer response_code;  // set to uniquely identify the notecard read in progress.
list global_config_list;  // a collection of configuration parameters from our notecard.
integer global_config_index;  // allows wrap-around feature, which we don't use here.

list texture_names;  // the set of textures we've got in inventory.

integer LISTENING_CHANNEL = 1008;
    // the default listening channel is hardly ever used; the channel should
    // come from the notecard.

//////////////

// processes the variables that come from the notecard.
parse_variable_definition(string to_parse)
{
    string content;  // filled after finding a variable name.
    if ( (content = get_variable_value(to_parse, "channel")) != "") {
//        log_it("channel=" + content);
        LISTENING_CHANNEL = (integer)content;
    }
}

//////////////

// sets the viewing sides' textures to a particular inventory item.
set_texture_to_item(string inv_tex)
{
    llSetTexture(inv_tex, 1);
    llSetTexture(inv_tex, 3);
}

// sets our viewing sides back to the default texture.
reset_textures()
{
    string tex_cur = llGetTexture(0);
    set_texture_to_item(tex_cur);
}

// startup for the very early part of the object's lifetime.
initialize_phase_1()
{
    // reset our relevant variables.
    global_notecard_name = "";
    global_config_list = [];
    global_config_index = 0;
    texture_names = [];

    // request that the noteworthy library start looking for our notecard.
    request_configuration();

//    log_it("started, free mem=" + (string)llGetFreeMemory());
}

// this gets us ready to enter our active state.
initialize_phase_2()
{
    // listen for commands on the channel we're configured for.
    llListen(LISTENING_CHANNEL, "", NULL_KEY, "");
    // load all the texture names we know about.    
    integer texture_count = llGetInventoryNumber(INVENTORY_TEXTURE);
    integer indy;
    for (indy = 0; indy < texture_count; indy++)
        texture_names += llGetInventoryName(INVENTORY_TEXTURE, indy);
}

manage_those_voices(integer channel, string message)
{
    if (channel != LISTENING_CHANNEL) return;  // not for us.
    if (message == "reset-texture") {
        reset_textures();
        return;
    }
    // wasn't a command to reset, so let's see if we know it.
    integer texture_count = llGetListLength(texture_names);
    integer indy;
    for (indy = 0; indy < texture_count; indy++) {
        integer posn = llListFindList(texture_names, (message));
        if (posn >= 0) {
            // found one...
            set_texture_to_item(message);
            return;
        }
    }    
}

//////////////

// this chunk largely comes from the example noteworthy usage...

// requires noteworthy library v8.4 or better.
//////////////
// do not redefine these constants.
integer NOTEWORTHY_HUFFWARE_ID = 10010;
    // the unique id within the huffware system for the noteworthy script to
    // accept commands on.  this is used in llMessageLinked as the num parameter.
string HUFFWARE_PARM_SEPARATOR = "{~~~}";
    // this pattern is an uncommon thing to see in text, so we use it to separate
    // our commands in link messages.
string HUFFWARE_ITEM_SEPARATOR = "{|||}";
    // used to separate lists of items from each other when stored inside a parameter.
    // this allows lists to be passed as single string parameters if needed.
integer REPLY_DISTANCE = 100008;  // offset added to service's huffware id in reply IDs.
// commands available via the noteworthy library:
string READ_NOTECARD_COMMAND = "#read_note#";
    // command used to tell the script to read notecards.  needs a signature to find
    // in the card as the first parameter, and a randomly generated response code for
    // the second parameter.  the response code is used to uniquely identify a set of
    // pending notecard readings (hopefully).  the signature can be empty or missing.
    // the results will be fired back as the string value returned, which will have
    // as first element the notecard's name (or "bad_notecard" if none was found) and
    // as subsequent elements an embedded list that was read from the notecard.  this
    // necessarily limits the size of the notecards that we can read and return.
//
//////////////
// joins a list of parameters using the parameter sentinel for the library.
string wrap_parameters(list to_flatten)
{ return llDumpList2String(to_flatten, HUFFWARE_PARM_SEPARATOR); }
//////////////

// this function fires off a request to the noteworthy library via a link message.
// noteworthy will look for a notecard with our particular signature in it and
// if it finds one, it will read the configuration therein.  an empty string is
// returned if noteworthy couldn't find anything.
request_configuration()
{
    global_notecard_name = "";  // reset any previous card.
    // try to find a notecard with our configuration.
    response_code = -1 * (integer)randomize_within_range(23, 80000, FALSE);
    string parms_sent = wrap_parameters([SHOWER_SIGNATURE, response_code]);
    llMessageLinked(LINK_THIS, NOTEWORTHY_HUFFWARE_ID, READ_NOTECARD_COMMAND,
         parms_sent);
}

// processes link messages received from the noteworthy library and others.
// if the message indicates we should change states, then TRUE is returned.
integer handle_link_message(integer which, integer num, string msg, key id)
{
    if (num != NOTEWORTHY_HUFFWARE_ID + REPLY_DISTANCE)
        return FALSE;  // not for us.
    // process the result of reading the notecard.
    list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
    string notecard_name = llList2String(parms, 0);
    integer response_for = llList2Integer(parms, 1);
    if (response_for != response_code) return FALSE;  // oops, this isn't for us.
    if (notecard_name != "bad_notecard") {
        // a valid notecard has been found.
        global_notecard_name = notecard_name;  // record its name for later use.
        global_config_index = 0;  // we are starting over in the config list.
        // snag all but the first two elements for our config now.
        global_config_list = llList2List(parms, 2, -1);
        // and process the file as a set of definitions.
        process_ini_config();
        return TRUE;
    } else {
        // we hated the notecards we found, or there were none.
        log_it("We apologize; there seem to be no notecards with a first line of '"
            + SHOWER_SIGNATURE
            + "'.  We can't read any configuration until that situation improves.");
    }
    return FALSE;
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
    // say this on an unusual channel for chat if it's not intended for general public.
//    llSay(108, llGetScriptName() + "[" + (string)debug_num + "] " + to_say);
    // say this on open chat that anyone can hear.  we take off the bling for this one.
//    llSay(0, to_say);
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

// returns TRUE if the "prefix" string is the first part of "compare_with".
integer is_prefix(string compare_with, string prefix)
{ return (llSubStringIndex(compare_with, prefix) == 0); }

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

// strips the spaces off of the beginning and end of a string.
string strip_spaces(string to_strip)
{
    // clean out initial spaces.
    while (llGetSubString(to_strip, 0, 0) == " ")
        to_strip = llDeleteSubString(to_strip, 0, 0);
    // clean out ending spaces.
    while (llGetSubString(to_strip, -1, -1) == " ")
        to_strip = llDeleteSubString(to_strip, -1, -1);
    return to_strip;
}

// parses a variable definition to find the name of the variable and its value.
// this returns two strings [X, Y], if "to_split" is in the form X=Y.
list separate_variable_definition(string to_split)
{
    integer equals_indy = llSubStringIndex(to_split, "=");
    // we don't support missing an equals sign, and we don't support it as the first character.
    if (equals_indy <= 0) return [];  // no match.
    string x = llGetSubString(to_split, 0, equals_indy - 1);
    string y = llGetSubString(to_split, equals_indy + 1, -1);
    to_split = "";  // save space.
    return [ strip_spaces(x), strip_spaces(y) ];
}

// returns a non-empty string if "to_check" defines a value for "variable_name".
// this must be in the form "X=Y", where X is the variable_name and Y is the value.
string get_variable_value(string to_check, string variable_name)
{
    list x_y = separate_variable_definition(to_check);
    if (llGetListLength(x_y) != 2) return "";  // failure to parse a variable def at all.
    if (!is_prefix(llList2String(x_y, 0), variable_name)) return "";  // no match.
    return llList2String(x_y, 1);  // a match!
}

// examines all entries that we got from the notecard to see if any contain definitions.
// this is basically an INI file reader, but it uses a list instead of a file.
// ini files provide a format with multiple sections of config information, like so:
//    [section_1]
//    name1=value1
//    name2=value2 ...etc...
//    [section_2]
//    name1=value1 ...etc...
process_ini_config()
{
//    log_it("scanning notecard for variable definitions...");
    integer indy;
    integer count = llGetListLength(global_config_list);

    // iterate across the items in our configuration to look for ones that are not done yet.            
    for (indy = global_config_index; indy < count; indy++) {
        string line = llList2String(global_config_list, indy);
        // search for a section beginning.
        if (llGetSubString(line, 0, 0) == "[") {
            // we found the start of a section name.  now read the contents.
            indy++;  // skip section line.
            log_it("reading section: " + llGetSubString(line, 1, -2));
        }
        integer sec_indy;
        for (sec_indy = indy; sec_indy < count; sec_indy++) {
            // read the lines in the section.
            line = llList2String(global_config_list, sec_indy);
            if (llGetSubString(line, 0, 0) != "[") {
                // try to interpret this line as a variable setting.  this is just
                // one example of a way to handle the config file; one might instead
                // want to do something below once a whole section is read.
                parse_variable_definition(line);
            } else {
                // we're at the beginning of a new section now, so start processing its
                // configuration in the outer loop.
                indy = sec_indy - 1;  // set indy to proper beginning of section.
                global_config_index = indy;  // remember where we had read to.
                sec_indy = count + 3;  // skip remainder of inner loop.
            }
        }
    }

    global_config_index = 0;  // reset outer position if want to re-read.
}

// locates the item with "name_to_find" in the inventory items with the "type".
// a value from 0 to N-1 is returned if it's found, where N is the number of
// items in the inventory.
integer find_in_inventory(string name_to_find, integer inv_type)
{
    integer num_inv = llGetInventoryNumber(inv_type);
    if (num_inv == 0) return -1;  // nothing there!
    integer inv;
    for (inv = 0; inv < num_inv; inv++) {
        if (llGetInventoryName(inv_type, inv) == name_to_find)
            return inv;
    }
    return -2;  // failed to find it.
}
// end hufflets
//////////////

default {
    state_entry() { if (llSubStringIndex(llGetObjectName(), "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default
{
    state_entry() {
        auto_retire();  // make sure newest addition is only version of script.
        initialize_phase_1();
    }

    on_rez(integer param) { state rerun; }
    
    // reset when we see changes to our notecard configuration.
    changed(integer change) {
        if (change & CHANGED_INVENTORY) {
            llSleep(3.14159265358);  // delay to avoid interfering with upgrade.
            state rerun;
        }
    }

    // process the response from the noteworthy library.
    link_message(integer which, integer num, string msg, key id) {
        integer retval = handle_link_message(which, num, msg, id);
        if (retval) state configured;  // skip states if we were asked to.
    }
}

state configured
{
    state_entry() {
        initialize_phase_2();
    }
    
    listen(integer channel, string name, key id, string message) {
        manage_those_voices(channel, message);
    }

    link_message(integer which, integer num, string msg, key id) {
        manage_those_voices(num, msg);
    }

    // reset when we see changes to our notecard configuration.
    changed(integer change) {
        if (change & CHANGED_INVENTORY) {
            llSleep(3.14159265358);  // delay to avoid interfering with upgrade.
            state default;
        }
    }
}
