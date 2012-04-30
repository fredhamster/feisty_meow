
// huffware script: huff-update client, by fred huffhines.
//
// this script is the client side of the update process.  it should reside in an object that
// has scripts which should be automatically updated.  it will listen for announcements by
// an update server and communicate with the server to ensure that all of its scripts are
// the most up to date available with the server.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

// global constants...

integer DEBUGGING = FALSE;  // if TRUE, the script will output status information.

integer SERVER_IGNORE_TIME = 1200;  // number of seconds between performing an upgrade with the same server.

integer MAXIMUM_UPDATE_TIME_ALLOWED = 140;  // we allow one upgrade process to take this long overall.

integer UPDATE_ANNOUNCEMENT_CHANNEL   = -420108;  // used by server to brag about itself.
integer OLD_REQUEST_INVENTORY_CHANNEL = -421008;  // used by clients to request an update list.

string UPDATE_ANNOUNCEMENT_PREFIX   = "#huff-update#";  // first part of any announcement.
string REQUEST_INVENTORY_PREFIX     = "#huff-reqinv#";  // first part of request for inventory list.
string REPORT_AVAILABLE_SCRIPTS     = "#scripts#";  // server's keyword to let client know script inventory.
string REQUEST_SCRIPT_UPDATE        = "#updatego#";  // keyword used by client to request some updates.
string SHUT_THEM_DOWN               = "#huffdown#";  // server tells client to stop any non-updater scripts.
string READY_TO_UPDATE              = "#listoneeds#";  // the client tells the server the scripts it wants.
string SCRIPTS_ARE_CURRENT          = "#gottemthx#";  // client says this when all new scripts are in place.
string START_THEM_UP                = "#huffup#";  // server tells client to start up other scripts again.
string DONE_UPDATING                = "#finito#";  // the client is done updating.
string BUSY_BUSY                    = "#busymuch#";  // a signal that the server is too busy to update us.

float UPDATE_TIMER_INTERVAL = 2.0;  // interval between checks on our update status.

integer UPDATER_SCRIPT_PIN = -1231008;  // the hook for our scripts to be modified.

///float BUSY_SERVER_PAUSE_TIME = 38.0;  // num seconds to delay when server says it's too busy.

string UPDATER_PARM_SEPARATOR = "~~~";
    // three tildes is an uncommon thing to have otherwise, so we use it to separate
    // our commands in linked messages.

string SCRIPT_DEPENDENCY_MARK = "DEP";  // signals that a dependency is coming.
string ITEM_LIST_SEPARATOR = "``";  // separates dependencies.

integer MAXIMUM_SERVERS_TRACKED = 32;
    // we will listen to this many servers before we decide to remove one.

string CONTINUANCE_MARKER = "...";
    // a string sent when the update list is too long and needs to be continued in another chat.

string SERVER_SCRIPT = "a huffotronic update server";
    // the prefix of our server script that hands out updates.

// global variables...

integer inventory_request_channel;  // used for newer version servers to cut down cross chatter.
list updaters_heard;  // the update servers we've heard from recently.
list last_interactions;  // times of the last update process engaged with the updater.
integer update_channel;  // current channel for interaction with specific server.
key current_server;  // the updater that is active right now, if any.
integer update_start_time;  // when the last update process began.
list updates_needed;  // stores the set of scripts that are in need of an update.
list known_script_dependencies;  // stores the list of dependency info.

careful_crankup()
{
    knock_around_other_scripts(TRUE);
    // clean out the older items and scripts.  we do this after getting everyone running
    // since we might be whacking ourselves.
    destroy_older_versions();
}

// reset our variables.
initialize()
{
    updaters_heard = [];
    last_interactions = [];
    inventory_request_channel = 0;
    update_channel = 0;
    current_server = NULL_KEY;
    llSetTimerEvent(0.0);
    llSetRemoteScriptAccessPin(UPDATER_SCRIPT_PIN);
    // a new enhancements; tells the server that this guy has finished an update cycle.  this
    // only comes into play when the updater script itself has just been updated, but it's
    // nice for the server to avoid claiming erroneous timeouts occurred.
    llSay(OLD_REQUEST_INVENTORY_CHANNEL, DONE_UPDATING);
    llSleep(0.4);  // snooze and repeat to overcome occasionally lossy chats.
    llSay(OLD_REQUEST_INVENTORY_CHANNEL, DONE_UPDATING);
}

whack_updater_record(key id)
{
    integer prev_indy = find_in_list(updaters_heard, id);
    if (prev_indy < 0) return;  // not there.
    updaters_heard = chop_list(updaters_heard, 0, prev_indy - 1)
            + chop_list(updaters_heard, prev_indy + 1, llGetListLength(updaters_heard) - 1);
    last_interactions = chop_list(last_interactions, 0, prev_indy - 1)
            + chop_list(last_interactions, prev_indy + 1, llGetListLength(last_interactions) - 1);
}

// note that this new, lower memory version, depends on the inventory functions returning
// items in alphabetical order.
scrub_items_by_type(string this_guy, integer inventory_type)
{
    list removal_list;
    integer outer;
    for (outer = 0; outer < llGetInventoryNumber(inventory_type); outer++) {
        string curr = llGetInventoryName(inventory_type, outer);
        list split = compute_basename_and_version(curr);
        // make sure there was a comparable version number in this name.
        if ( (curr != this_guy) && llGetListLength(split)) {
            string curr_base = llList2String(split, 0);
            float curr_ver = (float)llList2String(split, 1);
//log_it("outer: " + curr_base + " / " + (string)curr_ver);
            integer inner;
            for (inner = outer + 1; inner < llGetInventoryNumber(inventory_type); inner++) {
                string next_guy = llGetInventoryName(inventory_type, inner);
                list comp_split = compute_basename_and_version(next_guy);
                if (llGetListLength(comp_split)) {
                    string comp_base = llList2String(comp_split, 0);
                    float comp_ver = (float)llList2String(comp_split, 1);
                    // okay, now we can actually compare.
                    if (curr_base != comp_base) {
                        // break out of inner loop.  we are past where the names can matter.
                        inner = 2 * llGetInventoryNumber(inventory_type);
                    } else {
//log_it("inner: " + comp_base + " / " + (string)comp_ver);
                        if (curr_ver <= comp_ver) {
                            // the script at inner index is comparable or better than
                            // the script at the outer index.
                            removal_list += curr;
                        } else {
                            // this inner script must be inferior to the outer one,
                            // somehow, which defies our expectation of alphabetical ordering.
                            removal_list += next_guy;
                        }
                    }
                }
            }
        }
    }

    // now actually do the deletions.
    for (outer = 0; outer < llGetListLength(removal_list); outer++) {
        string to_whack = llList2String(removal_list, outer);
        if (DEBUGGING)
            log_it("removing older asset: " + to_whack);
        llRemoveInventory(to_whack);
    }
}

// ensures that only the latest version of any script or object is kept in our inventory.
destroy_older_versions()
{
    // firstly, iterate across scripts to clean out older versions.
    scrub_items_by_type(llGetScriptName(), INVENTORY_SCRIPT);
    // secondly, try to clean out the objects.
    scrub_items_by_type(llGetScriptName(), INVENTORY_OBJECT);
    // thirdly, try to clean out the notecards.
    scrub_items_by_type(llGetScriptName(), INVENTORY_NOTECARD);
}

// sets the object to be listening for update info.
// if "just_owner" is true, then we will not listen on the general announcement channel.
listen_for_orders(integer just_owner)
{
    if (!just_owner) {
        // try to hear an update being announced.
        llListen(UPDATE_ANNOUNCEMENT_CHANNEL, "", NULL_KEY, "");
    }

    // super secret owner controls.
    llListen(0, "", llGetOwner(), "");
}

// returns true if this object is a huffotronic updater of some sort.
integer inside_of_updater()
{
    return find_substring(llGetObjectName(), "huffotronic") >= 0;
}

// returns true if a script is a version of our update server.
integer matches_server_script(string to_check)
{
    return is_prefix(to_check, SERVER_SCRIPT);
}

// stops all the scripts besides this one.
knock_around_other_scripts(integer running_state)
{
    integer insider = inside_of_updater();
    if (running_state == TRUE) {
        // make sure we crank up the scripts that are new first.  we want to reset them
        // as well, which we don't want to do for any existing scripts.
        integer crank_indy;
        for (crank_indy = 0; crank_indy < llGetListLength(updates_needed); crank_indy++) {
            string crankee = llList2String(updates_needed, crank_indy);
            if (find_in_inventory(crankee, INVENTORY_SCRIPT, TRUE) >= 0) {
                if (!insider || matches_server_script(crankee)) {
                    // allow it to run again.
                    llSetScriptState(crankee, TRUE);
                    // reset it, to make sure it starts at the top.
                    llResetOtherScript(crankee);
                }
            }
        }
    }

    integer indy;
    string self_script = llGetScriptName();
    // we set all other scripts to the running state requested.
    for (indy = 0; indy < llGetInventoryNumber(INVENTORY_SCRIPT); indy++) {
        string curr_script = llGetInventoryName(INVENTORY_SCRIPT, indy);
        if ( (curr_script != self_script)
            && (!insider || matches_server_script(curr_script)) ) {
            // this one seems ripe for being set to the state requested.
            llSetScriptState(curr_script, running_state);
        }
    }
}

// a random channel for the interaction with the server.
integer random_channel() { return -(integer)(llFrand(800000) + 20000); }

// make sure that any dependencies for the script with "basename" are added to the list
// of requests we make during an update.
list add_dependencies(string basename)
{
    list to_return;
    integer indy;
    for (indy = 0; indy < llGetListLength(known_script_dependencies); indy++) {
        list deps = llParseString2List(llList2String(known_script_dependencies, indy),
            [ITEM_LIST_SEPARATOR], []);
//log_it("base=" + llList2String(dep, 0) + " lastver=" + llList2String(dep, 1) + " newdep=" + llList2String(dep, 2));
        if (basename == llList2String(deps, 0)) {
            // first off, is this item with new dependencies actually present?
            integer where = find_in_inventory(basename, INVENTORY_SCRIPT, FALSE);
            if (where >= 0) {
                // we do use the script with deps, but is the dependent item really missing?
                where = find_in_inventory(llList2String(deps, 1), INVENTORY_SCRIPT, FALSE);
                if (where < 0) {
                    // we found a dependency match for this script, so we'll ask for the missing item.
                    if (DEBUGGING)
                        log_it("missing dep: " + llList2String(deps, 1));
                    to_return += [ llList2String(deps, 1) ];
                }
            }
        }
    }
    return to_return;
}

// complains if memory seems to be getting tight.
test_memory()
{
    if (llGetFreeMemory() < 4096)
        log_it("mem_free = " + (string)llGetFreeMemory());
}

// starts an update given a list of scripts that the server has available, encoded as
// a string in the "encoded_list".
integer initiate_update(string encoded_list)
{
    list scripts_avail = llParseString2List(encoded_list, [UPDATER_PARM_SEPARATOR], []);
    integer continue_listening_for_scripts = FALSE;
      // if true, we aren't done hearing about available scripts yet.
    encoded_list = "";
    // figure out which scripts we need by comparing the list available from the server
    // against our current inventory.  we only want scripts with newer version numbers.
    integer sindy;
    for (sindy = 0; sindy < llGetListLength(scripts_avail); sindy++) {
        string curr = llList2String(scripts_avail, sindy);
        if (curr == CONTINUANCE_MARKER) {
            // this is a special continuation signal.  we need to hear the rest of the list.
            continue_listening_for_scripts = TRUE;
        } else if (is_prefix(curr, SCRIPT_DEPENDENCY_MARK)) {
            // we've found a dependency item.
            known_script_dependencies += [ llGetSubString(curr, llStringLength(SCRIPT_DEPENDENCY_MARK), -1) ];
//log_it("script dep: " + llGetSubString(curr, llStringLength(SCRIPT_DEPENDENCY_MARK), -1));
        } else {
            list split = compute_basename_and_version(curr);
            if (llGetListLength(split) == 2) {
                string basename = llList2String(split, 0);
                string version = llList2String(split, 1);
                split = [];
                integer oy_indy;
//replace common code with func.
                for (oy_indy = 0; oy_indy < llGetInventoryNumber(INVENTORY_OBJECT); oy_indy++) {
                    list srv_split = compute_basename_and_version
                        (llGetInventoryName(INVENTORY_OBJECT, oy_indy));
                    if ( (llGetListLength(srv_split) == 2)
                            && (basename == llList2String(srv_split, 0))
                            && ((float)version > (float)llList2String(srv_split, 1)) ) {
//                        if (DEBUGGING) {
                            log_it("i need '" + curr + "' from server " + (string)inventory_request_channel);
//                        }
                        test_memory();
                        updates_needed += [ curr ];
                    }
                }
                for (oy_indy = 0; oy_indy < llGetInventoryNumber(INVENTORY_NOTECARD); oy_indy++) {
                    list srv_split = compute_basename_and_version
                        (llGetInventoryName(INVENTORY_NOTECARD, oy_indy));
                    if ( (llGetListLength(srv_split) == 2)
                            && (basename == llList2String(srv_split, 0))
                            && ((float)version > (float)llList2String(srv_split, 1)) ) {
                        if (DEBUGGING) {
                            log_it("i need '" + curr + "' from server " + (string)inventory_request_channel);
                        }
                        test_memory();
                        updates_needed += [ curr ];
                    }
                }
                for (oy_indy = 0; oy_indy < llGetInventoryNumber(INVENTORY_SCRIPT); oy_indy++) {
                    list srv_split = compute_basename_and_version
                        (llGetInventoryName(INVENTORY_SCRIPT, oy_indy));
                    if ( (llGetListLength(srv_split) == 2)
                            && (basename == llList2String(srv_split, 0))
                            && ((float)version > (float)llList2String(srv_split, 1)) ) {
                        if (DEBUGGING) {
                            log_it("i need '" + curr + "' from server " + (string)inventory_request_channel);
                        }
                        test_memory();
                        updates_needed += [ curr ];
                    }
                }
                updates_needed += add_dependencies(basename);
            }
        }
    }
    // we skip the next step if we're still waiting to hear about more.
    if (continue_listening_for_scripts) {
//log_it("still listening for more updates...");
        return FALSE;
    }
    if (llGetListLength(updates_needed)) {
//log_it("update chan=" + (string)update_channel);
        llSay(update_channel, REQUEST_SCRIPT_UPDATE);
        if (DEBUGGING) {
            log_it("told server " + (string)inventory_request_channel + " that i need updating.");
        }
    } else {
        if (DEBUGGING) {
            log_it("told server " + (string)inventory_request_channel + " that i am done updating.");
        }
        llSay(update_channel, DONE_UPDATING);
    }
    return TRUE;
}    

// this alerts the server to our most desired scripts.
tell_server_our_wish_list()
{
    llSay(update_channel, READY_TO_UPDATE + wrap_parameters(updates_needed));
}

// checks whether all of the updates needed are present yet.
integer check_on_update_presence()
{
    integer indy;
    for (indy = 0; indy < llGetListLength(updates_needed); indy++) {
        integer found = find_in_inventory(llList2String(updates_needed, indy), INVENTORY_ALL, TRUE);
        // any single missing guy means they aren't up to date yet.
        if (found < 0) {
            if (DEBUGGING) log_it(llList2String(updates_needed, indy) + " not seen as updated yet.");
            return FALSE;
        }
    }
    // nothing was detected as missing anymore.
    return TRUE;
}

// respond to spoken commands from the server.
integer process_update_news(integer channel, string name, key id, string message)
{
    if (!channel) {
        // this is a command.
        if (message == "ureset") {
            llResetScript();  // start over.
        }
        if (message == "ushow") {
            integer sindy;
            integer script_count = llGetInventoryNumber(INVENTORY_SCRIPT);
            list script_list = [ "scripts--" ];  // first item is just a header.
            for (sindy = 0; sindy < script_count; sindy++) {
                script_list += [ llGetInventoryName(INVENTORY_SCRIPT, sindy) ];
            }
            dump_list_to_log(script_list);
        }
        return FALSE;  // nothing to do here.
    }
    if (!update_channel && (channel == UPDATE_ANNOUNCEMENT_CHANNEL)) {
/* never seen.        if (id == llGetKey()) {
if (DEBUGGING) log_it("ignoring update from self.");            
            return FALSE;  // ack, that's our very object.
        }
*/
        if (llStringLength(message) > llStringLength(UPDATE_ANNOUNCEMENT_PREFIX)) {
            // this is a new style update message.  we can set a different request channel.
            string just_chan = llDeleteSubString(message, 0, llStringLength(UPDATE_ANNOUNCEMENT_PREFIX) - 1);
            inventory_request_channel = (integer)just_chan;
        }
        integer prev_indy = find_in_list(updaters_heard, id);
        // find the talker in our list.
        if (prev_indy >= 0) {
            // that guy was already heard from.  check when last interacted.
            integer last_heard = llList2Integer(last_interactions, prev_indy);
            if (llAbs(llGetUnixTime() - last_heard) < SERVER_IGNORE_TIME) {
                return FALSE;  // not time to update with this guy again yet.
            }
//            if (DEBUGGING) { log_it("started listening again to server " + (string)id); }
            // make sure we think of this as a new updater now.
            whack_updater_record(id);
        }

        if (DEBUGGING) { log_it("heard server " + (string)inventory_request_channel + "'s announcement."); }
        // record our new server.
        current_server = id;
        // make a random pause so not all updaters try to crank up at same time.
        llSleep(randomize_within_range(2.8, 18.2, FALSE));

        if (llGetListLength(updaters_heard) > MAXIMUM_SERVERS_TRACKED) {
            // oops, this is not good.  we have too many servers now.
//hmmm: room for improvement here by tossing out the server that is oldest.
            updaters_heard = llDeleteSubList(updaters_heard, 0, 0);
            last_interactions = llDeleteSubList(last_interactions, 0, 0);
        }
        
        // add the talker to our list.
        updaters_heard += id;
        last_interactions += llGetUnixTime();
    
        // begin the update interaction with this guy.
        update_channel = random_channel();
        return TRUE;
    }
    if (update_channel && (channel == update_channel) ) {
        if (is_prefix(message, REPORT_AVAILABLE_SCRIPTS)) {
            // tasty, this is a list of scripts that can be had.
            message = llDeleteSubString(message, 0, llStringLength(REPORT_AVAILABLE_SCRIPTS) - 1);
            if (message == BUSY_BUSY) {
                // server has signified that it's too busy (or its owner is a moron) because it is
                // claiming it has no scripts at all.
                if (DEBUGGING) {
                    log_it("server " + (string)inventory_request_channel + " is too busy to update us now.");
                }
                // make it seem like we need to do this one again sooner than normal.
                whack_updater_record(id);
                // busy server means move no further forward.
                return FALSE;
            }
            return initiate_update(message);
        } else if (is_prefix(message, SHUT_THEM_DOWN)) {
            if (DEBUGGING) { log_it("stopping other scripts."); }
            knock_around_other_scripts(FALSE);
            // now that we know for sure the server's ready to update us,
            // we tell it what we need.
            tell_server_our_wish_list();
            return FALSE;
        } else if (is_prefix(message, START_THEM_UP)) {
            // let the server know that we've finished, for all intents and purposes.
            llSay(update_channel, DONE_UPDATING);
            // we pause a random bit first; we want to ensure we aren't swamping
            // SL with our inventory loading.
            llSleep(randomize_within_range(2.5, 8.2, FALSE));
            if (DEBUGGING) { log_it("starting other scripts."); }
            careful_crankup();
            return TRUE;  // change state now.
//        } else {
//log_it("unknown command on update channel: " + message);
        }
    }
    return FALSE;
}

//////////////
// from hufflets...

integer debug_num = 0;

// a debugging output method.  can be disabled entirely in one place.
log_it(string to_say)
{
    debug_num++;
    llWhisper(0, llGetScriptName() + " [" + (string)debug_num + "] (" + (string)llGetFreeMemory() + ") " + to_say);
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

// returns the portion of the list between start and end, but only if they are
// valid compared with the list length.  an attempt to use negative start or
// end values also returns a blank list.
list chop_list(list to_chop, integer start, integer end)
{
    integer last_len = llGetListLength(to_chop) - 1;
    if ( (start < 0) || (end < 0) || (start > last_len) || (end > last_len) ) return [];
    return llList2List(to_chop, start, end);
}

// joins a list of parameters using the parameter sentinel for the library.
string wrap_parameters(list to_flatten)
{ return llDumpList2String(to_flatten, UPDATER_PARM_SEPARATOR); }

// locates the item with "name_to_find" in the inventory items with the "type".
// a value from 0 to N-1 is returned if it's found, where N is the number of
// items in the inventory.
integer find_in_inventory(string name_to_find, integer inv_type, integer exact_match)
{
    integer num_inv = llGetInventoryNumber(inv_type);
    if (num_inv == 0) return -1;  // nothing there!
    integer inv;
    for (inv = 0; inv < num_inv; inv++) {
        if (exact_match && (llGetInventoryName(inv_type, inv) == name_to_find) )
            return inv;
        else if (!exact_match && is_prefix(llGetInventoryName(inv_type, inv), name_to_find))
            return inv;
    }
    return -2;  // failed to find it.
}

//////////////

integer MAX_CHAT_LINE = 900;
    // the most characters we'll try to say in one chat.

dump_list_to_log(list to_show)
{
    string text = dump_list(to_show);  // get some help from the other version.
    integer len = llStringLength(text);
    integer i;
    for (i = 0; i < len; i += MAX_CHAT_LINE) {
        integer last_bit = i + MAX_CHAT_LINE - 1;
        if (last_bit >= len) last_bit = len - 1;
        string next_line = llGetSubString(text, i, last_bit);
        llWhisper(0, next_line);
    }
}

// returns a printable form of the list.
string dump_list(list to_show)
{
    integer len = llGetListLength(to_show);
    integer i;
    string text;
    for (i = 0; i < len; i++) {
        string next_line = llList2String(to_show, i);
        if (find_substring(next_line, " ") >= 0) {
            // this guy has a space in it, so quote it.
            next_line = "\"" + next_line + "\"";
        }
        text = text + next_line;
        if (i < len - 1) text = text + " ";
    }
    return text;
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

// no huffotronic trap state for startup, because this script will actually
// run (and is expected) inside a huffotronic updater object.

default
{
    state_entry()
    {
        auto_retire();  // only allow the most recent revision.
        initialize();
        state awaiting_commands;
    }
}

state awaiting_commands
{
    state_entry()
    {
        if (DEBUGGING) log_it("<awaiting_commands>");
        careful_crankup();  // we always start by getting everyone running.
        current_server = NULL_KEY;  // forget previous server.
        listen_for_orders(FALSE);
        inventory_request_channel = 0;  // no inventory request channel either.
        update_channel = 0;  // no channel currently.
        updates_needed = [];  // we know of no needs right now.
        known_script_dependencies = [];  // no deps either.
    }

    state_exit() { llSetTimerEvent(0.0); }

    listen(integer channel, string name, key id, string message)
    {
        if ((id != llGetOwner()) && (llGetOwnerKey(id) != llGetOwner())) {
            return;  // must be same owner to ensure proper perms.
        }
        if (process_update_news(channel, name, id, message))
            state establish_private_channel;
    }
}

state establish_private_channel
{
    state_entry()
    {
        if (DEBUGGING) log_it("<establish_private_channel>");
        llListen(update_channel, "", current_server, "");
        listen_for_orders(TRUE);
        if (inventory_request_channel)
            llSay(inventory_request_channel, REQUEST_INVENTORY_PREFIX + (string)update_channel);
        else
            llSay(OLD_REQUEST_INVENTORY_CHANNEL, REQUEST_INVENTORY_PREFIX + (string)update_channel);
        llSetTimerEvent(MAXIMUM_UPDATE_TIME_ALLOWED);
    }

    state_exit() { llSetTimerEvent(0); }

    listen(integer channel, string name, key id, string message)
    {
        if ((id != llGetOwner()) && (llGetOwnerKey(id) != llGetOwner())) {
            return;  // must be same owner to ensure proper perms.
        }
        if (process_update_news(channel, name, id, message)) {
            // ready for a state change, but what kind?
            if (llGetListLength(updates_needed)) {
//log_it("have a list of updates now.");
                state performing_update;
            } else {
//log_it("no updates needed in list, going back");
                state awaiting_commands;
            }
        }
    }
    
    timer() {
        if (DEBUGGING) {
            log_it("timed out establishing channel with server " + (string)inventory_request_channel);
        }
        whack_updater_record(current_server);
        state awaiting_commands;
    }

    on_rez(integer parm) { state default; }
}

state performing_update
{
    state_entry()
    {
        // must re-listen after a state change.
        llListen(update_channel, "", current_server, "");
        listen_for_orders(TRUE);
        if (DEBUGGING) log_it("<performing_update>");
        llSetTimerEvent(UPDATE_TIMER_INTERVAL);
        update_start_time = llGetUnixTime();
    }

    state_exit() { llSetTimerEvent(0.0); }

    listen(integer channel, string name, key id, string message)
    {
        if ((id != llGetOwner()) && (llGetOwnerKey(id) != llGetOwner())) {
            return;  // must be same owner to ensure proper perms.
        }
        if (process_update_news(channel, name, id, message)) {
            // normal finish of update process.
            state awaiting_commands;
        }
    }

    timer() {
        if (llGetListLength(updates_needed) == 0) {
//log_it("nothing to update, leaving perform state.");
            state awaiting_commands;  // we've got nothing to do.
        } else {
            // see if all our requested scripts are there yet; if not, we're not done updating.
            integer ready = check_on_update_presence();
            if (ready) {
                if (DEBUGGING) log_it("reporting scripts are current.");
                llSay(update_channel, SCRIPTS_ARE_CURRENT);
            }
        }
        if (llAbs(update_start_time - llGetUnixTime()) >= MAXIMUM_UPDATE_TIME_ALLOWED) {
            if (DEBUGGING) { log_it("timeout during update process with server " + (string)inventory_request_channel); }
            whack_updater_record(current_server);
            state awaiting_commands;
        }
    }

    on_rez(integer parm) { state default; }
}

