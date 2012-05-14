
// huffware script: noteworthy example usage, by fred huffhines
//
// shows how to use the noteworthy script as a configuration helper.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

// global constants...

string EXAMPLE_NOTEWORTHY_SIGNATURE = "#example_noteworthy";
    // the notecard must begin with this as its first line for it to be
    // recognized as our configuration card.

// global variables...

string global_notecard_name;  // name of our notecard in the object's inventory.
integer response_code;  // set to uniquely identify the notecard read in progress.
list global_config_list;  // a collection of configuration parameters from our notecard.
integer global_config_index;  // allows wrap-around feature, which we don't use here.

// requires noteworthy library v8.3 or better.
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
//////////////
string BAD_NOTECARD_INDICATOR = "bad_notecard";
    // indicates that the notecard reading process has failed to find an appropriate one.
string BUSY_READING_INDICATOR = "busy_already";
    // this return value indicates that the script is already in use by some other script.
    // the calling script should try again later.
string NOTECARD_READ_CONTINUATION = "continue!";
    // returned as first parameter if there is still more data to handle.
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
    string parms_sent = wrap_parameters([EXAMPLE_NOTEWORTHY_SIGNATURE, response_code]);
    llMessageLinked(LINK_THIS, NOTEWORTHY_HUFFWARE_ID, READ_NOTECARD_COMMAND,
         parms_sent);
}

// processes link messages received from the noteworthy library.
handle_link_message(integer which, integer num, string msg, key id)
{
    if ( (num != NOTEWORTHY_HUFFWARE_ID + REPLY_DISTANCE)
            || (msg != READ_NOTECARD_COMMAND) )
        return;  // not for us.
    // process the result of reading the notecard.
    list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
    string notecard_name = llList2String(parms, 0);
    integer response_for = llList2Integer(parms, 1);
    if (response_for != response_code) return;  // oops, this isn't for us.
    if (notecard_name == BAD_NOTECARD_INDICATOR) {
        // we hated the notecards we found, or there were none.
        log_it("We apologize; there seem to be no notecards with a first line of '"
            + EXAMPLE_NOTEWORTHY_SIGNATURE
            + "'.  We can't read any configuration until that situation improves.");
    } else {
        // snag all but the first two elements for our config now.
        global_config_list += llList2List(parms, 2, -1);
        // make sure we shouldn't keep going.
        if (notecard_name != NOTECARD_READ_CONTINUATION) {
            // a valid notecard has been found.
            global_notecard_name = notecard_name;  // record its name for later use.
            global_config_index = 0;  // we are starting over in the config list.
            // now echo the card we received...
            log_it("read notecard \"" + global_notecard_name + "\":");
            integer lines_to_say = llGetListLength(global_config_list);
            if (lines_to_say > 8)
                lines_to_say = 8;  // limit how much text is spoken.
            integer indy;
            for (indy = 0; indy < lines_to_say; indy++)
                log_it("line #" + (string)(indy + 1) + ": " + llList2String(global_config_list, indy));
            if (lines_to_say != llGetListLength(global_config_list))
                log_it("...and so forth....");
            // and process the file as a set of definitions.
            process_ini_config();
        }
    }
}

///////////////

// consumes the notecard in a very application specific way to retrieve our configuration items.
// the example script only looks for two variables: name and description.  if those are found in
// the sample card, then they are proudly shown.
parse_variable_definition(string to_parse)
{
//    string content;  // filled after finding a variable name.
//    if ( (content = get_variable_value(to_parse, "name")) != "")
//        log_it("** got a name of '" + content + "'");
//    else if ( (content = get_variable_value(to_parse, "description")) != "")
//        log_it("** got a description of '" + content + "'");
}

initialize()
{
    // reset our relevant variables.
    global_notecard_name = "";
    global_config_list = [];
    global_config_index = 0;

    // announce that we're open for business.
    log_it("example noteworthy usage is started, free mem=" + (string)llGetFreeMemory());

    // request that the noteworthy library start looking for our notecard.
    request_configuration();
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
    log_it("scanning notecard for variable definitions...");
    integer indy;
    integer count = llGetListLength(global_config_list);
    string section_name;  // set later if we see one.

    // iterate across the items in our configuration to look for ones that are not done yet.            
    for (indy = global_config_index; indy < count; indy++) {
        string line = llList2String(global_config_list, indy);
        // search for a section beginning.
        if (llGetSubString(line, 0, 0) == "[") {
            // we found the start of a section name.  now read the contents.
            indy++;  // skip section line.
            section_name = llGetSubString(line, 1, -2);
            log_it("reading section: " + section_name);
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
        initialize();
    }
    
    on_rez(integer parm) { llResetScript(); }
    
    touch_start(integer count) {
        log_it("re-initializing script now to read card again...");
        initialize();
    }

    // reset when we see changes to our notecard configuration.
    changed(integer change) {
        if (change & CHANGED_INVENTORY) {
            llSleep(3.14159265358);  // delay to avoid interfering with upgrade.
            llResetScript(); 
        }
    }
    
    // process the response from the noteworthy library.
    link_message(integer which, integer num, string msg, key id)
    { handle_link_message(which, num, msg, id); }
}
