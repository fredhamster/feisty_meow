
// huffware script: jaunt wik rez, by fred huffhines,  released under GPL license.
//
// this is a jaunter (teleporter) script with some useful and uncommon features.
// it can work around certain types of complicated land permissions on neighboring land
// that might otherwise lead to blocked jaunts.
// the usage of "jaunt" for teleportation is inspired by the alfred bester novel, "the stars
// my destination".  if you like science fiction and have not read this book, it is highly
// recommended.
// parts of this script are based on "Teleporter Script v 3.0 by Asira Sakai", from
// the LSL wiki.
//
// use jaunt wik rez by placing a notecard in the inventory of your jaunter object.  the notecard
// should start with "#jaunt" on the first line and then should have alternating lines of
// (1) jaunt target (as a vector) and (2) readable target name, for example:
//      #jaunt
//      <23, 18, 92>
//      gracy square
//      <182, 32, 56>
//      tourmaline palace
//      <23, 18, 92>|<118, 19, 108>|<120, 33, 57>
//      unblocked pathway, hurrah
// note the last destination; the script also supports a list of intermediate destinations.
// this feature can enable one to route a teleport around blocked lands by using waypoints
// that are still accessible.  but since the current version of the script automatically
// finds routes for most destinations, this is not needed very frequently.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

//////////////

integer DEBUGGING = FALSE;
    // make true to print diagnostic logging.

// configurable options (set in notecard):

integer SHOW_TEXT = TRUE;
    // if true, then the jaunter will show a text label for the destination.
    // this can be configured from the notecard with ":show_text=0" to turn off,
    // or 1 to turn on.
integer ADD_NAME = FALSE;
    // if this is true, then the text shown will include the object's name.

string TEXT_COLOR = "<0.3, 0.9, 0.7>";
    // set float text color to a nice color.
///hmmm: doesn't seem to be in the notecard yet!

//////////////

// the jaunter button pushing API.
//////////////
integer BUTTON_PUSHER_HUFFWARE_ID = 10029;
    // a unique ID within the huffware system for this script.
//////////////
string BUTTON_PUSHED_ALERT = "#btnp";
    // this event is generated when the button is pushed.  the number parameter will be
    // the huffware id plus the reply distance.  the id parameter in the link message will
    // contain the name of the button that was 
string JAUNT_NEXT_BUTTON_NAME = "next";
string JAUNT_MENU_BUTTON_NAME = "menu";

//////////////
    
// requires the jaunt config API.
//////////////
// do not redefine these constants.
integer JAUNT_CONFIGURATION_HUFFWARE_ID = 10022;
    // the unique id within the huffware system for this script's commands.
    // it's used in llMessageLinked as the num parameter.
string HUFFWARE_PARM_SEPARATOR = "{~~~}";
    // this pattern is an uncommon thing to see in text, so we use it to separate
    // our commands in link messages.
string HUFFWARE_ITEM_SEPARATOR = "{|||}";
    // used to separate lists of items from each other when stored inside a parameter.
    // this allows lists to be passed as single string parameters if needed.
integer REPLY_DISTANCE = 100008;  // offset added to service's huffware id in reply IDs.
//////////////
// commands available from the jaunt config library:
string LOAD_ALL_CONFIGURATIONS = "#rdcfg#";
    // this starts the process of loading jaunt destinations from the notecards and
    // landmarks that we find in the object.  the parms are: private channel
    // and conveyance mode.
string JAUNT_CFG_DATA_EVENT = "#yodata#";
    // this event is generated from this script back to the caller.  when we have some
    // landmark data or configuration information, it's passed back in chunks.  the
    // deluge of pieces will continue until the JAUNT_CFG_EOF event is passed.  the
    // parameters include packed destination name and vector pairs (which form 2 elements
    // in the list) and packed config variable definitions (where config items start
    // with a colon, and the definition is only one element in the list).
string JAUNT_CFG_EOF = "#done#";
    // sent after all valid configuration items that we could find were processed.
//
//////////////

// requires jaunting library API.
//////////////
// do not redefine these constants.
integer JAUNT_HUFFWARE_ID = 10008;
    // the unique id within the huffware system for the jaunt script to
    // accept commands on.  this is used in llMessageLinked as the num parameter.
//////////////
// commands available via the jaunting library:
string FULL_STOP_COMMAND = "#fullstop#";
    // command used to bring object to a halt.
string JAUNT_LIST_COMMAND = "#jauntlist#";
    // like regular jaunt, but expects a string in jaunt notecard format with vectors.
    // the second parameter, if any, should be 1 for forwards traversal and 0 for backwards.
//
//////////////

// requires menutini API.
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

// requires data cow library API.
//////////////
// do not redefine these constants.
integer DATA_COW_HUFFWARE_ID = 10017;
    // the unique id within the huffware system for the jaunt script to
    // accept commands on.  this is used in llMessageLinked as the num parameter.
//////////////
string RESET_LIST_COMMAND = "reset_L";
    // flushes out the currently held list.  does not send a reply.
string ADD_ITEM_COMMAND = "add_I";
    // adds items to the list.  this is a list of pairs of name/value, where the name is
    // how the item will be looked up from the list, and the value is the contents to be stored.
    // this command has no reply.
string GET_COW_LENGTH = "get_Lc";
    // returns a single integer which is the length of the cow's list currently.
//string REMOVE_ITEM_COMMAND = "rm_I";
    // accepts a list of names for items.  all the mentioned ones will be removed from the list.
    // this command also has no reply.
//string GET_ITEM_COMMAND = "get_I";
    // retrieves the item's contents for a given name.  first parm is the name.  if there
    // are other parameters, then they are taken as other items to return also.
    // the return data will be pairs of <name, entry> for each of the names in the request.
    // if the data was not located, then the entry will be empty.
string TAGGED_GET_ITEM_COMMAND = "get_T";
    // like get item, except the first parameter is an identifier that the caller wants to
    // use to tag this request.  the other parameters are still taken as names.  the response
    // will contain the tag as the first item, then the <name, entry> pairs that were found.

//////////////

// requires click action API...
//////////////
integer CHANGE_CLICK_ACTION_HUFFWARE_ID = 10024;
    // a unique ID within the huffware system for this script.
// the API only supports the service number; there are no commands to pass in the "msg"
// parameter.  the id does accept a parameter, which is the type of click action to begin
// using for this prim.
//////////////

// the API for this library script to be used in other scripts.
//////////////
// do not redefine these constants.
integer JAUNT_REZOLATOR_HUFFWARE_ID = 10025;
    // the unique id within the huffware system for this script's commands.
    // it's used in llMessageLinked as the num parameter.
//////////////
// commands available from the library:
string RESET_REZOLATOR = "#reset";
    // tells the script to stop any previous efforts to rez children.
string REZ_CHILD_NOW = "#rezkd#";
    // requests this script to create a new child object.  this requires several
    // parameters to succeed: 1) object to rez, 2) conveyance mode for the rezzed object
    // to implement, 3) the chat channel to listen for and speak to new child on,
    // 4) the destination name for where the child should go, 5) a count of the full
    // set of known destinations, 6) the target where the jaunt should arrive at.
//////////////
// events generated by the library:
string REZOLATOR_EVENT_REZZED_CHILD = "#donekd";
    // an event generated for a previous rez child request.  this lets the caller know that
    // the child has been created and told what to do.  the single parameter is where the
    // jaunter has been rezzed.
string REZOLATOR_EVENT_RECON_FINISHED = "#rzcnfn";
    // an event generated when the recon process has concluded.  this includes parms:
    // number of good destinations.
//
//////////////

// important constants used internally...  these should not be changed willy nilly.

////////////// jaunt base API

// the following constants define how the script should behave (i.e., its conveyance mode).
// TWO_WAY_TRIP: the script jaunts using the current target vectors to get somewhere
//   and then takes the same pathway back, but in reverse order.
// AUTOREZ_JAUNTER: the script rezzes the first regular object in its inventory next to
//   the root telehub.  that object is loaded with the destination notecard and this script.
//   the rezzed object can then be used for the next few minutes to jaunt to the selected
//   destination.  the temporary object will use the ONE_WAY_TRIP mode.
// ONE_WAY_TRIP: the object containing this script will take the user to a particular
//   destination, but the object does not survive the trip.  it self-destructs after
//   reaching the destination.  this mode is used in conjunction with the AUTOREZ_JAUNTER
//   mode and should generally never be used on its own.
// RECONNAISSANCE_TRIP: a survey run to test out a particular path to get to a
//   destination.
integer TWO_WAY_TRIP = 1;
integer AUTOREZ_JAUNTER = 2;
integer ONE_WAY_TRIP = 3;
integer RECONNAISSANCE_TRIP = 4;

// values used to represent different stages of verification.
integer VERIFY_UNTESTED = -3;  // don't know destinations state yet.
integer VERIFY_SAFE = -4;  // the destination last tested safe.
integer VERIFY_UNSAFE_SO_FAR = -5;  // this cannot be done with simple jaunt.
integer VERIFY_UNSAFE_DECIDED = -6;  // this means the destination seems intractable.

integer MAXIMUM_PRIV_CHAN = 90000;
    // the largest amount we will ever subtract from the tricky parms in order to
    // tell the sub-jaunter which channel to listen on.

string VECTOR_SEPARATOR = "|";
    // how we separate components of vectors from each other.
string DB_SEPARATOR = "``";  // separating items in lists.

////////////// end jaunt base API

integer MAX_DEST_NAME = 24;  // the longest name that we store.

float NORMAL_TIMER_PERIOD = 1.0;  // our normal timer rate.

string RESET_COMMAND = "jreset";  // forget everything and start over.
string SHOW_COMMAND = "show";  // describe the known destinations.
string RECON_COMMAND = "recon";  // tells the jaunter to run reconnaissance on destinations.

float POSITION_CHECKING_INTERVAL = 0.11;
    // how frequently the waiting for arrival state will check where we are.

integer MAXIMUM_SLACKNESS = 108;
    // how many timer hits we'll allow before reverting to the default state.

integer FREE_MEM_REQUIRED = 1600;
    // we need at least this much memory before adding new targets.

// somewhere between two and 10 minutes (default) will elapse before first recon testing.
float MIN_RECON_SNOOZE = 120;
float MAX_RECON_SNOOZE = 1200;

// the actions that can be held in the action queue.
integer AQ_SHOW_DEST_RECORD = 23;  // a record to be shown to the user.
integer AQ_FINISH_SHOW_DESTS = 24;  // last record in a jaunt destinations list.
integer AQ_CLICK_ACTION_REQUEST = 25;  // the response should set our click action.
integer AQ_SELECT_DESTINATION = 27;  // the user has chosen a different destination.
integer AQ_DONE_CONFIGURING = 28;  // the last step of the configuration process has finished.

string OUR_COW_TAG = "jwkrz_dc";  // unique id for this script to talk to the data cow with.

//////////////

// global variables.

integer private_chat_channel;  // where sub-jaunters communicate with root.
integer conveyance_mode;  /// = TWO_WAY_TRIP;
    // default is a standard two way trip, there and back again.  note that the object
    // will fail to return if any lands in between are blocking object entry.  those
    // situations should use an auto-rez jaunter, which is specified by prefixing the
    // target vectors with AR.
integer last_verification_state;  // the verification state of the last destination we asked for.
string last_pathway;  // similar, but the last pathway we heard.
integer current_target_index;  // the location in the list that we would currently jaunt to.

integer good_destinations;  // computed during the recon process.

// jaunter target configuration...
list global_names;  // the names for each destination.

// jaunt trip variables...
vector eventual_destination;  // where we're headed, if we're headed anywhere.
integer slackness_counter;  // snoozes had while waiting for destination.
list full_journey;  // the full pathway we expect to jaunt on.
integer jaunt_responses_awaited;  // number of pending jumps in progress.

// root jaunter variables...
integer child_needs_setup;
    // true when a child has been rezzed and still needs to be filled with the script.

integer recon_started_yet = FALSE;
    // true if we've finished running through the reconnaissance process yet.

//////////////

// makes our click action change to the type requested, and sends the request
// out to all our sub-prims also.
set_click_action(integer action)
{
    llSetClickAction(action);
    // secret message to other prims to change click action.
    llMessageLinked(LINK_SET, CHANGE_CLICK_ACTION_HUFFWARE_ID, "", (string)action);
}

integer random_channel() { return -(integer)(llFrand(40000) + 20000); }

// show the list of options for someone who used the click-hold feature.
show_jaunt_menu(key last_toucher)
{
    llMessageLinked(LINK_THIS, MENUTINI_HUFFWARE_ID, SHOW_MENU_COMMAND,
        "jm" + HUFFWARE_PARM_SEPARATOR
        + "Jaunt to..." + HUFFWARE_PARM_SEPARATOR
        + wrap_item_list(global_names)
        + HUFFWARE_PARM_SEPARATOR
        + (string)random_channel() + HUFFWARE_PARM_SEPARATOR
        + (string)last_toucher);
}

// respond to the user's choice.
react_to_menu(string which_choice)
{
    integer indy;
    // find the specified destination and select it.
    indy = llListFindList(global_names, [which_choice]);
    if (indy >= 0) {
        current_target_index = indy;
        select_destination(current_target_index);
    }
}

// asks the jaunting library to take us to the target using a list of waypoints.
request_jaunt(list journey, integer forwards)
{
    // ask for a jump.
    jaunt_responses_awaited++;
    llMessageLinked(LINK_THIS, JAUNT_HUFFWARE_ID, JAUNT_LIST_COMMAND,
        wrap_item_list(journey) + HUFFWARE_PARM_SEPARATOR + (string)forwards);
    // stops the jaunter in its tracks.
    llMessageLinked(LINK_THIS, JAUNT_HUFFWARE_ID, FULL_STOP_COMMAND, ""); 
}

// this function returns TRUE if we are close enough to the "destination".
integer close_enough(vector destination)
{
    float PROXIMITY_REQUIRED = 0.1;
        // how close we must be to the target location to call it done.
        // matches current jaunting library proximity.
    return (llVecDist(llGetPos(), destination) <= PROXIMITY_REQUIRED);
}

// returns appropriate sit text for our current jaunt mode.
string sit_text()
{
    if (conveyance_mode == AUTOREZ_JAUNTER) return "Rez Ride";
    return "Jaunt";
}

// plops a new destination on the end of the lists.
add_destination(string name, string path, integer verif)
{
//    if (DEBUGGING) log_it("adding " + name + " with path " + path);
    global_names += [ name ];
    // send our new information to the data cow.  we store the information encoded as
    // two separated items, where the first element is the destination and the second
    // is the verification state.
    string new_entry = wrap_item_list([path, verif]);
    llMessageLinked(LINK_THIS, DATA_COW_HUFFWARE_ID, ADD_ITEM_COMMAND,
        wrap_parameters([name, new_entry]));
}

// sets up the initial state, if this script has just started, or it
// cleans up the state, if the script was already running.
initialize_jaunter()
{
    llSetTimerEvent(0.0);  // cancel any existing timers.

    // reset all the important variables.
    global_names = [];
    current_target_index = 0;
    good_destinations = 0;

    llSitTarget(<0, 0, 0.1>, ZERO_ROTATION);
        // set a rudimentary sit target or opensim won't give us our changed events.
    // load up an arrival sound if any exist.
    if (llGetInventoryNumber(INVENTORY_SOUND))
        llPreloadSound(llGetInventoryName(INVENTORY_SOUND, 0));
    // reset the parallel processing scripts since we've restarted.
    llMessageLinked(LINK_THIS, DATA_COW_HUFFWARE_ID, RESET_LIST_COMMAND, "");
    llMessageLinked(LINK_THIS, JAUNT_REZOLATOR_HUFFWARE_ID, RESET_REZOLATOR, "");
    
    // see if we can load our configuration from notecards and landmarks.
    // we pick a private channel that fits in between our rez parm ranges.
    private_chat_channel = (integer)randomize_within_range(-MAXIMUM_PRIV_CHAN, -1, FALSE);
    // request that all the config get loaded up.
    llMessageLinked(LINK_THIS, JAUNT_CONFIGURATION_HUFFWARE_ID, LOAD_ALL_CONFIGURATIONS,
         wrap_parameters([private_chat_channel, conveyance_mode]));
    // during a normal trip, we will make sure the notecard reading doesn't stall.
    // the recon and one way jaunters don't need this; they're temporary.
    // we allow this number of seconds before notecard reader is awol.
    llSetTimerEvent(94);

    conveyance_mode = TWO_WAY_TRIP;  // default init in opensim friendly way.

    // set up some of the object properties...
    string msg;
    if (SHOW_TEXT) msg = "?...";
    llSetText(msg, (vector)TEXT_COLOR, 1.0);  // initial floating text for jaunter.
    llSetSitText(sit_text());  // change to the proper text for our mode.
}

init_normal_runtime()
{
    child_needs_setup = FALSE;
    // listen to the general public for commands.
    llListen(0, "", NULL_KEY, "");
    // set up a root jaunter.
    recon_started_yet = FALSE;        
    llSetTimerEvent(randomize_within_range(MIN_RECON_SNOOZE, MAX_RECON_SNOOZE, FALSE));
}

// signal that we are where we were going.
proclaim_arrival()
{
    if (conveyance_mode == TWO_WAY_TRIP) {
        // sing a little song, if there's a sound to use.
        if (llGetInventoryNumber(INVENTORY_SOUND))
            llTriggerSound(llGetInventoryName(INVENTORY_SOUND, 0), 1.0);
    }
}

string verification_name(integer enumtype)
{
    if (enumtype == VERIFY_SAFE) return "ok";
    else if (enumtype == VERIFY_UNSAFE_SO_FAR) return "uhh";
    else if (enumtype == VERIFY_UNSAFE_DECIDED) return "far";
    // catch-all, including untested.
    return "?";
}

// returns true if the slackness counter awaiting things has elapsed.
integer check_for_timeout()
{
    if (slackness_counter++ > MAXIMUM_SLACKNESS) {
        // go back to the main state.  we took too long.
        log_it("timed out!");
        llUnSit(llAvatarOnSitTarget());  // don't hang onto the avatar for this error.
        llSetTimerEvent(0.0);
        return TRUE;
    }
    return FALSE;
}

//////////////

// action queue for postponed activities.  the first field held in a list item here
// is an integer action code.  the format of the remaining parameters is up to the
// caller, and they can be used as the final parameters for when the queued action
// gets handled.
list action_queue;

// looks at the action code at the head of the queue without removing the action.
integer peek_action_code()
{
    list fields = llParseString2List(llList2String(action_queue, 0), [HUFFWARE_PARM_SEPARATOR], []);
    return extract_action_code(fields);
}

// extracts the action code from a retrieved list.
integer extract_action_code(list to_parse) { return llList2Integer(to_parse, 0); }

// removes the current head of the action queue and returns it.
list pop_action_record()
{
    if (llGetListLength(action_queue) == 0) {
//        if (DEBUGGING) log_it("failure in action q: no entries.");
        return [];
    }
    list top_action = llParseString2List(llList2String(action_queue, 0), [HUFFWARE_PARM_SEPARATOR], []);
    action_queue = llDeleteSubList(action_queue, 0, 0);
    jaunt_responses_awaited--;  // one less thing to wait for.
    return top_action;
}

// adds a new action to the end of the action queue.
push_action_record(integer action, list added_parms)
{
    action_queue += [ wrap_parameters([action] + added_parms) ];
    jaunt_responses_awaited++;  // add one counter to our pending responses.
}

//////////////

string show_buffer;  // used by the destination list showing code below.

// requests a dump of all the jaunt destinations so we can show them to the user.
get_destination_records()
{
    show_buffer = "[mem " + (string)llGetFreeMemory() + "]";  // set the first line in the output.
    
    integer indy;
    for (indy = 0; indy < llGetListLength(global_names); indy++) {
        string curr = llList2String(global_names, indy);
        // ask for this destination record.
        llMessageLinked(LINK_THIS, DATA_COW_HUFFWARE_ID, TAGGED_GET_ITEM_COMMAND,
            wrap_parameters([OUR_COW_TAG, curr]));
        integer action = AQ_SHOW_DEST_RECORD;
        if (indy == llGetListLength(global_names) - 1) {
            // if it's the last item, make sure we signal that.
            action = AQ_FINISH_SHOW_DESTS;
        }
        push_action_record(action, []);
    }
}

// displays the list of destinations in normal chat that the data cow is handing us.
show_destination_record(string loc_name, string pathway, integer verif, integer action)
{
    integer MAX_CHAT = 512;  // the largest we let the buffer get before saying it.
    show_buffer += "   " + loc_name + " ("
        + verification_name(verif) + ") ↣ " + pathway;
    // if we're done, add a note about quality of targets.
    if (action == AQ_FINISH_SHOW_DESTS) {
        show_buffer += "\n  (" + (string)good_destinations + " safe & "
            + (string)(llGetListLength(global_names) - good_destinations)
            + " tough ones)";
    }
        
    // if we're done or if the string gets too big, print and reset it.
    if ( (action == AQ_FINISH_SHOW_DESTS) || (llStringLength(show_buffer) >= MAX_CHAT) ) {
        llWhisper(0, show_buffer);
        show_buffer = "";
    }
}

// returns the value of a boolean variable definition.
integer parse_bool_def(string def)
{ return !(llGetSubString(def, find_substring(def, "=") + 1, -1) == "0"); }

// processes a configuration option from the notecard.
handle_config_item(string item)
{
    item = llGetSubString(item, 1, -1);  // chop the command signifier character.
    if (is_prefix(item, "show_text")) {
        // they are controlling whether to show the text label or not.
        SHOW_TEXT = parse_bool_def(item);
    } else if (is_prefix(item, "add_name")) {
        ADD_NAME = parse_bool_def(item);
    }
}

// processes items coming from the configuration library.
consume_configuration_items(string msg, string id)
{
    list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
    integer mem_okay = llGetFreeMemory() >= FREE_MEM_REQUIRED;  // false if we're low on memory.
    if (!mem_okay) return;
    integer indy;
    for (indy = 0; indy < llGetListLength(parms); indy += 2) {
        string name = llList2String(parms, indy);
        if (is_prefix(name, ":")) {
            handle_config_item(name);
            indy--;  // skip back so we continue looking for destinations.
        } else {
            // this should be a normal pair of destination vector and name.
            integer verif_state = VERIFY_UNTESTED;
            // check if it's a wacky location.
            string map_coord = llList2String(parms, indy + 1);
            list chewed = chewed_destination(map_coord);
            if (outside_of_sim((vector)llList2String(chewed, 0))) {
                if (DEBUGGING) log_it("ruled map_coor " + (string)map_coord + " out of sim.");
                // make sure we consider the tp location "unsafe", since we can't run in-sim
                // reconnaissance against it.  also, we must postpone marking it until
                // the destination is listed, given current implementation.
                verif_state = VERIFY_UNSAFE_DECIDED;
            }
            add_destination(name, llList2String(parms, indy + 1), verif_state);
        }
    }
}

// handles responses about our list coming back from the data cow.
integer answer_to_the_cow(string msg, string id, list parms)
{
    string tag = llList2String(parms, 0);
    if (tag != OUR_COW_TAG) return FALSE;  // was not for us.
    parms = llDeleteSubList(parms, 0, 0);  // trim out the tag.
    list action_record = pop_action_record();
    integer actyo = extract_action_code(action_record);
    list split = llParseString2List(llList2String(parms, 1), [HUFFWARE_ITEM_SEPARATOR], []);
    if ( (actyo == AQ_SHOW_DEST_RECORD) || (actyo == AQ_FINISH_SHOW_DESTS) ) {
        show_destination_record(llList2String(parms, 0), llList2String(split, 0),
            llList2Integer(split, 1), actyo);
    } else if (actyo == AQ_CLICK_ACTION_REQUEST) {
        // make sure we're in the right mode for clicking based on how
        // many destinations we have.  if there's only one, we might as
        // well make this jaunter just go there.
        // note: just discovered that we cannot set the click action if the jaunter
        // is multiple prims.  that's totally fubar.  it means we cannot correctly
        // set the jaunter main body to show 'sit' as the option, even when jaunting
        // directly to the destination is possible.  freaking lindens.  maybe osgrid
        // will fix this someday.
        if (llGetListLength(global_names) == 1) {
            if (llList2Integer(split, 1) != VERIFY_UNSAFE_DECIDED)
                set_click_action(CLICK_ACTION_SIT);
            else
                // we have a non-local jaunter, so set up for map clicking.
                set_click_action(CLICK_ACTION_TOUCH);
        } else set_click_action(CLICK_ACTION_TOUCH);
    } else if (actyo == AQ_SELECT_DESTINATION) {
        integer verif = llList2Integer(split, 1);
        // we only switch conveyance mode if we have not been told to
        // do the one-way trip or recon.
        if (verif == VERIFY_SAFE) conveyance_mode = TWO_WAY_TRIP;
        else conveyance_mode = AUTOREZ_JAUNTER;
        llSetSitText(sit_text());  // change to the proper text for our mode.
        full_journey = [ vector_chop(llGetPos()) ] + chewed_destination(llList2String(split, 0));
        integer last_selecting_index = llList2Integer(action_record, 1);
        string destname = llList2String(global_names, last_selecting_index);
        llWhisper(0, "Next stop: " + destname);
        text_label_for_destination(destname);
        last_verification_state = verif;
        last_pathway = llList2String(split, 0);
    } else if (actyo == AQ_DONE_CONFIGURING) {
        // done with notecard reading, but we still need reconnaissance.
        // we'll do that in the normal runtime state.
        return TRUE;        
    }
    return FALSE;  // unknown request or a fall-through.
}

// asks the data cow for our current set of configured destinations and their
// requisite click actions.
request_destinations()
{
    push_action_record(AQ_CLICK_ACTION_REQUEST, []);
    llMessageLinked(LINK_THIS, DATA_COW_HUFFWARE_ID, TAGGED_GET_ITEM_COMMAND,
        wrap_parameters([OUR_COW_TAG, llList2String(global_names, 0)]));
}

// processes link messages received from support libraries.
integer handle_link_message(integer which, integer num, string msg, string id)
{
    // is it an answer about our configuration?
    if (num == JAUNT_CONFIGURATION_HUFFWARE_ID + REPLY_DISTANCE) {
        if (msg == JAUNT_CFG_DATA_EVENT) {
            // eat these configuration items we were given.
            consume_configuration_items(msg, id);
        } else if (msg == JAUNT_CFG_EOF) {
            // we have finished up on the configuration.
            completed_configuration();
        }
        return FALSE;
    }
    
    // is it a jaunting library response?
    if (num == JAUNT_HUFFWARE_ID + REPLY_DISTANCE) {
        jaunt_responses_awaited--;  // one less response being awaited.
        if (jaunt_responses_awaited < 0) {
            if (DEBUGGING) log_it("error: j-rsp-awtd<0");
            jaunt_responses_awaited = 0;
        }
        return FALSE;
    }
    
    // is it an event from the rezolator script?
    if (num == JAUNT_REZOLATOR_HUFFWARE_ID + REPLY_DISTANCE) {
        list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
        if (msg == REZOLATOR_EVENT_REZZED_CHILD) {
            // we know the child is ready for us now.
            child_needs_setup = FALSE;
//            if (DEBUGGING) log_it("setting journey to rez place " + vector_chop(llGetPos()));
            full_journey = [ vector_chop(llGetPos()), llList2String(parms, 0) ];
            if (DEBUGGING) log_it("heard kid is ready, helping.");
            request_jaunt(full_journey, TRUE);
            eventual_destination = (vector)llList2String(full_journey, llGetListLength(full_journey) - 1);
            llSetTimerEvent(POSITION_CHECKING_INTERVAL);
        } else if (msg == REZOLATOR_EVENT_RECON_FINISHED) {
            good_destinations = llList2Integer(parms, 0);
            if (DEBUGGING) log_it("recon finished, total=" + llGetListLength(global_names) + " good=" + good_destinations);
            request_destinations();
            select_destination(current_target_index);
        }
        return FALSE;
    }

    // is this a menu response from the user?
    if (num == MENUTINI_HUFFWARE_ID + REPLY_DISTANCE) {
        // all we care about in our parms is the choice made.
        react_to_menu(llList2String(llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []), 1));
        return FALSE;
    }
    // or maybe it's a piece of data from the data cow.
    if (num == DATA_COW_HUFFWARE_ID + REPLY_DISTANCE) {
        return answer_to_the_cow(msg, id, llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []));
    }
    // perhaps it's a click on a button.
    if (num == BUTTON_PUSHER_HUFFWARE_ID + REPLY_DISTANCE) {
        if (msg == BUTTON_PUSHED_ALERT) {
            list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
            if (llList2String(parms, 0) == JAUNT_NEXT_BUTTON_NAME) {
                select_next_destination();
            } else if (llList2String(parms, 0) == JAUNT_MENU_BUTTON_NAME) {
                show_jaunt_menu(llList2String(parms, 1));
            }
        }
    }

    return FALSE;
}

// make some noise about getting configured right.
completed_configuration()
{
    current_target_index = 0;  // set proper index.    
    get_destination_records();

    // set up the first destination.
    select_destination(current_target_index);

    // schedule the request asking how many destinations there are.
    request_destinations();
/*    push_action_record(AQ_CLICK_ACTION_REQUEST, []);
    llMessageLinked(LINK_THIS, DATA_COW_HUFFWARE_ID, TAGGED_GET_ITEM_COMMAND,
        wrap_parameters([OUR_COW_TAG, llList2String(global_names, 0)]));*/

    // add an item in the queue to actually begin jaunt services.
    push_action_record(AQ_DONE_CONFIGURING, []);
    llMessageLinked(LINK_THIS, DATA_COW_HUFFWARE_ID, TAGGED_GET_ITEM_COMMAND,
            wrap_parameters([OUR_COW_TAG, llList2String(global_names, 0)]));
}

// process what we hear in open chat and on our special channels.
// this function should always return FALSE unless it's been given enough info
// to enter a new state, in which case it should return true.
integer listen_to_voices(integer channel, string name, key id, string message)
{
    if (channel == 0) {
        // provide common command handling.
        if (message == SHOW_COMMAND) {
            // anyone can use this command.
            get_destination_records();
            return FALSE;
        }
        // the rest of the commands only listen to the owner.
        if (id != llGetOwner()) return FALSE;
        if (message == RESET_COMMAND) llResetScript();
        else if (message == RECON_COMMAND) {
            // if they want a recon, we'll do it right away.
            recon_started_yet = FALSE;
            // we want to reset the rezolator's state, in case it had done this before.
            llMessageLinked(LINK_THIS, JAUNT_REZOLATOR_HUFFWARE_ID, RESET_REZOLATOR, "");
            llSetTimerEvent(0.4);
        }
        return FALSE;
    }

    return FALSE;  // we do not want to go to next state; stay in current one.
}

// shows our next target for jaunting above the object.
text_label_for_destination(string target)
{
    string msg;
    if (SHOW_TEXT) {
        if (ADD_NAME) msg += llGetObjectName() + ":\n";
        msg += "↣ " + target;
    }
    llSetText(msg, (vector)TEXT_COLOR, 1.0);
}

select_next_destination()
{
    // picks the next place in the list for the destination, or wraps around.
    current_target_index++;
    if (current_target_index >= llGetListLength(global_names))
        current_target_index = 0;
    select_destination(current_target_index);
}
        
touchy_feely()
{
    // check the safety of the target and show the map if the conditions are right.
    vector check_posn = (vector)llList2String(chewed_destination(last_pathway), 0);
///    if (outside_of_sim(check_posn)) {
    /////old && (llGetListLength(global_names) == 1) ) {
        // bring up the map; maybe we can't get there from here.  definitely out of sim.
        llMapDestination(llGetRegionName(), check_posn, ZERO_VECTOR);
        proclaim_arrival();
///    }
}

// if the string "to_chew" looks like an offset vector, we return the calculated position.
// otherwise we just convert the string to a list of vectors.
list chewed_destination(string dests)
{

//hmmm: document this!!!
//      as in, we support the offset format!

    // look for our special start character for offsets.
    if (is_prefix(dests, "o"))
        return [ (vector)llDeleteSubString(dests, 0, find_substring(dests, "<") - 1) + llGetPos() ];

//        if (DEBUGGING) log_it("decided for normal jaunt instead.");        
    // jaunt to list of absolute coordinates.
    return llParseString2List(dests, [VECTOR_SEPARATOR], []);
}

// sets the current destination to the vector at "which_index".
select_destination(integer which_index)
{
    push_action_record(AQ_SELECT_DESTINATION, [which_index]);
    llMessageLinked(LINK_THIS, DATA_COW_HUFFWARE_ID, TAGGED_GET_ITEM_COMMAND,
        wrap_parameters([OUR_COW_TAG, llList2String(global_names, which_index)]));
}

// processes the timer events from normal runtime.
handle_normal_runtime_timer()
{
    if (!recon_started_yet) {
        if (DEBUGGING) log_it("moving on to recon finally.");
        llMessageLinked(LINK_THIS, JAUNT_REZOLATOR_HUFFWARE_ID, REZ_CHILD_NOW,
            wrap_parameters( [llGetInventoryName(INVENTORY_OBJECT, 1),
                RECONNAISSANCE_TRIP, private_chat_channel,
                "n",  // don't care about a particular name.
                llGetListLength(global_names),
                "n"  // no particular path.
                ]));
        recon_started_yet = TRUE;  // well, started really.                
    }
    llSetTimerEvent(0.0);  // stop coming here.
}

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

///////////////

// returns TRUE if the value in "to_check" specifies a legal x or y value in a sim.
integer valid_sim_value(float to_check)
{
    if (to_check < 0.0) return FALSE;
    if (to_check >= 257.0) return FALSE;
    return TRUE;
}

// returns TRUE if the "to_check" vector is a location outside of the current sim.
integer outside_of_sim(vector to_check)
{
    return !valid_sim_value(to_check.x) || !valid_sim_value(to_check.y);
}

// returns text for a floating point number, but includes only
// two digits after the decimal point.
string float_chop(float to_show)
{
    integer mant = llAbs(llRound(to_show * 100.0) / 100);
    string neg_sign;
    if (to_show < 0.0) neg_sign = "-";
    string dec_s = (string)((llRound(to_show * 100.0) - mant * 100) / 100.0);
    dec_s = llGetSubString(llGetSubString(dec_s, find_substring(dec_s, ".") + 1, -1), 0, 2);
    // strip off all trailing zeros.
    while (llGetSubString(dec_s, -1, -1) == "0")
        dec_s = llDeleteSubString(dec_s, -1, -1);
    string to_return = neg_sign + (string)mant;
    if (llStringLength(dec_s)) to_return += "." + dec_s;
    return to_return;
}

// returns a prettier form for vector text, with chopped floats.
string vector_chop(vector to_show)
{
    return "<" + float_chop(to_show.x) + ","
        + float_chop(to_show.y) + ","
        + float_chop(to_show.z) + ">";
}

// joins a list of parameters using the parameter sentinel for the library.
string wrap_parameters(list to_flatten)
{ return llDumpList2String(to_flatten, HUFFWARE_PARM_SEPARATOR); }
//
// joins a list of sub-items using the item sentinel for the library.
string wrap_item_list(list to_wrap)
{ return llDumpList2String(to_wrap, HUFFWARE_ITEM_SEPARATOR); }

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

// returns the portion of the list between start and end, but only if they are
// valid compared with the list length.  an attempt to use negative start or
// end values also returns a blank list.
list chop_list(list to_chop, integer start, integer end)
{
    integer last_len = llGetListLength(to_chop) - 1;
    if ( (start < 0) || (end < 0) || (start > last_len) || (end > last_len) ) return [];
    return llList2List(to_chop, start, end);
}

// returns the index of the first occurrence of "pattern" inside
// the "full_string".  if it is not found, then a negative number is returned.
integer find_substring(string full_string, string pattern)
{
    string full_lower = llToLower(full_string);
    return llSubStringIndex(full_lower, pattern);
}

// returns TRUE if the "prefix" string is the first part of "compare_with".
integer is_prefix(string compare_with, string prefix)
{ return find_substring(compare_with, prefix) == 0; }

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

//////////////

// default state scrounges for information in a notecard and looks for landmarks in
// inventory to add as destinations.
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
        if (DEBUGGING) log_it("=> default, mem=" + (string)llGetFreeMemory());
        initialize_jaunter();
    }

    timer() {
        // config loading timed out.
        log_it("config timeout; resetting.");
        llResetScript();
    }
    
    // process the response from the noteworthy library.
    link_message(integer which, integer num, string msg, key id) {
        if (handle_link_message(which, num, msg, id)) {
            state normal_runtime;
        }
    }
    
    on_rez(integer parm) { state default; }  // start-up from scratch.
}

// the normal state is pretty calm; the jaunter just sits there waiting for
// an avatar who needs it to do something.
state normal_runtime
{
    state_entry() {
        if (DEBUGGING) log_it("=> normal");
        init_normal_runtime();
    }

    listen(integer channel, string name, key id, string message) {
        if (listen_to_voices(channel, name, id, message)) {
            // we're ready to take off now.
            state jaunting_now;
        }
    }

    timer() { handle_normal_runtime_timer(); }

    touch_end(integer total_number)
    {
        if (llDetectedLinkNumber(0) != llGetLinkNumber()) return;
        touchy_feely();
    }

    changed(integer change) {
        // always react to a change by resetting the label, in hopes we'll see
        // the targeting arrow properly.
        string destname = llList2String(global_names, current_target_index);
        text_label_for_destination(destname);
        // now do our 'real' activities for changes.
        if (change & CHANGED_INVENTORY) {
            log_it("inventory changed; restarting.");
            llResetScript();
        }
        if (!(change & CHANGED_LINK)) return;  // don't care then.
        if (llAvatarOnSitTarget() == NULL_KEY) return;  // there is no one sitting now.
        state jaunting_now;  // sweet, we're off.
    }
        
    // process the response from the menu library.
    link_message(integer which, integer num, string msg, key id) {
        handle_link_message(which, num, msg, id);
    }

    on_rez(integer parm) { state default; }  // start-up from scratch.
}

// once someone is trying to jump to a target, this state processes the request.
state jaunting_now
{
    state_entry() {
        if (DEBUGGING) log_it("=> jauntnow, posn=" + vector_chop(llGetPos()));
        jaunt_responses_awaited = 0;  // nothing pending right now.
        slackness_counter = 0;
        if (conveyance_mode == AUTOREZ_JAUNTER) {
            // we're going to create a kid, so we will not change state until we hear from it.
            child_needs_setup = TRUE;
            // give the person a vehicle to ride to the unsafe locale.  we assume the
            // one way jaunter is the first inventory item.
            llMessageLinked(LINK_THIS, JAUNT_REZOLATOR_HUFFWARE_ID, REZ_CHILD_NOW,
                wrap_parameters([llGetInventoryName(INVENTORY_OBJECT, 0),
                    ONE_WAY_TRIP, private_chat_channel,
                    llList2String(global_names, current_target_index),
                    llGetListLength(global_names),
                    llDumpList2String(full_journey, VECTOR_SEPARATOR) ]));
        } else {
//            if (DEBUGGING) log_it("now going to first locat: " + (string)full_journey);
            // patch the journey list for our most current location.
            full_journey = [ vector_chop(llGetPos()) ] + llDeleteSubList(full_journey, 0, 0);

            // most jaunters go to at least the first location...
            request_jaunt(full_journey, TRUE);
            eventual_destination = (vector)llList2String(full_journey, llGetListLength(full_journey) - 1);
            llSetTimerEvent(POSITION_CHECKING_INTERVAL);
        }
    }

    listen(integer channel, string name, key id, string message) {
        listen_to_voices(channel, name, id, message);
    }

    timer() {
        if (jaunt_responses_awaited || child_needs_setup) {
            // we are not quite there yet.
            if (check_for_timeout()) state normal_runtime;  // oops.
            return;  // not time yet.
        }
        // we got to where we were going, maybe.
        // now unseat the avatar.  this leaves her at the destination.
        llUnSit(llAvatarOnSitTarget());
        state arrived_at_target;
    }

    // process the response from the jaunting library.
    link_message(integer which, integer num, string msg, key id)
    { handle_link_message(which, num, msg, id); }

    on_rez(integer parm) { state default; }  // start-up from scratch.
}

// this state is activated when the first jaunt is complete.
state arrived_at_target
{
    state_entry() {
        if (DEBUGGING) log_it("=> arv_targ, pos=" + vector_chop(llGetPos()));
        // we are close enough; get back to work.
        jaunt_responses_awaited = 0;  // nothing pending right now.
        if ( ///(conveyance_mode != RECONNAISSANCE_TRIP) && 
        (conveyance_mode != AUTOREZ_JAUNTER) )
            llWhisper(0, "↣ " + llList2String(global_names, current_target_index));

        // we've gotten where we were going.
        proclaim_arrival();

        // reverse direction and head back without rider.
        eventual_destination = (vector)llList2String(full_journey, 0);
        request_jaunt(full_journey, FALSE);
        llSetTimerEvent(POSITION_CHECKING_INTERVAL);
        slackness_counter = 0;
    }

    timer() {
        if (jaunt_responses_awaited) {
            // we are not quite there yet.
            if (check_for_timeout()) state normal_runtime;  // oops.
            return;  // not time yet.
        }
        if (!close_enough(eventual_destination))
            log_it("far away, path okay?");
        state normal_runtime;
    }
    
    // process the response from the jaunting library.
    link_message(integer which, integer num, string msg, key id)
    { handle_link_message(which, num, msg, id); }

    on_rez(integer parm) { state default; }  // start-up from scratch.
}

