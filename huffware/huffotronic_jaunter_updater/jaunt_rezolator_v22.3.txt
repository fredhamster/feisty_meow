
// huffware script: jaunt rezolator, by fred huffhines.
//
// this script supports the jaunt wik rez script by dealing with the rezzed objects.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

// global constants that can be changed to good effect.

integer DEBUGGING = FALSE;  // set this to true for noisier run times.

// important constants used internally...  these should not be changed willy nilly.

// begin jaunt base API:

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

// indicates a reconnaissance command when found as a prefix on chat text
// on our official recon channel.
string CHILD_CHAT_TEXT = "#rcn";
string READY_TEXT = "-y";  // lets us know that a jaunter is ready to be filled.
string RETURN_WORD = "-b";  // used to signal a returning recong jaunter.

// values used to represent different stages of verification.
integer VERIFY_UNTESTED = -3;  // don't know destinations tate yet.
integer VERIFY_SAFE = -4;  // the destination last tested safe.
integer VERIFY_UNSAFE_SO_FAR = -5;  // this cannot be done with simple jaunt.
integer VERIFY_UNSAFE_DECIDED = -6;  // this means the destination seems intractable.

integer ONE_WAY_TRICKY_PARM = -100000;
    // used to signal to the one way jaunter that it's taking a single pathway.
integer RECONNAISSANCE_TRICKY_PARM = -200000;
    // on_rez parm indicates that this will be a recon trip to check out a path.
integer MAXIMUM_PRIV_CHAN = 90000;
    // the largest amount we will ever subtract from the tricky parms in order to
    // tell the sub-jaunter which channel to listen on.

string VECTOR_SEPARATOR = "|";
    // how we separate components of vectors from each other.
string DB_SEPARATOR = "``";  // separating items in lists.

// end of jaunt base API.

integer REZ_KID_FIELDS_NEEDED = 6;  // the rez kid API method needs this many parms.

integer MAXIMUM_RECON_ATTEMPTS = 7;
    // maximum tries to find a path for the hard cases (after first simple jaunt fails).

float TIMER_PERIOD = 1.0;  // we run a pretty slow timer to check on destinations.

integer MAXIMUM_SNOOZES = 20;  // how long we let a pending action go before declaring it failed.

string OUR_COW_TAG = "l_jrzltr_dc";
    // a hopefully unique id for this script to talk to the data cow with.

// the actions that can be held in the action queue.
integer AQ_ONEWAY_JAUNTER_VOYAGE = 40;  // information will be used for one-way jaunter.
integer AQ_PREP_RECON_JAUNTER = 41;  // info for the reconnaissance jaunter to go check out.
integer AQ_CHECK_HARD_CASE = 42;  // working on hard cases to get to destination somehow.
integer AQ_ACQUIRE_BASE_PATH = 43;  // we are grabbing a random path that we think is healthy.

string QUADRANT_TAG_NAME = "q:";  // a piece of text we put in front of generated destinations.

integer MAXIMUM_SAFE_LOCATIONS = 28;  // we'll track this many extra locations.
float MINIMUM_DISTANCE_FOR_EXTRAS = 10.0;  // how far a new locale must be to add.

integer MAX_CHAT_REPEAT = 3;  // give instructions N times in case things are slow.
float SLEEPY_TIME = 0.02;  // how long to snooze between chats.

//////////////

// global variables.

list global_verifications;  // we gradually acquire knowledge of the health of the destinations.
string global_object_name;  // the object to rez from inventory.
string global_destination_name;  // the name of the place where the jaunter should go.
vector global_rez_place;  // starting location for the rezzed jaunter.
integer conveyance_mode;  // how the rezzed jaunter should process its destination.

integer private_id_from_listen;  // set when we listen to our private channel.
integer private_chat_channel;  // where sub-jaunters communicate with root.

integer serving_root;  // if this is true, it means we are serving a root jaunter.

integer destinations_total;  // how many destinations are known in total?  we compute this.
integer destinations_real;  // how many are user-configured destinations?

integer snooze_counter;  // how many times have we slept during a process?

integer heard_from_root;  // true if the root jaunter has told us (a child jaunter) where to go.

// recon variables...
integer is_recon_pending;  // are we awaiting a reconnaissance?
integer trying_hard_cases;  // are we already working on the tough ones?
integer current_recon_index;  // where is the recon jaunter testing.
integer recon_reattempts;  // how many tries did we take?
integer done_reconnoiter;  // is recon process finished?
integer succeeded_recon;  // set to true if close enough.
integer good_destinations_counted;  // a tally of how many destinations looked healthy.
integer ungenerated_good_destinations_counted;  // counts only user selected good dests.
vector final_destination;  // where the recon process is really headed.
integer partial_path_count;  // the counter for partial path replies.
list safe_locales_seen;  // locations that we have a safe route to, as reported by recons.

// jaunt trip variables...
list full_journey;  // the full pathway we expect to jaunt on.
integer last_verification;  // the last state we heard.

// root jaunter variables...
integer child_needs_setup;
    // true when a child has been rezzed and still needs to be filled with the script.

// the API for this library script to be used in other scripts.
//////////////
// do not redefine these constants.
integer JAUNT_REZOLATOR_HUFFWARE_ID = 10025;
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
// commands available from the library:
string RESET_REZOLATOR = "#reset";
    // tells the script to stop any previous efforts to rez children.
string REZ_CHILD_NOW = "#rezkd#";
    // requests this script to create a new child object.  this requires several
    // parameters to succeed: 1) object to rez, 2) conveyance mode for the rezzed object
    // to implement, 3) the chat channel to listen for and speak to new child on,
    // 4) the destination name for where the child should go, 5) a count of the full
    // set of known destinations, 6) the target where the jaunt should arrive at.
string REPORT_CHILD_REZZED = "#reziam";
    // requests that this class report to the root jaunter that a child has rezzed and
    // is ready for service.  there are no required parameters.
//hmmm: combine the above with the below!
string REZOLATOR_CHILD_SUPPORT = "#rzsup";
    // used by child jaunters to request that the rezolator handle things for them.
    // the first parameter is the startup parameter handed to the control script.
string REZOLATOR_CHILD_RETURNED = "#rezdon";
    // used by the child jaunter to tell the rezolator that it has jumped to wherever it
    // was supposed to and is ready to report to the parent, if needed.  the first parameter
    // required for the rezolator is whether the jump was successful (1) or not (0), and
    // the second parameter should be the last safe position that the jaunter was at when
    // it was possibly closest to the target.
//////////////
// events generated by the library:
string REZOLATOR_EVENT_REZZED_CHILD = "#donekd";
    // an event generated for a previous rez child request.  this lets the caller know that
    // the child has been created and told what to do.  the single parameter is where the
    // jaunter has been rezzed.
string REZOLATOR_EVENT_GOT_INSTRUCTIONS = "#rzsta";
    // the root jaunter has given a child instructions on where to go.  this info includes
    // the name of the destination, the pathway to get there, and the conveyance mode.
string REZOLATOR_EVENT_RECON_FINISHED = "#rzcnfn";
    // an event generated when the recon process has concluded.  this includes parms:
    // number of good destinations.
//
//////////////

// requires click action API...
//////////////
integer CHANGE_CLICK_ACTION_HUFFWARE_ID = 10024;
    // a unique ID within the huffware system for this script.
// the API only supports the service number; there are no commands to pass in the "msg"
// parameter.  the id does accept a parameter, which is the type of click action to begin
// using for this prim.
//////////////

// requires data cow library...
//////////////
// do not redefine these constants.
integer DATA_COW_HUFFWARE_ID = 10017;
    // the unique id within the huffware system for the jaunt script to
    // accept commands on.  this is used in llMessageLinked as the num parameter.
//////////////
//string RESET_LIST_COMMAND = "reset_L";  // flushes out the currently held list.
string ADD_ITEM_COMMAND = "add_I";
    // adds items to the list.  this is a list of pairs of name/value, where the name is
    // how the item will be looked up from the list, and the value is the contents to be stored.
//string REMOVE_ITEM_COMMAND = "rm_I";
    // accepts a list of names for items.  all the mentioned ones will be removed from the list.
//string GET_ITEM_COMMAND = "get_I";
    // retrieves the item's contents for a given name.  first parm is the name.  if there
    // are other parameters, then they are taken as other items to return also.
    // the return data will be pairs of <name, entry> for each of the names in the request.
string TAGGED_GET_ITEM_COMMAND = "get_T";
    // like get item, except the first parameter is an identifier that the caller wants to
    // use to tag this request.  the other parameters are still taken as names.  the response
    // will contain the tag as the first item, then the <name, entry> pairs that were found.
//////////////

// sets up the initial state, if this script has just started, or it
// cleans up the state, if the script was already running.
initialize_rezolator()
{
    if (DEBUGGING) log_it("start mem=" + (string)llGetFreeMemory());
    global_rez_place = llGetPos();
    heard_from_root = FALSE;  // we don't know root from adam yet.
}

// sets up the rezolator to be a child jaunter and report to a root jaunter.
provide_child_support(integer startup_parm)
{
    serving_root = FALSE;
    // we are serving as a special purpose jaunter object.
    // set our method of jaunting based on the secret signal.
    if (-(startup_parm - ONE_WAY_TRICKY_PARM) < MAXIMUM_PRIV_CHAN) {
        conveyance_mode = ONE_WAY_TRIP;
        private_chat_channel = startup_parm - ONE_WAY_TRICKY_PARM;
        if (DEBUGGING) log_it("one way child: private chat on " + private_chat_channel);
    } else if (-(startup_parm - RECONNAISSANCE_TRICKY_PARM) < MAXIMUM_PRIV_CHAN) {
        conveyance_mode = RECONNAISSANCE_TRIP;
        private_chat_channel = startup_parm - RECONNAISSANCE_TRICKY_PARM;
        if (DEBUGGING) log_it("recon child: private chat on " + private_chat_channel);
    }

    if (private_id_from_listen) {
        // toss old listening since we're working on a new deal now.
        llListenRemove(private_id_from_listen);
    }
    // listen to chats on our special channel.
    private_id_from_listen = llListen(private_chat_channel, "", NULL_KEY, "");
    if (DEBUGGING) log_it("listening for cmds on chan " + (string)private_chat_channel);
}

// this handles the case where we need to send out another seeker drone.
// true is returned if this function thinks things are doing well.
integer advance_reconnaissance_position()
{
    if (!trying_hard_cases) {
        list added_parms = [];
        if (current_recon_index < 0) {
            // this is a signalled condition.  we have just started chowing through
            // the destinations to look at them.
            current_recon_index = 0;
        } else {
            // a normal move to the next slot now.
            current_recon_index++;
        }
        if (DEBUGGING)
            log_it("total dests are " + destinations_total
                + " and we're at indy " + current_recon_index);
        if (current_recon_index <= destinations_total - 1) {
            // try jumping to the next place.
            if (DEBUGGING) log_it("next normal test, index " + (string)current_recon_index);
            push_action_record(AQ_PREP_RECON_JAUNTER, added_parms);
        } else {
            // we've gone off the edge of our list in normal mode.
            // now check those that are marked as failures.
            trying_hard_cases = TRUE;
            if (DEBUGGING) log_it("started trying hard cases, back to index 0");
            // start over in processing the list.
            current_recon_index = 0;
            is_recon_pending = FALSE;
            good_destinations_counted = 0;
            ungenerated_good_destinations_counted = 0;
            push_action_record(AQ_CHECK_HARD_CASE, added_parms);
        }
        llMessageLinked(LINK_THIS, DATA_COW_HUFFWARE_ID, TAGGED_GET_ITEM_COMMAND,
            wrap_parameters([OUR_COW_TAG, "#" + (string)current_recon_index]));
        return TRUE;
    }
    if ((current_recon_index < destinations_total) && trying_hard_cases) {
        // if the conditions are very right or very wrong, move to next place.
        if ( (last_verification == VERIFY_SAFE) || (last_verification == VERIFY_UNSAFE_DECIDED) 
            || (current_recon_index < 0) ) {
            current_recon_index++;
        }
        if (DEBUGGING) log_it("next hard test at index " + (string)current_recon_index);
        // make sure we're still in range.
        if (current_recon_index < destinations_total) {
            // we're running the hard cases now, so go to the next one of those.
            list added_parms = [];
            push_action_record(AQ_CHECK_HARD_CASE, added_parms);
            llMessageLinked(LINK_THIS, DATA_COW_HUFFWARE_ID, TAGGED_GET_ITEM_COMMAND,
                wrap_parameters([OUR_COW_TAG, "#" + (string)current_recon_index]));
            return TRUE;
        }
    }
    
    if (!done_reconnoiter) {
        if (DEBUGGING) log_it("decided we're done with recon.");
        // all have been checked.
        done_reconnoiter = TRUE;
        // let the root know how many were tasty healthy places.
        llMessageLinked(LINK_THIS, JAUNT_REZOLATOR_HUFFWARE_ID + REPLY_DISTANCE,
            REZOLATOR_EVENT_RECON_FINISHED,
            (string)ungenerated_good_destinations_counted);
    }
    if (DEBUGGING) log_it("doing nothing else in advance.");
    return FALSE;  // not good.
}

// considers the current state of affairs and schedules the next reasonable thing
// that needs to be done to move the rezzing process along.
take_the_next_appropriate_action()
{
    // see if the child setup process is still running.
    if (child_needs_setup || is_recon_pending || (llGetListLength(action_queue) > 0) ) {
        // we're still waiting on something.  make sure it hasn't timed out.
        if (!check_for_child_timeout()) return;  // we still have time.
        // saying this one failed, so we set it such that we are not waiting for kids
        // or recons or the data cow right now.
        snooze_counter = 0;
        if (child_needs_setup) {
            if (DEBUGGING) log_it("TNAP: child needs setup...");
            child_needs_setup = FALSE;
            if (conveyance_mode == ONE_WAY_TRIP) {
//log_it("TNAP: telling root one way kid is hosed.");
                // the startup of a child jaunter has failed.  let the root think we're
                // doing okay so it can get on with its life.
                llMessageLinked(LINK_THIS, JAUNT_REZOLATOR_HUFFWARE_ID + REPLY_DISTANCE,
                    REZOLATOR_EVENT_REZZED_CHILD, (string)global_rez_place);
                return;
            } else {
//log_it("TNAP: recon fail for kid needs setup, fall through.");
            }
        } else if (is_recon_pending) {
            // the current reconnaissance drone has crashed apparently.  deal with it.
            if (DEBUGGING) log_it("TNAP: setting recon pend to false");
            is_recon_pending = FALSE;
            return;
        } else if (llGetListLength(action_queue) > 0) {
            // action_queue must be non-empty, or we wouldn't be here.
            list action_rec = pop_action_record();  // what else can we do?
            if (DEBUGGING) log_it("TNAP: time-out on action: " + (string)action_rec);
            return;
        }
        // proceed to the next steps which are probably remedial.
    }

    if (DEBUGGING) log_it("TNAP: nothing pending, move things along...");

    // nothing is pending right now.
    if (conveyance_mode == ONE_WAY_TRIP) {
        // this means we are done, since the only thing we do is start the kid up.
        llSetTimerEvent(0.0);
        return;
    }

    // if the recon advancement returns true, then we are all set for the next place.
    if (advance_reconnaissance_position()) return;
    
    // if we got to here, then we need to do something about being past end of list...

    if (done_reconnoiter) {
        // all done with that, so we don't need the timer anymore.
        llSetTimerEvent(0.0);  // stop coming here.
        return;  // don't need to do any more.
    }

    if (trying_hard_cases) {
        try_the_hard_cases();
    }

    if (DEBUGGING) log_it("fell completely through TNAP()");
}

// tries to crack the hard cases using added random jaunt destinations.
try_the_hard_cases()
{
    if (DEBUGGING) log_it("hardcase performing recon re-attempt, name=" + global_destination_name);
    if (is_prefix(global_destination_name, QUADRANT_TAG_NAME)) {
//log_it("seeing that " + global_destination_name + " is a generated dest, ignoring.");
        splice_destination_info(current_recon_index, "#" + (string)current_recon_index,
            vector_chop(final_destination), VERIFY_UNSAFE_DECIDED);
        advance_reconnaissance_position();
        return;
    }
    // count that as a failure, since we're not still on our first run through the list.
    recon_reattempts++;
    if (recon_reattempts > MAXIMUM_RECON_ATTEMPTS) {
        // we have totally failed to find a workable pathway.  we'll store the
        // most simple form of the path back as the one to use.
        log_it("total failure reaching " + (string)final_destination);
        splice_destination_info(current_recon_index, "#" + (string)current_recon_index,
            vector_chop(final_destination), VERIFY_UNSAFE_DECIDED);
        recon_reattempts = 0;
        advance_reconnaissance_position();
        return;
    }
    
    // start the process of finding a good random path.
    if (!load_random_safe_path()) {
        // can't get there from anywhere else, so try from here.
        log_it("could not find a good random path!");
        scramble_to_find_a_way([]);
    }
}

// called to start a jaunter in the appropriate mode.
rez_requested_child(string object_name, integer conv_mode,
    integer chat_channel, string destination_name, integer all_dests_count,
    string target_location)
{
    if (DEBUGGING)
        log_it("rez req: " + object_name + " convey=" + (string)conveyance_mode + " chat=" + (string)chat_channel + " dest=" + destination_name + " fullcount=" + (string)all_dests_count + " targ=" + target_location);

    if (private_id_from_listen) {
        // toss old listening since we're working on a new deal now.
        llListenRemove(private_id_from_listen);
    }
    // listen to chats on our special channel.
    private_id_from_listen = llListen(chat_channel, "", NULL_KEY, "");
    
    // remember the info so we can tell our new child.
    conveyance_mode = conv_mode;
    global_destination_name = destination_name;
    global_object_name = object_name;  // the object to rez from inventory.
    private_chat_channel = chat_channel;
    full_journey = llParseString2List(target_location, [VECTOR_SEPARATOR], []);
    destinations_total = all_dests_count;
    global_verifications = [];  // clear any old verifications.
    integer indy;
    for (indy = 0; indy < destinations_total; indy++)
        global_verifications += [ VERIFY_UNTESTED ];

    if (conveyance_mode == RECONNAISSANCE_TRIP) {
        // we need to start reconnaissance mode.
        current_recon_index = -1;  // a signal that we haven't started.
        snooze_counter = 0;
        is_recon_pending = FALSE;
        done_reconnoiter = FALSE;

//turned off currently.  doesn't seem to help much.
if (FALSE) {
        // experimental: add the quadrants of the sim as destinations.  if some of these
        // are reachable, then they should help us out.
        destinations_real = destinations_total;
        integer x;
        integer y;
        for (x = 32; x <= 255; x += 64) {
            for (y = 32; y <= 255; y += 64) {
                vector calculated = <x, y, 0.0>;
                float height = llGround(calculated - llGetPos());
                calculated.z = height + 64;
                add_destination(QUADRANT_TAG_NAME + (string)x + "," + (string)y, 
                    (string)adjust_to_ground(<x, y, 0>, 64), VERIFY_UNTESTED);
            }
        }
}

        take_the_next_appropriate_action();
    } else {
        // create the jaunter object for a one way trip.
        rez_a_jaunter(object_name, ONE_WAY_TRICKY_PARM + chat_channel, conveyance_mode);
    }
    llSetTimerEvent(TIMER_PERIOD);
}

// handles responses about our list coming back from the data cow.
answer_to_the_cow(string msg, string id, list parms)
{
    string tag = llList2String(parms, 0);
    if (tag != OUR_COW_TAG) return;  // was not for us.
    parms = llDeleteSubList(parms, 0, 0);  // trim out the tag.
    list action_record = pop_action_record();
    integer actyo = extract_action_code(action_record);
    // get the name out of the set first.
    list split = llParseString2List(llList2String(parms, 1), [HUFFWARE_ITEM_SEPARATOR], []);
    
    if (actyo != AQ_ACQUIRE_BASE_PATH) {
        // it's very important that when we're acquiring other targets' path components,
        // we do not mistake that for our own state for the path we're trying to reach.
        // thus we only record the answer for actions that are about our real target.
        global_destination_name = llList2String(parms, 0);
        last_verification = llList2Integer(split, 1);
        full_journey = prechewed_destinations(llList2String(split, 0));
        if (DEBUGGING)
            log_it("just heard verif=" + (string)last_verification + " for " + global_destination_name);
    }

    if (actyo == AQ_ONEWAY_JAUNTER_VOYAGE) {
//log_it("one way, saying on chan " + (string)private_chat_channel + " -- " + global_destination_name + " fullj=" + (string)full_journey);

        child_needs_setup = FALSE;  // count off the one we just set up.

        integer indy;  // i told you N times.
        for (indy = 0; indy < MAX_CHAT_REPEAT; indy++) {
            llSay(private_chat_channel, CHILD_CHAT_TEXT
                + global_destination_name
                + DB_SEPARATOR
                + llDumpList2String(full_journey, VECTOR_SEPARATOR));
            // pausing to let our words sink in.
            llSleep(SLEEPY_TIME);
        }

        // tell the root jaunter that this kid has been issued.
        llMessageLinked(LINK_THIS, JAUNT_REZOLATOR_HUFFWARE_ID + REPLY_DISTANCE,
            REZOLATOR_EVENT_REZZED_CHILD, (string)global_rez_place );
    } else if (actyo == AQ_PREP_RECON_JAUNTER) {
        // we got info about a place to try.  record the last verif state locally.
        if (DEBUGGING) log_it("info re destination: verif = " + last_verification);
        splice_just_verification(current_recon_index, last_verification);
        if ( (last_verification == VERIFY_UNTESTED)
                || (trying_hard_cases && (last_verification == VERIFY_UNSAFE_SO_FAR) ) ) {
            // this one is not tested yet or had problems.  try it now.
//log_it("testing unchecked locale.");
            is_recon_pending = TRUE;
            // if the destination is claiming it's untested, we fix that now untrue statement.
            if (last_verification == VERIFY_UNTESTED) {
                splice_destination_info(current_recon_index, "#" + (string)current_recon_index,
                    llDumpList2String(full_journey, VECTOR_SEPARATOR), VERIFY_UNSAFE_SO_FAR);
            }
            if (DEBUGGING)
                log_it("rezzing jaunter for: name " + global_destination_name + " journ" + (string)full_journey);
            rez_a_jaunter(global_object_name,
                RECONNAISSANCE_TRICKY_PARM + private_chat_channel, RECONNAISSANCE_TRIP);
        } else {
            // this one doesn't need a recon.
            is_recon_pending = FALSE;
//log_it("skipping recon for verif " + (string)last_verification);
            advance_reconnaissance_position();
        }
    } else if (actyo == AQ_CHECK_HARD_CASE) {
        // we got an answer to our request for a new hard locale to try.
        // remember the verification state locally.
        splice_just_verification(current_recon_index, last_verification);

        if ( (last_verification == VERIFY_UNTESTED) || (last_verification == VERIFY_UNSAFE_SO_FAR) ) {
            // this one is not tested yet or had problems.  try it now.
            if (DEBUGGING)
                log_it("testing hard case locale at " + (string)current_recon_index
                    + " name=" + global_destination_name);
            final_destination = (vector)llList2String(full_journey, -1);
            try_the_hard_cases();
        } else {
//log_it("check hard case--already safe or decidedly unsafe, skipping.");
            if (last_verification == VERIFY_SAFE) {
                good_destinations_counted++;
                if (!is_prefix(global_destination_name, QUADRANT_TAG_NAME)) {
                    ungenerated_good_destinations_counted++;
                }
            }
            advance_reconnaissance_position();
        }
        take_the_next_appropriate_action();
    } else if (actyo == AQ_ACQUIRE_BASE_PATH) {
        // a response with a hopefully good destination arrived; base a path on that.
        scramble_to_find_a_way(prechewed_destinations(llList2String(split, 0)));
    }
}

// looks at the location "look_here" and finds the ground height as the z component
// of the returned vector (where x and y are copied from "look_here").  if the
// "height_add_in" is non-zero, it is added to the z component to adjust the ground
// height by an offset.
vector adjust_to_ground(vector look_here, float height_add_in)
{
    float height = llGround(look_here - llGetPos());
    look_here.z = height + height_add_in;
    return look_here;
}

// tries to come up with a random path that can reach a difficult place.
scramble_to_find_a_way(list possible_helpful_path)
{
    // we were told this was a good pathway, so let's try it.
    if (randomize_within_range(0.0, 1.0, FALSE) > 0.5) {
        // if we feel like it, add some huge random vectors in.
        integer indy;
        integer how_many = (integer)randomize_within_range(1, 5, FALSE);
//magic constant above.
        vector rando = llGetPos();  // start with current place.
        for (indy = 0; indy < how_many; indy++) {
            // try to add a random completely other place in the sim.
            vector other_rando = adjust_to_ground(random_vector(0.0, 256.0, FALSE), 64);
            rando.z = other_rando.z;
            // by only changing one axis at a time for x and y, we kind of slide around.
            // this is considered more beneficial than just jumping randomly.
            if (randomize_within_range(0.0, 1.0, FALSE) > 0.5) {
                // this 50% modulates the x.
                rando.x = other_rando.x;
            } else {
                // this other 50% modulates the y.
                rando.y = other_rando.y;
            }
//log_it("scramble: wacky rando: " + vector_chop(rando));
            possible_helpful_path += [ vector_chop(rando) ];
        }
    }
    if (randomize_within_range(0.0, 1.0, FALSE) > 0.2) {
        // maybe we also feel like adding a small amount to the final destination,
        // trying for a hook shot, most of the time.
//log_it("scramble: addition relative to final destination.");
        vector rando = adjust_to_ground(final_destination + random_vector(0.0, 8.0, TRUE), 64);
//magic constant above.
        possible_helpful_path += [ vector_chop(rando) ];
    }
    if ( (final_destination.z < 1000.0) && (randomize_within_range(0.0, 1.0, FALSE) > 0.05) ) {
//hmmm: is this redundant now?
        // we very often add a vertical component so as to swoop down on the destination
        // and hopefully avoid ground fault conditions.
        possible_helpful_path += [ vector_chop(<final_destination.x, final_destination.y,
            final_destination.z + 64>) ];
//magic constant above.
    }
    // now finally we add in the real place where we're going.
    possible_helpful_path += [ vector_chop(final_destination) ];
    // prepare our state for the new attempt.
    is_recon_pending = TRUE;
    snooze_counter = 0;
    full_journey = possible_helpful_path;
//log_it("scramble: final guess=" + (string)full_journey);
    // rez for this nutty test path.
    rez_a_jaunter(global_object_name,
        RECONNAISSANCE_TRICKY_PARM + private_chat_channel, RECONNAISSANCE_TRIP);
}

// this function returns TRUE if we are close enough to the "destination".
integer close_enough(vector destination)
{
    float PROXIMITY_REQUIRED = 0.1;
        // how close we must be to the target location to call it done.
        // matches current jaunting library proximity.
    return (llVecDist(llGetPos(), destination) <= PROXIMITY_REQUIRED);
}

// records a verification state locally so we can make decisions during testing.
splice_just_verification(integer index, integer verif_state)
{
    if ( (index < 0) || (index >= destinations_total) ) return;  // can't do it.
    global_verifications = chop_list(global_verifications, 0, index - 1)
        + [ verif_state ]
        + chop_list(global_verifications, index + 1, destinations_total - 1);
}

// replaces an existing entry at the "index" if known (otherwise this should be negative)
// for the location named "dest_name" if known (otherwise should be a restatement of the
// numerical index with '#' in front).
splice_destination_info(integer index, string dest_name, string pathway, integer verif)
{
//log_it("splicedest: " + (string)index + " name=" + dest_name + " verif=" + (string)verif + " path=" + pathway);
    splice_just_verification(index, verif);
    string new_entry = wrap_item_list([pathway, verif]);
    llMessageLinked(LINK_THIS, DATA_COW_HUFFWARE_ID, ADD_ITEM_COMMAND,
        wrap_parameters([dest_name, new_entry]));
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
integer check_for_child_timeout()
{
    if (snooze_counter++ > MAXIMUM_SNOOZES) {
        // go back to the main state.  we took too long.
//        log_it("timed out!");
        llUnSit(llAvatarOnSitTarget());  // don't hang onto the avatar for this error.
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
    list top_action = llParseString2List(llList2String(action_queue, 0), [HUFFWARE_PARM_SEPARATOR], []);
    action_queue = llDeleteSubList(action_queue, 0, 0);
    return top_action;
}

// adds a new action to the end of the action queue.
push_action_record(integer action, list added_parms)
{
    snooze_counter = 0;  // reset back for new action.
    action_queue += [ wrap_parameters([action] + added_parms) ];
}

//////////////

// processes link messages received from support libraries.
handle_link_message(integer which, integer num, string msg, string id)
{
    // maybe it's a piece of data from the data cow.
    if (num == DATA_COW_HUFFWARE_ID + REPLY_DISTANCE) {
        // unpack the parameters we were given.
        list parms_x = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
        answer_to_the_cow(msg, id, parms_x);
        return;
    }
    
    if (num != JAUNT_REZOLATOR_HUFFWARE_ID) return;  // not interesting.
    if (DEBUGGING) log_it("linkmsg: msg " + msg + " id " + id);
    list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
    if (msg == REZ_CHILD_NOW) {
        serving_root = TRUE;
        snooze_counter = 0;  // reset counter.
        if (llGetListLength(parms) < REZ_KID_FIELDS_NEEDED) {
//                log_it("insufficient parameters for rezzing child.");
            return;
        }
        rez_requested_child(llList2String(parms, 0), llList2Integer(parms, 1),
            llList2Integer(parms, 2), llList2String(parms, 3),
            llList2Integer(parms, 4), llList2String(parms, 5));
    } else if (msg == REPORT_CHILD_REZZED) {
        report_to_root();
    } else if (msg == RESET_REZOLATOR) {
        llResetScript();
    } else if (msg == REZOLATOR_CHILD_SUPPORT) {
        provide_child_support(llList2Integer(parms, 0));
    } else if (msg == REZOLATOR_CHILD_RETURNED) {
        if (conveyance_mode == RECONNAISSANCE_TRIP) {
            // unpack the success value from jaunter's check.
            integer succeeded_recon = llList2Integer(parms, 0);
            // report on our success.
            list report_loc = full_journey;
            if (!succeeded_recon) {
                // we did at least get to somewhere, even if not to target.
                report_loc = llParseString2List(llList2String(parms, 1),
                    [VECTOR_SEPARATOR], []);
            }
            llWhisper(private_chat_channel, CHILD_CHAT_TEXT + RETURN_WORD
                + (string)succeeded_recon + DB_SEPARATOR
                + global_destination_name
                + DB_SEPARATOR
                + llDumpList2String(report_loc, VECTOR_SEPARATOR));
//log_it("child reporting success=" + (string)succeeded_recon + " for " + global_destination_name + " via " + llDumpList2String(report_loc, VECTOR_SEPARATOR) );
            // shuffle off...
//log_it("now dying at posn " + (string)llGetPos());
            llDie();  // oh crud, we're toast.
        } else {
            // one way trip??
            if (!succeeded_recon) log_it("too far away; path okay?");
        }
    }
}

// makes our click action change to the type requested, and sends the request
// out to all our sub-prims also.
set_click_action(integer action)
{
    llSetClickAction(action);
    // secret message to other prims to change click action.
    llMessageLinked(LINK_SET, CHANGE_CLICK_ACTION_HUFFWARE_ID, "", (string)action);
}

// tries to find a known safe locale that's close to the target.  if there are any
// that are close, then we will not add this location.  otherwise, we do add it,
// if the list is not already too large.
add_if_no_close_match(string pathway)
{
    // make sure there's enough room to bother checking.
    if (llGetListLength(safe_locales_seen) >= MAXIMUM_SAFE_LOCATIONS) return;
    list dests = llParseString2List(pathway, [VECTOR_SEPARATOR], []);
    vector final_locn = (vector)llList2String(dests, -1);
    // set the z component to zero for this comparison, which is only about x,y position.
    // since we then add this trimmed vector, we don't need to worry about the z component
    // for the items already in the list.
    final_locn.z = 0;
    integer i;
    for (i = 0; i < llGetListLength(safe_locales_seen); i++) {
        vector curr = (vector)llList2String(safe_locales_seen, i);
        if (llVecDist(curr, final_locn) < MINIMUM_DISTANCE_FOR_EXTRAS) {
//log_it("dest too close: dist = " + (string)llVecDist(curr, final_locn));
            return;
        }
    }
    if (DEBUGGING) log_it("dest far enough to add: " + pathway);
    // no known locale is close, so we'll add this one.
    safe_locales_seen += [ final_locn ];
    good_destinations_counted++;
    add_destination(QUADRANT_TAG_NAME + "pt#" + (string)(partial_path_count++),
        pathway, VERIFY_SAFE);
}

// process what we hear in open chat and on our special channels.
// this function should always return FALSE unless it's been given enough info
// to enter a new state, in which case it should return true.
listen_to_voices(integer channel, string name, key id, string message)
{
    // check to see if the text is right.
    if (DEBUGGING)
        log_it("heard: " + message + " from " + name);
    if (!is_prefix(message, CHILD_CHAT_TEXT)) return;  // not right.
    message = llDeleteSubString(message, 0, llStringLength(CHILD_CHAT_TEXT) - 1);
    // maybe this is a startup request.
    if (is_prefix(message, READY_TEXT) && serving_root) {
        // yes, this is a request for a destination.  we are going to fill the rezzed object
        // now, so figure out what type of jaunter needs info.  formerly fill_rezzed_object.
        integer is_recon = (integer)llGetSubString(message, -1, -1);
        if (done_reconnoiter && is_recon) return;  // we already did all that.
        if (is_recon) {
            // a recon jaunter needs its instructions.
            if (DEBUGGING)
                log_it("telling recon kid: name " + global_destination_name + " journ=" + (string)full_journey);
            is_recon_pending = TRUE;
            integer indy;  // i told you N times.
            for (indy = 0; indy < MAX_CHAT_REPEAT; indy++) {
                llSay(private_chat_channel, CHILD_CHAT_TEXT
                    + global_destination_name
                    + DB_SEPARATOR
                    + llDumpList2String(full_journey, VECTOR_SEPARATOR));
                // snooze to make sure we pause a little between mouthings.
                llSleep(SLEEPY_TIME);
            }

            snooze_counter = 0;  // starting over.
            child_needs_setup = FALSE;  // count off the one we just set up.
        } else {
            if (DEBUGGING)
                log_it("into code for one way jaunter");
            // simple one way jaunter needs fillin'.
            list added_parms = [];
            push_action_record(AQ_ONEWAY_JAUNTER_VOYAGE, added_parms);
            llMessageLinked(LINK_THIS, DATA_COW_HUFFWARE_ID, TAGGED_GET_ITEM_COMMAND,
                wrap_parameters([OUR_COW_TAG, global_destination_name]));
        }
        return;
    }

    if (DEBUGGING) log_it("into kid side of rezolator...");

    if (!serving_root) {
        if ( (conveyance_mode == RECONNAISSANCE_TRIP) || (conveyance_mode == ONE_WAY_TRIP) ) {
            if (heard_from_root) return;  // we already did this bit.
            // this jaunter is destined for short-lived work, but now at least has a goal.
            // we just pass along the parameters that were told to us from the root jaunter.
            list parms = llParseString2List(message, [DB_SEPARATOR], []);
            global_destination_name = llList2String(parms, 0);
            full_journey = llParseString2List(llList2String(parms, 1), [VECTOR_SEPARATOR], []);
            final_destination = (vector)llList2String(full_journey, -1);
            parms += [ conveyance_mode ];
    
            if (conveyance_mode == ONE_WAY_TRIP) {
                // set the click action properly.
                if (outside_of_sim(final_destination)) set_click_action(CLICK_ACTION_TOUCH);
                else set_click_action(CLICK_ACTION_SIT);
            }
    
            if (DEBUGGING) log_it("kid heard go to: " + global_destination_name + " via " + (string)full_journey);
            llMessageLinked(LINK_THIS, JAUNT_REZOLATOR_HUFFWARE_ID + REPLY_DISTANCE,
                REZOLATOR_EVENT_GOT_INSTRUCTIONS, wrap_parameters(parms));
            // make sure we cancel the listening so we don't hear directions that aren't for us.
            llListenRemove(private_id_from_listen);
            heard_from_root = TRUE;
            return;  // only leave for recon.
        }
    } else {
        // this jaunter's role should be a root jaunter, and we have a reply about a recon mission.
        is_recon_pending = FALSE;  // we got an acceptable answer.
        list parms = llParseString2List(llDeleteSubString
            (message, 0, llStringLength(RETURN_WORD) - 1), [DB_SEPARATOR], []);
        integer success = llList2Integer(parms, 0);
        string dest_name = llList2String(parms, 1);
        string pathway = llList2String(parms, 2);
//log_it("|||| recon reply: " + dest_name + " good=" + (string)success + " path=" + pathway);
        if (success) {
            // record this as a safe pathway.
            last_verification = VERIFY_SAFE;
            good_destinations_counted++;
        } else {
            // a failure is being reported.  record this as a bad pathway.
            last_verification = VERIFY_UNSAFE_SO_FAR;
            // however, the jaunter should have told us where it did get to.  record that whole
            // thing if it seems like a good distance from our other known good places.
            if (DEBUGGING) log_it("failed path did get to: " + pathway);
            add_if_no_close_match(pathway);

        }
        // only save this info if we're not working on recons.
        if (!trying_hard_cases || success) {
            // fix name for tough destination's real one.
            if (trying_hard_cases) {
                dest_name = "#" + (string)current_recon_index;
            }
            splice_destination_info(current_recon_index, dest_name,
                llDumpList2String(full_journey,
                    ///llList2List(full_journey, 1, -1), 
                VECTOR_SEPARATOR),
                last_verification);
        }
        take_the_next_appropriate_action();
    }
}

// processes a destination set to handle special cases, like for offset jaunting.
list prechewed_destinations(string dests)
{

//hmmm: document this!  as in, we support the offset format.

    // look for our special start character for offsets.
    if (is_prefix(dests, "o"))
        // if this is an offset version, then chop whatever word they used starting with 'o'
        // and compute the destination based on current position.
        return [ (vector)llDeleteSubString(dests, 0, find_substring(dests, "<") - 1) + llGetPos() ];
    else
        // normal jaunt to absolute coordinates.
        return llParseString2List(dests, [VECTOR_SEPARATOR], []);
}

// creates a jaunter object given which inventory item and the startup parm.
rez_a_jaunter(string object_name, integer rez_parm, integer conveyance_mode)
{
    // the minimum and maximum distances for placing the auto-rezzed jaunter object.
    integer MIN_REZ_DISTANCE = 1;
    integer MAX_REZ_DISTANCE = 3;
    vector RECON_ROTATION = <270.0, 0.0, 0.0>;
        // what angle we use when planting the recon jaunter.
        // this is tightly coupled to our particular object.
    vector point_at_jaunter;
    if (conveyance_mode == RECONNAISSANCE_TRIP) {
        // we just use our specified rotation.
        point_at_jaunter = RECON_ROTATION * DEG_TO_RAD;
        // start at where the jaunter will be when it tries the same path.
        global_rez_place  = llGetPos();
    } else {
        // we randomize the location to avoid people having the objects
        // in a big pile on top of each other.
        global_rez_place  = llGetPos()
            + <randomize_within_range(MIN_REZ_DISTANCE, MAX_REZ_DISTANCE, TRUE),
                randomize_within_range(MIN_REZ_DISTANCE, MAX_REZ_DISTANCE, TRUE),
                0.4>;  // pop it up a bit.
        // aim the temporary ride at the root jaunter.
        vector aim_back = llGetPos() - global_rez_place;
        float z_rot = llAtan2(aim_back.y, aim_back.x);
        // calculate the vector for rotation given the z rotation.
        point_at_jaunter = <0.0, 0.0, z_rot>;
    }    
    llRezObject(object_name, global_rez_place,
        ZERO_VECTOR, llEuler2Rot(point_at_jaunter), rez_parm);
    if (conveyance_mode != RECONNAISSANCE_TRIP) {
        // this model only jaunts to where it gave out the object.  that
        // should hopefully always work.  but at least if it can't get back,
        // it's really close by.
        full_journey = [ vector_chop(llGetPos()), vector_chop(global_rez_place) ];
    }
    child_needs_setup = TRUE;  // we aren't ready to go even if jaunt is done.
    snooze_counter = 0;  // amnesty on the timer.
}

// finds a pathway that's already in our list and that we think is safe.
integer load_random_safe_path()
{
    integer indy;
    integer goodunz = good_destinations_counted;

    // now that we know how many are safe, we can pick a good destination.
    integer randola = llRound(randomize_within_range(0, goodunz - 1, FALSE));
    integer safe_indy = -1;  // tracks which safe item we're at.
    for (indy = 0; indy < llGetListLength(global_verifications); indy++) {
        // scoot across the list to find the Nth safe one.
        if (llList2Integer(global_verifications, indy) == VERIFY_SAFE) {
            if (safe_indy == randola) {
//log_it("like path at " + (string)safe_indy);
                // we're at the Nth safe item, so this is our random choice.
                list added_parms = [];
                push_action_record(AQ_ACQUIRE_BASE_PATH, added_parms);
                llMessageLinked(LINK_THIS, DATA_COW_HUFFWARE_ID, TAGGED_GET_ITEM_COMMAND,
                    wrap_parameters([OUR_COW_TAG, "#" + (string)safe_indy]));
                return TRUE;
            }
            // increment to the next position, since we've seen this one.
            safe_indy++;
        }
    }
//log_it("found no good path to use");
    return FALSE;
}

// processes the timer events during an ongoing rezzing process.
handle_timer()
{
    // this is the root's timer process, which is implemented in great detail
    // in the function below.
    if (serving_root) take_the_next_appropriate_action();
}

// file a report with the root jaunter that the child has been created and is
// ready for service.
report_to_root()
{
    // tell the root jaunter know that we're ready to be packed for our trip.
    llSay(private_chat_channel, CHILD_CHAT_TEXT + READY_TEXT
        + (string)(conveyance_mode == RECONNAISSANCE_TRIP));
}

// plops a new destination on the end of the lists.
add_destination(string name, string path, integer verif)
{
//log_it("adding " + name + " with path " + path + " and verif=" + (string)verif);
    global_verifications += [ verif ];
    // send our new information to the data cow.  we store the information encoded as
    // two separated items, where the first element is the destination and the second
    // is the verification state.
    string new_entry = wrap_item_list([path, verif]);
    llMessageLinked(LINK_THIS, DATA_COW_HUFFWARE_ID, ADD_ITEM_COMMAND,
        wrap_parameters([name, new_entry]));
    destinations_total++;  // we just added another destination.
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
    // say this on open chat, but use an unusual channel.
//    llSay(108, (string)debug_num + "- " + to_say);
}

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

// returns a random vector where x,y,z will be between "minimums" and "maximums"
// x,y,z components.  if "allow_negative" is true, then any component will
// randomly be negative or positive.
vector random_bound_vector(vector minimums, vector maximums, integer allow_negative)
{
    return <randomize_within_range(minimums.x, maximums.x, allow_negative),
        randomize_within_range(minimums.y, maximums.y, allow_negative),
        randomize_within_range(minimums.z, maximums.z, allow_negative)>;
}

// returns a vector whose components are between minimum and maximum.
// if allow_negative is true, then they can be either positive or negative.
vector random_vector(float minimum, float maximum, integer allow_negative)
{
    return random_bound_vector(<minimum, minimum, minimum>,
        <maximum, maximum, maximum>, allow_negative);
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
    state_entry() { auto_retire(); initialize_rezolator(); }

    on_rez(integer startup) { llResetScript(); }

    state_exit() { llSetTimerEvent(0.0); }

    link_message(integer which, integer num, string msg, key id)
    { handle_link_message(which, num, msg, id); }

    listen(integer channel, string name, key id, string message)
    { listen_to_voices(channel, name, id, message); }

    timer() { handle_timer(); }
}

