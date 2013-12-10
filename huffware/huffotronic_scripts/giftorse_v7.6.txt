
// huffware script: giftorse, by fred huffhines
//
// this script advertises a product in a store.
// how to configure this script: the object name is known automatically, but the seller
// and store names are not.  also the cost is not currently automatic, since there seems
// to be no way to figure it out.  all of the variable data can be stored in a notecard
// in the following format:
//
//   #giftorse
//   cost = N
//   creator = X
//   shop = S
//   label = L
//
// where N is a numerical cost for the item in linden dollars, X is the name of the creator
// of the object, S is the name of the shop where the object is being sold, and L is a label
// to show above the object.
//
// this script supports a sales paradigm, where normally it just shows the
// appropriate title, but once the customer has bought it, it will permanently set
// it's last title and exit, but not before telling the user how to get the contents.
// this script depends on the "non-script giver" script also being in the same object.
// that's what let's it promise that the stuff will be given out on touch.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//


// every product has a different cost.
string DEFAULT_COST = "**UNSET**";
string OBJ_COST;

// these are hard-coded for my own shop, since it's more convenient for me.
// remember to change these to your name and store name.
string SCRIPT_CREATOR = "Fred Huffhines";  // uber-default, do not change.
string CREATOR = "Fred Huffhines";  // change this if you are not me.
string SHOP_NAME = "eepaw shop (Eclectic Electric Patterns and Widgets)";

// if this is anything other than "default", then the text is used as the label.
// if left as "default", then the object's own name is used as the label.
string OBJECT_LABEL = "default";

// the color of the text above the object.
vector TEXT_COLOR = <0.6, 0.7, 0.8>;

// how long should the text stay on the object after a potential customer clicks it?
float DELAY_BEFORE_CLEARING_TEXT = 10.0;
    // a fairly liberal allowance for how long it might take to read the object.
    // we are trying to balance information flow with how obnoxious people feel text labels are.

//////////////

// constants that should really stay constant, like they are now...

string GIFTORSE_SIGNATURE = "#giftorse";  // the expected first line of our notecards.

//////////////

// global variables...

string global_notecard_name;  // name of our notecard in the object's inventory.

integer response_code;  // set to uniquely identify the notecard read in progress.

// programmer friendly variables...

integer DEBUGGING = FALSE;
    // if true, then the code will be noisy during its processing.

// requires noteworthy library v3.3 or higher.
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
string BAD_NOTECARD_INDICATOR = "bad_notecard";
    // indicates that the notecard reading process has failed to find an appropriate one.
string NOTECARD_READ_CONTINUATION = "continue!";
    // returned as first parameter if there is still more data to handle.
//////////////
// commands available via the noteworthy library:
string READ_NOTECARD_COMMAND = "#read_note#";
    // command used to tell the script to read notecards.  needs a signature to find
    // in the card as the only parameter.  the signature can be empty or missing.
    // the results will be fired back as the string value returned, which will have
    // an embedded list that was read from the notecard.  this necessarily limits the
    // size of the notecards that we can read and return.
//
// joins a list of parameters using the parameter sentinel for the library.
string wrap_parameters(list to_flatten)
{ return llDumpList2String(to_flatten, HUFFWARE_PARM_SEPARATOR); }
//////////////

//////////////

// from hufflets...

integer debug_num = 0;

// a debugging output method.  can be disabled entirely in one place.
log_it(string to_say)
{
    debug_num++;
    // tell this to the owner.    
    llOwnerSay(llGetScriptName() + "[" + (string)debug_num + "] " + to_say);
    // say this on open chat, but use an unusual channel.
//    llSay(108, (string)debug_num + "- " + to_say);
}

/////////////

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

// returns a non-empty string if "to_check" defines a value for "variable_name".
// this must be in the form "X=Y", where X is the variable_name and Y is the value.
string get_variable_value(string to_check, string variable_name)
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
    if (DEBUGGING)
        log_it("set " + variable_name + " = " + to_check);
    string chewed_content = to_check;
    integer indy;
    for (indy = 0; indy < llStringLength(chewed_content); indy++) {
        if (llGetSubString(chewed_content, indy, indy) == "\\") {
            if (llGetSubString(chewed_content, indy+1, indy+1) == "n") {
                chewed_content = llGetSubString(chewed_content, 0, indy - 1)
                    + "\n"
                    + llGetSubString(chewed_content, indy + 2,
                        llStringLength(chewed_content) - 1);
            }
        }
    }
    // return what's left of the string.
    return chewed_content;
}

/////////////

// is the merchant/shop guy still the owner?
integer merchant_owns()
{
    string owner_now = llToLower(llKey2Name(llGetOwner()));
    if ( (owner_now != "") && (owner_now != llToLower(CREATOR))
            && (owner_now != llToLower(SCRIPT_CREATOR) ) ) {
        // we're not able to compare the owner to what we expect.
        return FALSE;
    }
    return TRUE;
}

// returns the index of the first occurrence of "pattern" inside
// the "full_string".  if it is not found, then a negative number is returned.
integer find_substring(string full_string, string pattern)
{ return llSubStringIndex(llToLower(full_string), llToLower(pattern)); }

// applies a text label above the object.
set_text(integer enabled)
{
    if (enabled) {
        string object_label = OBJECT_LABEL;
        // reset the label to a decorated version of object name if it was default.
        if (object_label == "default") {
            list name_bits = compute_basename_and_version(llGetObjectName());
            string name = llList2String(name_bits, 0);
            if (name == "") name = llGetObjectName();  // didn't split, use the full name.
            object_label = "[ " + name + " ]";
        }
//hmmm: retired, since it's an untruth to people nearby who don't own it.
//        if (!merchant_owns()) object_label += "\n-\nClick on this object to get its contents.";
        llSetText(object_label, TEXT_COLOR, 1.0);
    } else {
        // no label, no how.
        llSetText("", TEXT_COLOR, 0.0);
    }
}

// returns TRUE if the "prefix" string is the first part of "compare_with".
integer is_prefix(string compare_with, string prefix)
{ return (llSubStringIndex(compare_with, prefix) == 0); }

parse_variable_definition(string to_parse)
{
    string content;  // filled after finding a variable name.
    if ( (content = get_variable_value(to_parse, "cost")) != "")
        OBJ_COST = content;
    else if ( (content = get_variable_value(to_parse, "creator")) != "")
        CREATOR = content;
    else if ( (content = get_variable_value(to_parse, "shop")) != "")
        SHOP_NAME = content;
    else if ( (content = get_variable_value(to_parse, "label")) != "")
        OBJECT_LABEL = content;
    else if ( (content = get_variable_value(to_parse, "text_color")) != "")
        TEXT_COLOR = (vector)content;
}

// handles a set of information from the notecard about this product.
process_product_definition(list lines)
{
    integer indy;
    for (indy = 0; indy < llGetListLength(lines); indy++) {
        string line = llList2String(lines, indy);
        // try to interpret that as a variable setting.
        parse_variable_definition(line);
    }
}

// display information about the product in open chat.
show_product_info()
{
    string product_name = llGetObjectName();
    string cost_string = "L$" + OBJ_COST + ".";
    if (OBJ_COST == "0") cost_string = "free!";
    llSay(0, " is available for " + cost_string + "\n"
        + "[Created by " + CREATOR + ", Sold by " + SHOP_NAME + "]");
}

//////////////
// huffware script: auto-retire, by fred huffhines, version 2.4.
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
// example usage of the auto-retirement script:
//     default {
//         state_entry() {
//            auto_retire();  // make sure newest addition is only version of script.
//        }
//     }
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
    state_entry() {
        auto_retire();
        set_text(FALSE);  // no label to start out.
        global_notecard_name = "";
        response_code = 0;
        OBJ_COST = DEFAULT_COST;  // reset in opensim friendly way.

        // see if we can load a notecard for product info.
        response_code = -1 * (integer)randomize_within_range(23, 80000, FALSE);
        string parms_sent = wrap_parameters([GIFTORSE_SIGNATURE, response_code]);
        llMessageLinked(LINK_THIS, NOTEWORTHY_HUFFWARE_ID, READ_NOTECARD_COMMAND,
             parms_sent);
        llSetTimerEvent(32);  // make sure that if we don't get a config, we try again.
    }
    
    // processes link messages received from support libraries.
    link_message(integer which, integer num, string msg, key id) {
        if (num != NOTEWORTHY_HUFFWARE_ID + REPLY_DISTANCE) return;  // not for us.
        if (msg == READ_NOTECARD_COMMAND) {
            // process the result of reading the notecard.
            list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
            string notecard_name = llList2String(parms, 0);
            integer response_for = llList2Integer(parms, 1);
            if (response_for != response_code) {
                // oops, this isn't for us.  stop looking at it.
                return;
            }

            integer done_reading = TRUE;
            if (notecard_name == NOTECARD_READ_CONTINUATION) done_reading = FALSE;
            if (notecard_name != BAD_NOTECARD_INDICATOR) {
                global_notecard_name = notecard_name;
                list lines = llDeleteSubList(parms, 0, 1);
                process_product_definition(lines);
            } else {
                // we hated the notecards we found, or there were none.
//                log_it("There is no product definition found.  We will proceed with defaults.");
//                state describe_product_and_neigh;
            }
            if (done_reading) {
                // since we got our final response for config, stop the timer.
                llSetTimerEvent(0);
                state describe_product_and_neigh;
            }
        }
    }

    on_rez(integer parm) { llResetScript(); }
    
    timer() {
        log_it("failed to read configuration; retrying.");
        llResetScript();
    }
    
}

// this is the active state, once we have read our product configuration from
// a notecard.
state describe_product_and_neigh
{
    state_entry() {
        set_text(FALSE);  // clear our text label just in case.
        if (OBJ_COST == DEFAULT_COST) {
            log_it("resetting due to missing configuration.");
            llResetScript();
        }
        show_product_info();
        if (!merchant_owns()) {
//hmmm: potentially might want to allow for multiple merchants so your partners don't go nuts hearing this.
            // rip this script out, since it's not intended for other folks.
            llSay(0, "\n\nThanks for your purchase from " + SHOP_NAME
                + "!\nTo get your product's contents, just click this package.");
        }
    }
    
    on_rez(integer parm) { llResetScript(); }

    touch_start(integer total_number) {
        set_text(TRUE);  // let them see our name.
        show_product_info();
        llSetTimerEvent(DELAY_BEFORE_CLEARING_TEXT);
    }

    timer() {
        // the only reason we should be here is if we are trying to clear up our text.
        set_text(FALSE);
        llSetTimerEvent(0);
    }
            
    changed(integer change) {
        if (change & CHANGED_INVENTORY) {
            llSleep(3.14159265358);
                // snooze to ensure that we don't reset before a new version gets a
                // chance to remove this.
            llResetScript();  // start over.
        }
    }
}

//eternal questions...

//hmmm: why can't we automatically get the object's sale price?
