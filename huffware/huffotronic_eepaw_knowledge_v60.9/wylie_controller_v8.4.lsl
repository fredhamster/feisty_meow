
// huffware script: wylie controller, by fred huffhines and wam7c macchi.
//
// accepts wylie input and distributes the command to the various parts of the letter.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

// configurable values...  the settings below are just defaults.

integer LISTENING_CHANNEL = 0;  // the channel where we'll listen for commands.
integer ONLY_LISTEN_TO_OWNER = FALSE;  // if true, only the owner's voice counts.
integer SHOW_CHANNEL = TRUE;  // puts a title over our heads if true.
vector TITLE_COLOR = <1.0, 1.0, 1.0>;  // the color the title should be.

integer DEBUGGING = FALSE;  // if this is true, then the debugging will be noisy.

// constants...  do not change these.

integer STARTUP_CODE_NOTECARD_RETRIES = 14042;
    // secret code used to launch the wylie writers with many notecard retries.

// channels for speech / link messages.
integer LOWER_VOWEL_CHANNEL = 11008;
integer CONSONTANT_CHANNEL  = 21008;
integer UPPER_VOWEL_CHANNEL = 31008;

string RESET_TEXTURE_WORD = "reset-texture";
    // tells the letter blocks to go back to default texture.
string BROADCAST_WORD = "bcast";
    // used to tell many wylie writers to do something at the same time.

string WYLIE_NOTEWORTHY_SIGNATURE = "#wylie";
    // the notecard must begin with this as its first line for it to be
    // recognized as our configuration card.

integer SNOOZE_BEFORE_CONFIG_RESET = 14;
    // number of seconds to wait before trying to read notecard again.

// requires noteworthy library v10.2 or better.
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
string NOTECARD_READ_CONTINUATION = "continue!";
    // returned as first parameter if there is still more data to handle.
string BAD_NOTECARD_INDICATOR = "bad_notecard";
    // indicates that the notecard reading process has failed to find an appropriate one.
string BUSY_READING_INDICATOR = "busy_already";
    // this return value indicates that the script is already in use by some other script.
    // the calling script should try again later.
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

// global variables that track notecard reading and any configuration found.
string global_notecard_name;  // name of our notecard in the object's inventory.
integer response_code;  // set to uniquely identify the notecard read in progress.
list global_config_list;  // a collection of configuration parameters from our notecard.
integer global_config_index;  // allows wrap-around feature, which we don't use here.

integer startup_parm = 0;  // the startup parameter for the script.
integer retries_allowed = 1;  // number of attempts to read notecards.

// sets the object up.
initialize_wylie()
{
    // listen for commands from the appropriate person(s) on the right channel.
    key id = NULL_KEY;
    if (ONLY_LISTEN_TO_OWNER) id = llGetOwner();
    llListen(LISTENING_CHANNEL, "", id, "");
    // display a text title if we were configured to.
    float title_alpha = 1.0;
    if (!SHOW_CHANNEL || !LISTENING_CHANNEL) title_alpha = 0.0;
    llSetText("/" + (string)LISTENING_CHANNEL, TITLE_COLOR, title_alpha);
    if (LISTENING_CHANNEL) {
        // if we're not on open chat listen, then we still peek out on the
        // open chat channel for certain instructions.
        llListen(0, "", id, "");
    }
    llSetTimerEvent(SNOOZE_BEFORE_CONFIG_RESET);  // snooze a bit, and try reading again.

    if (startup_parm == STARTUP_CODE_NOTECARD_RETRIES) {
///hmmm: parameterize
        retries_allowed = 11;
        startup_parm = -3;  // no longer allowed to hit unless we get restarted.
    }
    retries_allowed--;
}

// tells all the character blocks to go back to their default texture.
reset_textures()
{
    set_texture(LOWER_VOWEL_CHANNEL, RESET_TEXTURE_WORD);
    set_texture(CONSONTANT_CHANNEL, RESET_TEXTURE_WORD);
    set_texture(UPPER_VOWEL_CHANNEL, RESET_TEXTURE_WORD);
}

// makes one of the blocks show the tibetan character, if it knows it.
set_texture(integer block_number, string tib_char_wylie)
{
    llMessageLinked(LINK_SET, block_number, tib_char_wylie, NULL_KEY);
}

// find the vowel in the string provided and separate the word into its
// components for each of the blocks (in the order: upper, middle, lower).
list consume_wylie(string maybe_wylie_in)
{
    string maybe_wylie = llToLower(maybe_wylie_in);  // drop to lower case.
    // if it has a space in it, then it's not for us.
    integer posn = llSubStringIndex(maybe_wylie, " ");
    if (posn >= 0) return [];  // wrong answer.
    list vowels = [ "a", "e", "i", "o", "u" ];
    integer vow_indy;
    for (vow_indy = 0; vow_indy < llGetListLength(vowels); vow_indy++) {
        string curr_vow = llList2String(vowels, vow_indy);
        posn = find_substring(maybe_wylie, curr_vow);
        if (posn >= 0) {
            // found one!
//            log_it("found the vowel " + curr_vow);

//hmmm: here is where better logic would be needed to support multi-vowel words.

//hmmm: here is also where better logic needed for processing words, with multiple chars.
//      that would just require splitting the remainder off instead of having the rule about
//      vowel must be at end.

            // compute the consonant portion.
            string consonant = llGetSubString(maybe_wylie, 0, posn - 1) + "a";
                // we guess that the consonant is about right already.
            // dratted special cases for specific characters.
            if (maybe_wylie == curr_vow + "h") consonant = "ah";
            else if (maybe_wylie == curr_vow) consonant = "a";
//            log_it("consonant now is " + consonant);

            // now calculate the right vowel(s) and where they live.
            string lower_vowel;
            string upper_vowel;
            if (curr_vow == "a") {
                // no extra vowel markers, since there's an 'a'.
                lower_vowel = RESET_TEXTURE_WORD;
                upper_vowel = RESET_TEXTURE_WORD;
            } else if (curr_vow == "u") {
                // our one vowel that goes below.
                lower_vowel = curr_vow;
                upper_vowel = RESET_TEXTURE_WORD;
            } else {
                // mainstream vowels fly high.
                lower_vowel = RESET_TEXTURE_WORD;
                upper_vowel = curr_vow;
            }
//            log_it("success, answering: " + upper_vowel + " / " + consonant + " / " + lower_vowel);
            return [ upper_vowel, consonant, lower_vowel ];
        }

    }
    // no vowel was found!?
    return [];
}

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
    global_config_list = [];  // toss previous contents.
    // try to find a notecard with our configuration.
    response_code = -1 * (integer)randomize_within_range(23, 80000, FALSE);
    string parms_sent = wrap_parameters([WYLIE_NOTEWORTHY_SIGNATURE, response_code]);
    llMessageLinked(LINK_THIS, NOTEWORTHY_HUFFWARE_ID, READ_NOTECARD_COMMAND,
         parms_sent);
}

// processes link messages received from the noteworthy library.
integer handle_link_message(integer which, integer num, string msg, key id)
{
//log_it("got linkmsg: " + (string)which + " - " + (string)num + " - " + msg);
    if (num != NOTEWORTHY_HUFFWARE_ID + REPLY_DISTANCE) return FALSE;  // not for us.
    // process the result of reading the notecard.
    list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
    string notecard_name = llList2String(parms, 0);
    integer response_for = llList2Integer(parms, 1);
    if (response_for != response_code) return FALSE;  // oops, this isn't for us.
    if (notecard_name == BAD_NOTECARD_INDICATOR) {
// should we try again?
        return FALSE;  // drop that one.
    }
    
    // a valid notecard has been found.
//log_it("notecard name: " + notecard_name);
    // snag all but the first two elements for our config now.
    global_config_list += llList2List(parms, 2, -1);
    if (notecard_name == BUSY_READING_INDICATOR) {
        llSetTimerEvent(SNOOZE_BEFORE_CONFIG_RESET);
        return FALSE;
    } else if (notecard_name != NOTECARD_READ_CONTINUATION) {
        global_notecard_name = notecard_name;  // record its name for later use.
        // now we're done reading, process the file as a set of definitions.
        global_config_index = 0;  // we are starting over in the config list.
        process_ini_config();
        return TRUE;
    } else {
        // we're going to keep reading; notecard has more.
        llSetTimerEvent(SNOOZE_BEFORE_CONFIG_RESET);
        return FALSE;
    }
}

///////////////

// consumes the notecard in a very application specific way to retrieve our configuration items.
// the example script only looks for two variables: name and description.  if those are found in
// the sample card, then they are proudly shown.
parse_variable_definition(string to_parse)
{
//log_it("parse_var: " + to_parse);
    string content;  // filled after finding a variable name.
    if ( (content = get_variable_value(to_parse, "channel")) != "") {
        LISTENING_CHANNEL = (integer)content;
    } else if ( (content = get_variable_value(to_parse, "owner_only")) != "") {
        ONLY_LISTEN_TO_OWNER = (integer)content;
    } else if ( (content = get_variable_value(to_parse, "show_channel")) != "") {
        SHOW_CHANNEL = (integer)content;
    } else if ( (content = get_variable_value(to_parse, "title_color")) != "") {
        TITLE_COLOR = (vector)content;
    }
}

initialize_notecard_reader()
{
    // reset our relevant variables.
    global_notecard_name = "";
    global_config_list = [];
    global_config_index = 0;
    // reset our configurable constants to the defaults.
    LISTENING_CHANNEL = 0;
    ONLY_LISTEN_TO_OWNER = FALSE;
    SHOW_CHANNEL = TRUE;
    TITLE_COLOR = <1.0, 1.0, 1.0>;

    // request that the noteworthy library start looking for our notecard.
    request_configuration();
    
    llSetTimerEvent(SNOOZE_BEFORE_CONFIG_RESET);  // snooze a bit, and try reading again.
}

hear_the_voices(integer channel, string name, key id, string message)
{
    if (ONLY_LISTEN_TO_OWNER && (id != llGetOwner())) return;  // not valid from this guy.
    if (!channel && (id == llGetOwner())) {
///&& LISTENING_CHANNEL && 
        // this is general chat, which is not our listening channel.  we only
        // allow special owner commands here.
        if (is_prefix(message, BROADCAST_WORD)) {
            // whoa, they know the secret broadcasting code.
            channel = LISTENING_CHANNEL;
            // chop the bcast word, plus a space, from the command.
            message = llDeleteSubString(message, 0, llStringLength(BROADCAST_WORD));
        }
    }
    if (channel != LISTENING_CHANNEL) return;  // this is not for us.
    if (message == "reset") {
        reset_textures();
        return;
    } else if ( (message == "self-destruct") && (id == llGetOwner())) {
        // this is going to send us to object heaven, we hope.
        llDie();
    }
    // wasn't a command to reset, so let's see if we know it.
    list chopped = consume_wylie(message);
    if (llGetListLength(chopped) != 3) return;  // not understood as even close to wylie.
    set_texture(UPPER_VOWEL_CHANNEL, llList2String(chopped, 0));
    set_texture(CONSONTANT_CHANNEL, llList2String(chopped, 1));
    set_texture(LOWER_VOWEL_CHANNEL, llList2String(chopped, 2));
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

// the string processing methods are not case sensitive.
  
// returns TRUE if the "pattern" is found in the "full_string".
integer matches_substring(string full_string, string pattern)
{ return (find_substring(full_string, pattern) >= 0); }

// returns the index of the first occurrence of "pattern" inside
// the "full_string".  if it is not found, then a negative number is returned.
integer find_substring(string full_string, string pattern)
{ return llSubStringIndex(llToLower(full_string), llToLower(pattern)); }

// returns TRUE if the "prefix" string is the first part of "compare_with".
integer is_prefix(string compare_with, string prefix)
{ return find_substring(compare_with, prefix) == 0; }

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
    integer indy;
    integer count = llGetListLength(global_config_list);

    // iterate across the items in our configuration to look for ones that are not done yet.            
    for (indy = global_config_index; indy < count; indy++) {
        string line = llList2String(global_config_list, indy);
        // search for a section beginning.
        if (llGetSubString(line, 0, 0) == "[") {
            // we found the start of a section name.  now read the contents.
            indy++;  // skip section line.
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
                indy = sec_indy;  // track that we've passed this line.
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

//defect in default state:
//  if no response from noteworthy library, infinite hang.

default {
    state_entry() { if (llSubStringIndex(llGetObjectName(), "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { startup_parm = parm; state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default
{
    state_entry()
    {
        if (DEBUGGING) log_it("started, free mem=" + (string)llGetFreeMemory());
        auto_retire();  // make sure newest addition is only version of script.
        if (DEBUGGING) log_it("entered default state.");
        initialize_wylie();
        initialize_notecard_reader();
    }

    state_exit() { llSetTimerEvent(0); }

    // process the response from the noteworthy library.
    link_message(integer which, integer num, string msg, key id)
    {
        if (handle_link_message(which, num, msg, id))
            state ready_for_instructions;
    }
    
    // restart when we see changes to our notecard configuration.
    changed(integer change) {
        if (change & CHANGED_INVENTORY) {
            llSleep(4);  // delay to avoid rez-time premature startup.
            if (DEBUGGING) log_it("restarting due to inv change.");
            state rerun;
        }
    }

    // invoked when the notecard reading has failed.
    timer() {
        if (global_notecard_name == "") {
            if (retries_allowed < 0) {
                if (DEBUGGING) log_it("ran out of retries, skipping reset.");
                state ready_for_instructions;
            }
            if (DEBUGGING) log_it("timed out, going back to default.");
            state default;  // start over again.
        }
    }

    listen(integer channel, string name, key id, string message) {
        hear_the_voices(channel, name, id, message);
    }

    on_rez(integer parm) { startup_parm = parm; state rerun; }
}

state ready_for_instructions
{
    state_entry() {
        if (DEBUGGING) log_it("entered ready_for_instructions state.");
        initialize_wylie(); 
    }
    
    state_exit() { llSetTimerEvent(0); }

    listen(integer channel, string name, key id, string message) {
        hear_the_voices(channel, name, id, message);
    }

    // reset when we see changes to our notecard configuration.
    changed(integer change) {
        if (change & CHANGED_INVENTORY) {
            if (DEBUGGING) log_it("inventory change seen; restarting");
            state rerun;
        }
    }

    on_rez(integer parm) { startup_parm = parm; state default; }
}
