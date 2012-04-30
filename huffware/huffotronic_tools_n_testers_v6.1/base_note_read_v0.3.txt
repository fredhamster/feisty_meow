
// add-in...  huffware script: notecard library, by fred huffhines
//
//   this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
//   do not use it in objects without fully realizing you are implicitly accepting that license.
//

// items controlled by the notecard...

string NOTECARD_SIGNATURE = "#hoopy";  // the first line of the notecard must be this.

string current_notecard_name = "";  // the name of the card we're reading now.
key current_query_id = NULL_KEY;  // the query ID for the current notecard.
list query_contents;  // the lines we have read from the notecard.
integer line_number;  // which line are we at in notecard?
integer debug = FALSE;

string HUFFWARE_PARM_SEPARATOR = "{~~~}";
    // this pattern is an uncommon thing to see in text, so we use it to separate
    // our commands in link messages.
    
initialize()
{
    // we keep the same notecard name, in case it's still good.  we want to
    // avoid re-reading the notecard unless we see an inventory change.
    current_query_id = NULL_KEY;
    query_contents = [];
    line_number = 0;
}

// returns a non-empty string if "to_check" defines contents for "variable_name".
string defines_variable(string to_check, string variable_name)
{
    // clean initial spaces.
    while (llGetSubString(to_check, 0, 0) == " ")
        to_check = llDeleteSubString(to_check, 0, 0);
    if (!is_prefix(to_check, variable_name)) return "";
    to_check = llDeleteSubString(to_check, 0, llStringLength(variable_name) - 1);
    // clean any spaces or valid assignment characters.
    while ( (llGetSubString(to_check, 0, 0) == " ")
            || (llGetSubString(to_check, 0, 0) == "=")
            || (llGetSubString(to_check, 0, 0) == ",") )
        to_check = llDeleteSubString(to_check, 0, 0);
    if (debug)
        log_it("set " + variable_name + " = " + to_check);    
    // return what's left of the string.
    return to_check;
}

parse_variable_definition(string to_parse)
{
    string content;  // filled after finding a variable name.
    string texture_name;  // temporary used in reading texture name.

//etc.    
//    if ( (content = defines_variable(to_parse, "debug")) != "")
//        debug = (integer)content;

}

process_particle_settings(list particle_definitions)
{
    integer current_item = 0;
    integer max_items = llGetListLength(particle_definitions);
    while (current_item < max_items) {
        string curr_line = llList2String(particle_definitions, current_item);
        parse_variable_definition(curr_line);
        current_item++;
    }
}

check_for_notecard()
{
    if (current_notecard_name != "") return;
    current_notecard_name = llGetInventoryName(INVENTORY_NOTECARD, 0);
    // if the notecard is real, then we will start reading it.
    if (current_notecard_name != "") {
        line_number = 0;
        query_contents = [];
        current_query_id = llGetNotecardLine(current_notecard_name, 0);
    }
}

//////////////
// from hufflets...

//////////////

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

// handles when blank strings need to come through the pipe.
string wrap_blank_string(string to_wrap)
{
    if (llStringLength(to_wrap)) return to_wrap;  // that one is okay.
    return "\"\"";  // return a quoted nothing as a signal for a blank.
}

// undoes a previously wrapped blank string.
string interpret_blank_string(string to_unwrap)
{
    if (to_unwrap == "\"\"") return "";  // that was an encoded blank.
    return to_unwrap;  // no encoding.
}

//////////////

// returns a number at most maximum and at least minimum.
// if "allow_negative" is TRUE, then the return may be positive or negative.
float randomize_within_range(float minimum, float maximum, integer allow_negative)
{
    float to_return = minimum + llFrand(maximum - minimum);
    if (allow_negative) {
        if (llFrand(1.0) < 0.5) to_return *= -1.0;
    }
    return to_return;
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
//        auto_retire();
        initialize();
        check_for_notecard();
    }

    changed(integer change_type) {
        if (change_type != CHANGED_INVENTORY) {
            // we only care about inventory changes here.
            return;
        }
        if (current_query_id != NULL_KEY) {
            // we're already reading a card right now.
            return;
        }
        // make sure we reset the old name.
        current_notecard_name = "";
        check_for_notecard();
    }
    
    dataserver(key query_id, string data) {
        if (query_id != current_query_id) {
log_it("not our query id somehow?");
//log_it("weird query had: " + (string)data);
            return;
        }
        // if we're not at the end of the notecard we're reading...
        if (data != EOF) {
            if (!line_number) {
                if (data != NOTECARD_SIGNATURE) {
                    // this card has the wrong signature at the top.  quit bothering
                    // with it now.
                    return;
                }
                log_it("starting to read notecard " + current_notecard_name + "...");    
            }
            if (data != "") {
                 // add the non-blank line to our destination list.
                query_contents += data;
//log_it("line " + (string)line_number + ": data=" + data);
            }
            line_number++;  // increase the line count.
            // request the next line from the notecard.
            current_query_id = llGetNotecardLine(current_notecard_name, line_number);
        } else {
            // no more data, so we're done with this card.
            current_query_id = NULL_KEY;
            if (!llGetListLength(query_contents)) {
                // nothing was read?  the heck with this card.
                current_notecard_name = "";  // toss bad card.
                return;
            }
log_it("notecard said:\n" + (string)(query_contents));
            log_it("done reading notecard " + current_notecard_name + ".");
        }
    }
}

