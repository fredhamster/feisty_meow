﻿
// huffware script: rezzeroni, by fred huffhines
//
// a script that rezzes objects for you according to some game plan, which is defined
// in a notecard.  this script can also run periodically to create temporary objects
// on a schedule.  the real power of this script comes from its ability to rez up a
// collection of objects using relative positioning.  the newly rezzed objects can be
// given a startup notecard also, which is held inside the rezzer.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

// * note regarding the periodic rezzing capability: the objects that are created
//   really need to be marked as temporary objects.  otherwise the newly rezzed
//   objects will clutter up the sim, flooding it with objects, and will eventually
//   go over the owner's prim limit.  that's not cool.  so if you use a periodic
//   rezzer, make sure your contained objects are temporaries.

// global constants...

integer DEBUGGING = FALSE;  // produces noisy diagnostics if enabled.

integer SUPER_NOISY_DEBUG = FALSE;  // extraordinarily noisy items are logged.

float HOME_PROXIMITY = 0.1;  // object must be this close to home before we quit.

integer PERFORM_JAUNTS = TRUE;  // set to true if rezzer should teleport to rez locations.

integer OBJECT_STARTUP_PARM = 14042;
//hmmm: may want this to come from notecard someday.

// notecard configurable variables:
integer owner_only;  // is the owner the only one allowed to run this rezzer?
vector obj_position;  // absolute position for object to rez at.
integer obj_is_singleton;  // should the object not be rezzed if similar named one exists?
//hmmm: support configurable singleton sensor range?
integer rezzed_at_once;  // the number of each object that are rezzed at the same time.
float obj_rezzing_period;  // how often the objects are rezzed up, in seconds.
integer max_run_time;  // how long to run once enabled, in seconds.  1200=20 minutes.
string gift_card_name;  // a gift object placed inside newly rezzed object.
vector obj_max_offset_add = <2.0, 2.0, 2.0>;  // random offsets can be this large.
vector obj_max_rotation_add = <TWO_PI, TWO_PI, TWO_PI>;  // max random rotation.
// these object variables can be randomized.  "random" means pick a random item.
string obj_name;  // what is the object called?  "ALL" means all items.
string obj_offset;  // how far away from current position object rezzes.
string obj_rotation;  // euler rotation for object to point at upon rez.
string obj_multi_offset;  // the amount to add to the offset each time for count > 1.
string obj_multi_angle;  // the angle added to each rez.
//hmmm: also allow position to be random?
integer DIE_ON_DEMAND_CHANNEL = 4826;  // default, can be loaded from notecard.

// assorted global variables...
integer is_enabled_now;  // records whether the rezzer is running a plan right now.
integer when_last_enabled;  // tracks time when device was turned on.
// notecard globals.
string global_notecard_name;  // name of our notecard in the object's inventory.
integer response_code;  // set to uniquely identify the notecard read in progress.
list global_config_list;  // a collection of configuration parameters from our notecard.
// globals tracking the current rezzing run.
list already_done_items;  // the items that are finished being rezzed, since they have no rate.
integer rezzed_overall;  // a count of the number of rezzes during this run.
list gift_stack;  // a stack of items to hand to newly rezzed objects.
// somewhat ugly rez plan globals.
integer global_config_index;  // where are we in the config list?
string global_singleton_checker;  // is a singleton object being sensor checked?
integer global_found_the_singleton;  // did we see the singleton object requested?
integer sensor_checks_pending;  // how many sensor scans are awaited (zero or one).
integer recent_items_rezzed;  // how many got rezzed in last run?
// object creation loop vars:
integer current_object_index = 0;  // which object are we at, in the current section?
integer current_count;  // the number already created of the current object.

vector main_home;
    // the home location for the rezzer object.  it will return to this after
    // performing the requested rezzes.
rotation main_rotate;
    // original rotation of the rezzer object.

integer jaunt_responses_awaited;
    // the number of jumps still pending.  should never be more than one in this script.

// constants that should generally not be modified unless you are an expert...

string REZZERONI_SIGNATURE = "#rezzeroni";  // the expected first line of our notecards.

float MAXIMUM_REZ_DISTANCE = 9.9;
  // the farthest distance that we can rez an object at reliably.  if the distance is
  // larger than this, then the rezzer will teleport to the work zone.

float BASE_TIMER_PERIOD = 0.04;  // the rate at which the timer runs to rez objects.

// requires noteworthy library v9.3 or better.
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
string NOTECARD_READ_CONTINUATION = "continue!";
    // returned as first parameter if there is still more data to handle.
// commands available via the noteworthy library:
string READ_NOTECARD_COMMAND = "#read_note#";
    // command used to tell the script to read notecards.  needs a signature to find
    // in the card as the first parameter, and a randomly generated response code for
    // the second parameter.  the response code is used to uniquely identify a set of
    // pending notecard readings (hopefully).  the signature can be blank.
    // the results will be fired back as the string value returned, which will have
    // as first element the notecard's name (or BAD_NOTECARD_INDICATOR if none was
    // found) and as subsequent elements an embedded list that was read from the
    // notecard.  this necessarily limits the size of the notecards that we can read
    // and return.
string READ_SPECIFIC_NOTECARD_COMMAND = "#read_thisun#";
    // like the read notecard command, but specifies the notecard name to use.  only that
    // specific notecard file will be consulted.  first and second parm are still signature
    // and response code, third parm is the notecard name.
//
//////////////
// joins a list of parameters using the parameter sentinel for the library.
string wrap_parameters(list to_flatten)
{ return llDumpList2String(to_flatten, HUFFWARE_PARM_SEPARATOR); }
//////////////

// looks for a notecard with our signature in it and reads the configuration.
// an empty string is returned if we couldn't find anything.
request_configuration()
{
    global_notecard_name = "";
    response_code = 0;
    
    // try to find a notecard with our configuration.
    response_code = -1 * (integer)randomize_within_range(23, 80000, FALSE);
    string parms_sent = wrap_parameters([REZZERONI_SIGNATURE, response_code]);
    llMessageLinked(LINK_THIS, NOTEWORTHY_HUFFWARE_ID, READ_NOTECARD_COMMAND,
         parms_sent);
}

// processes link messages received from support libraries.
integer handle_link_message(integer which, integer num, string msg, key id)
{
    list parms;
    if (num == JAUNT_HUFFWARE_ID + REPLY_DISTANCE) {
        // did we get a response to a teleport request?
        if (msg == JAUNT_LIST_COMMAND) {
//log_it("msg: " + (string)num + " str=" + str);
            jaunt_responses_awaited--;  // one less response being awaited.
            if (jaunt_responses_awaited < 0) {
                log_it("error, jaunts awaited < 0!");
                jaunt_responses_awaited = 0;
            }
            // unpack the reply.
            parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
            integer last_jaunt_was_success = llList2Integer(parms, 0);
            vector posn = (vector)llList2String(parms, 1);
    //log_it("got a reply for a jaunt request, success=" + (string)last_jaunt_was_success + " posn=" + (string)posn);
    //do anything with the parms?
        }
        return FALSE;
    }
    if ( (num != NOTEWORTHY_HUFFWARE_ID + REPLY_DISTANCE)
        || (msg != READ_NOTECARD_COMMAND) ) return FALSE;  // not for us.

    // process the result of reading the notecard.
    parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
    string notecard_name = llList2String(parms, 0);
    integer response_for = llList2Integer(parms, 1);
    if (response_for != response_code) return FALSE;  // oops, this isn't for us.
    // make sure if we are being told to keep going.
    if (notecard_name == NOTECARD_READ_CONTINUATION) {
//log_it("continuation of notecard seen.");
        // snag all but the first two elements for our config now.
        global_config_list += llList2List(parms, 2, -1);
        // we're not done reading yet, so fall out to the false return.
    } else if (notecard_name != "bad_notecard") {
        // a valid notecard has been found and we're done with it now.
        global_notecard_name = notecard_name;
        // snag all but the first two elements for our config now.
        global_config_list += llList2List(parms, 2, -1);
        global_config_index = 0;
        // signal that we're done getting the config.
        return TRUE;
    } else {
        // we hated the notecards we found, or there were none.
        log_it("sorry, no notecards starting with '"
            + REZZERONI_SIGNATURE
            + "'; cannot rez yet.");
    }
    return FALSE;
}

///////////////

// requires jaunting library v10.5 or greater.
//////////////
// do not redefine these constants.
integer JAUNT_HUFFWARE_ID = 10008;
    // the unique id within the huffware system for the jaunt script to
    // accept commands on.  this is used in llMessageLinked as the num parameter.
// commands available via the jaunting library:
string JAUNT_COMMAND = "#jaunt#";
    // command used to tell jaunt script to move object.  pass a vector with the location.
string JAUNT_LIST_COMMAND = "#jauntlist#";
    // like regular jaunt, but expects a list of vectors as the first parameter; this list
    // should be in the jaunter notecard format (separated by pipe characters).
    // the second parameter, if any, should be 1 for forwards traversal and 0 for backwards.
//
//////////////
// encases a list of vectors in the expected character for the jaunting library.
string wrap_vector_list(list to_wrap)
{ return llDumpList2String(to_wrap, HUFFWARE_ITEM_SEPARATOR); }
//////////////

// asks the jaunting library to take us to the target using a list of waypoints.
request_jaunt(list full_journey, integer forwards)
{
    if (PERFORM_JAUNTS) {
        jaunt_responses_awaited++;
        llMessageLinked(LINK_THIS, JAUNT_HUFFWARE_ID, JAUNT_LIST_COMMAND,
            wrap_vector_list(full_journey)
            + HUFFWARE_PARM_SEPARATOR + (string)forwards);
    }
}

// jaunts back to our home location.
attempt_to_go_home()
{
    // jump back to home.
    request_jaunt([llGetPos(), main_home], TRUE);
}

///////////////

// creates the real list of items to rez based on the obj_name variable.
// this might turn out to be an empty list.
list build_rezzing_list()
{
    list objects_to_rez;  // what items are we expected to create in this round?
    
    integer num_inv = llGetInventoryNumber(INVENTORY_OBJECT);
    if (num_inv == 0) {
        log_it("Inventory is empty; please add objects to rez.");
        llSetTimerEvent(0.0);  // stop the timer; nothing further to do.
        return [];
    }
//hmmm: all has never been tested.
    if (obj_name == "ALL") {
        integer inv;
        for (inv = 0; inv < num_inv; inv++) {
            objects_to_rez += [ llGetInventoryName(INVENTORY_OBJECT, inv) ];
        }
        obj_is_singleton = FALSE;  // flag makes no sense for multiple objects.
    } else {
        // we know we are not being told to do a multiple set of objects.
        if (obj_name == "random") {
            // special keyword for a random object.
            integer inv = (integer)randomize_within_range(0.0, num_inv, FALSE);
            obj_name = llGetInventoryName(INVENTORY_OBJECT, inv);
            if (SUPER_NOISY_DEBUG)
                log_it("chose item #" + (string)inv + " randomly (called '" + obj_name + "').");
        } else {
            // a normal singular object to create.
            if (find_in_inventory(obj_name, INVENTORY_OBJECT) < 0) {
                // this item is bogus!
                log_it("item '" + obj_name + "' is bogus!  not in inventory.");
                obj_name = "";  // reset it.
            }
        }
        // now make sure we didn't already do this one.
        if (find_in_list(already_done_items, obj_name) >= 0) {
//            if (SUPER_NOISY_DEBUG)
//                log_it("item '" + obj_name + "' has already been rezzed.  skipping it.");
            obj_name = "";
        } else {
            if (obj_name != "") objects_to_rez += [ obj_name ];
        }
    }
    
    return objects_to_rez;
}

// checks whether the object is even supposed to be a singleton or not.
// if it should be a singleton, then we run a sensor ping to see if it exists in the
// vicinity or not.  if it's not there, we'll eventually hear about it and create it.
integer validate_singleton_object()
{
    // see if there's only supposed to be one of these objects.
    if (!obj_is_singleton) return TRUE;  // just keep going; it's not one.
    if (obj_name == "") return TRUE;  // this should never happen.  pretend it's okay.

    // okay, we know we should have a singleton.  now did we already check for its presence?    
    if (global_singleton_checker == "") {
        // have not scanned yet; postpone the rezzing action and start a sensor for the object.
        sensor_checks_pending++;
        global_singleton_checker = obj_name;
        // maximally distant ping in full arc on all types of objects we can see.
        llSensor(obj_name, NULL_KEY, AGENT|ACTIVE|PASSIVE, 96, PI);
        if (DEBUGGING) log_it("scanning for singleton item '" + obj_name + "' now.");
        return FALSE;
    } else {
        // we should have finished a sensor probe now, since there were no checks pending
        // and since this is a singleton.
        if (global_found_the_singleton) {
            if (DEBUGGING)
                log_it("singleton item '" + obj_name + "' already present; not re-rezzing.");
            already_done_items += [ obj_name ];
//if (TERMINATE_IF_PRESENT)
string DIE_ON_DEMAND_MESSAGE = "die-on-demand";
llShout(DIE_ON_DEMAND_CHANNEL, DIE_ON_DEMAND_MESSAGE);
        } else {
            if (DEBUGGING)
                log_it("singleton item '" + obj_name + "' is absent; will rez one now if possible.");
        }
        return TRUE;
    }
}

// operates on the current parameters to rez an object or objects.
integer perform_rezzing_action()
{
    vector current_home = llGetPos();
    list objects_to_rez = build_rezzing_list();
//log_it("rezzing " + (string)llGetListLength(objects_to_rez) + " objects");
    // create the specified number of the items we were told to use.
    integer num_objects = llGetListLength(objects_to_rez);
    if (num_objects == 0) {
        // nothing to do at this point.
//        if (SUPER_NOISY_DEBUG) log_it("no items in rez list; bailing out.");
        return TRUE;
    }
    // iterate across each object in inventory.
    integer inv;
    for (inv= current_object_index; inv < num_objects; inv++) {
//log_it("object index " + (string)inv);
        // rez as many of the object as we were requested to.
        string new_rezzee = llList2String(objects_to_rez, inv);
        integer counter;
        for (counter = current_count; counter < rezzed_at_once; counter++) {
            // make sure we've never rezzed a non-periodic with this name before.
            if (find_in_list(already_done_items, new_rezzee) < 0) {
///log_it("haven't rezzed '" + new_rezzee + "' yet.");
                vector rez_place = obj_position;
//log_it("configd rezplace=" + (string)rez_place);
                if (rez_place == <0.0, 0.0, 0.0>) {
                   // they did not give us an absolute position, so use the home position.
                   rez_place = main_home;
//log_it("rehomed rezplace=" + (string)rez_place);
                }
                vector calc_offset = maybe_randomize_offset(obj_offset, obj_max_offset_add);
                if (calc_offset != <0.0, 0.0, 0.0>) {
                    // the offset is non-zero, so apply it to the rez place.
                    rez_place += calc_offset * main_rotate;
//log_it("offsetd rezplace=" + (string)rez_place);
                }
                // add in the multiplier offset if this is not the first rezzing of this item.
                rez_place += ((float)counter)
                    * maybe_randomize_offset(obj_multi_offset, obj_max_offset_add)
                    * main_rotate;
//log_it("home=" + (string)main_home + " rezplace=" + (string)rez_place);
    
                // check that we are not too far away.
                float rez_distance = llVecDist(rez_place, llGetPos());
                if (rez_distance > MAXIMUM_REZ_DISTANCE) {
                    // try to jump there if we're too far away to rez from here.
                    if (DEBUGGING)
                        log_it("distance is " + (string)rez_distance
                            + "; jaunting closer.");
                    request_jaunt([llGetPos(), rez_place], TRUE);
                    current_count = counter;
                    current_object_index = inv;
                    return FALSE;
                }

                // make sure we aren't already done with this object, if it's a singleton.
                integer keep_going = validate_singleton_object();
                if (!keep_going) {
                    if (SUPER_NOISY_DEBUG) log_it("must sense for singleton, bailing.");
                    current_count = counter;
                    current_object_index = inv;
                    return FALSE;  // we can't proceed yet.  must check for singleton.
                }

//hmmm: we need to offer an angular offset also!
//  this would mean that the multiple guys could be set at nice different angles.

                // now check again whether to rez this or not; the validation method adds it to the
                // done list if the object is already present and is a singleton.
                if (find_in_list(already_done_items, new_rezzee) < 0) {
                    if (DEBUGGING)
                        log_it("now rezzing '" + new_rezzee + "', rot " + obj_rotation);
                    vector rota = maybe_randomize_rotation(obj_rotation, obj_max_rotation_add);
                    rotation real_rot = llEuler2Rot(rota * DEG_TO_RAD);

//hmmm: might be nice to offer an absolute version!
// that accounts for the specific position and offset, rather
// than relative to rezzer.  angle should be kept absolutist also.

                    // add in our rotation component as the rezzer.
                    real_rot *= main_rotate;
//this is bunkum.
                    // add in the angular addition for multiple object creation.
                    real_rot *= llEuler2Rot(
                        maybe_randomize_rotation(obj_multi_angle, obj_max_rotation_add)
                        * (float)counter * DEG_TO_RAD);
                    recent_items_rezzed++;  // we got to do something.
                    // rez it up since we know what is wanted and where, plus we're in range.
                    llRezObject(new_rezzee, rez_place, ZERO_VECTOR, real_rot, OBJECT_STARTUP_PARM);
                    rezzed_overall++;  // one more item.
                    if (gift_card_name != "") {
                        // this guy needs its startup gift.
                        string inv_name = gift_card_name + (string)rezzed_overall;
                        if (find_in_inventory(inv_name, INVENTORY_NOTECARD) >= 0) {
                            // this inventory item exists, so use it.
                            gift_stack += [ inv_name];
                        }
                    }
                    global_singleton_checker = "";  // reset singleton checker for next object.
                } else {
                    if (SUPER_NOISY_DEBUG) log_it("not rezzing, already saw it in list");
                }
            }
        }
        current_count = 0;  // got to the full count for the object.
        // if this is a non-periodic object, we don't need any more runs.
        if (obj_rezzing_period == 0.0) {
            if (find_in_list(already_done_items, new_rezzee) < 0) {
                if (SUPER_NOISY_DEBUG) log_it("adding newly done: " + (string)new_rezzee);
                already_done_items += [ new_rezzee ];
            }
        }
    }
    current_object_index = 0;  // reset the current object index.

    return TRUE;
}

///////////////

// returns true if the state should change.
integer handle_timer()
{
    // make sure we run at the fast rate; we'll reset the rate elsewhere if needed.
    llSetTimerEvent(BASE_TIMER_PERIOD);

    if (!is_enabled_now) {
        if (jaunt_responses_awaited > 0) {
            if (SUPER_NOISY_DEBUG) log_it("done rez, but jaunts awaited.");
            return FALSE;
        }
        if (llVecDist(llGetPos(), main_home) > HOME_PROXIMITY) {
            if (SUPER_NOISY_DEBUG) log_it("done rez, but not at home.");
            attempt_to_go_home();
            return FALSE;
        }
        if (llGetListLength(gift_stack)) {
            if (SUPER_NOISY_DEBUG) log_it("done rez, but still pending gifts.");
            return FALSE;
        }
        if (SUPER_NOISY_DEBUG) log_it("done rez, totally done.");
        llSetTimerEvent(0.0);
        return TRUE;
    }

    // check whether the max run time has elapsed.    
    integer run_time = llAbs(llGetUnixTime() - when_last_enabled);
    if (run_time > max_run_time) {
        log_it("Maximum run time elapsed; shutting down.");
        is_enabled_now = FALSE;
        llSetTimerEvent(0.0);
        return TRUE;
    }

    // we have not gotten a configuration yet.
    if (global_notecard_name == "") {
///        if (SUPER_NOISY_DEBUG) log_it("in timer with no notecard???");
        return FALSE;
    }

    // crank to next step in config.
    if (!run_through_rez_plan()) {
//log_it("told done with rez plan");
        // we were told not to keep going.  if this is a periodic run,
        // then we'll snooze until the next time.
        if (obj_rezzing_period != 0.0) {
            llSetTimerEvent(obj_rezzing_period);
            finished_file = FALSE;
        }
    }
    // even if the run were done, we still have to handle termination conditions.
    return FALSE;
}

// processes requests made by avatars via chat.
integer handle_hearing_voices(integer channel, string name, key id, string message)
{
    string rez_command = "#rez";
    // is this our command prefix?
    if (is_prefix(message, rez_command)) {
        // we found a command.  which specific one?
        string parm = llDeleteSubString(message, 0, llStringLength(rez_command));
            // eat the command portion, plus a space.
        if ( (parm == "on") && !is_enabled_now) {
            is_enabled_now = TRUE;
            return TRUE;
        } else if ( (parm == "off") && is_enabled_now) {
            is_enabled_now = FALSE;
            return TRUE;
        }
    }
    return FALSE;
}

// a helper function that turns the word "random" into a random value.
// we will never allow a negative z value for the random amount, since that will often be
// wrong (rezzing underneath where the object lives, e.g.).  this implies always
// putting the rezzer below where one wants to see the random rezzing action.
vector maybe_randomize_offset(string vector_or_not, vector largest_add)
{
    if (vector_or_not == "random") {
        vector just_xy = <largest_add.x, largest_add.y, 0.0>;
        vector just_z = <0.0, 0.0, largest_add.z>;
        vector to_return = random_bound_vector(<0.0, 0.0, 0.0>, just_xy, TRUE)
            + random_bound_vector(<0.0, 0.0, 0.0>, just_z, FALSE);
//log_it("calc rand off: " + (string)to_return);            
        return to_return;
    } else {
        return (vector)vector_or_not;
    }
}

vector maybe_randomize_rotation(string vector_or_not, vector largest_add)
{
    if (vector_or_not == "random") {
        vector to_return = random_bound_vector(<0.0, 0.0, 0.0>, largest_add, TRUE);
//log_it("calc rand rot: " + (string)to_return);            
        return to_return;
    } else {
        return (vector)vector_or_not;
    }
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

// consumes the notecard in a very application specific way
// to retrieve the configurations for the items to rez.
parse_variable_definition(string to_parse)
{
    string content;  // filled after finding a variable name.

    if ( (content = get_variable_value(to_parse, "name")) != "")
        obj_name = content;
    else if ( (content = get_variable_value(to_parse, "position")) != "")
        obj_position = (vector)content;
    else if ( (content = get_variable_value(to_parse, "offset")) != "")
        obj_offset = content;
    else if ( (content = get_variable_value(to_parse, "rotation")) != "")
        obj_rotation = content;
    else if ( (content = get_variable_value(to_parse, "multi_offset")) != "")
        obj_multi_offset = content;
    else if ( (content = get_variable_value(to_parse, "multi_angle")) != "")
        obj_multi_angle = content;
    else if ( (content = get_variable_value(to_parse, "singleton")) != "")
        obj_is_singleton = (integer)content;
    else if ( (content = get_variable_value(to_parse, "count")) != "")
        rezzed_at_once = (integer)content;
    else if ( (content = get_variable_value(to_parse, "rate")) != "")
        obj_rezzing_period = (float)content;
    else if ( (content = get_variable_value(to_parse, "max_run")) != "")
        max_run_time = (integer)content;
    else if ( (content = get_variable_value(to_parse, "gift")) != "")
        gift_card_name = content;
    else if ( (content = get_variable_value(to_parse, "maxrandoff")) != "")
        obj_max_offset_add = (vector)content;
    else if ( (content = get_variable_value(to_parse, "maxrandrot")) != "")
        obj_max_rotation_add = (vector)content;
    else if ( (content = get_variable_value(to_parse, "owner_only")) != "")
        owner_only = (integer)content;
    else if ( (content = get_variable_value(to_parse, "die_channel")) != "")
        DIE_ON_DEMAND_CHANNEL = (integer)content;
    else if ( (content = get_variable_value(to_parse, "perform_jaunts")) != "")
        PERFORM_JAUNTS = (integer)content;
}

// examines the configuration to find one section of rezzable plan.
// returns TRUE when the entire config has been consumed.
integer read_one_section()
{
    integer count = llGetListLength(global_config_list);
    if (global_config_index >= count) return TRUE;  // all done.
    integer sec_indy = global_config_index;
    string line = llList2String(global_config_list, sec_indy);
    // search for a section beginning.
    if (llGetSubString(line, 0, 0) == "[") {
        // we found the start of a section name.  now read the contents.
        reset_variables_to_defaults();  // clean out former thoughts.
        if (SUPER_NOISY_DEBUG)
            log_it("reading section: " + llGetSubString(line, 1, -2));
        // iterate across the items in current config section.
        // initializer skips line we just read.
        for (sec_indy++; sec_indy < count; sec_indy++) {
            // read the lines in the section.
            line = llList2String(global_config_list, sec_indy);
            if (llGetSubString(line, 0, 0) != "[") {
                // try to interpret this line as a variable setting.
                parse_variable_definition(line);
            } else {
                // done chowing on defs, so break out.
                global_config_index = sec_indy;
                // if the index is past the end, we're all done.
                return global_config_index >= count;
            }
        }
        global_config_index = sec_indy;
        if (SUPER_NOISY_DEBUG) log_it("read_one_section finished file, post loop.");
        return TRUE;  // at end of list.
    } else {
        if ( (line != "") && !is_prefix(line, "#") ) {
            if (SUPER_NOISY_DEBUG) log_it("skipping gibberish: " + line);
            global_config_index++;
            return FALSE;  // not done yet.
        }
    }

    if (SUPER_NOISY_DEBUG) log_it("read_one_section finished file, bottom bailout.");
    return TRUE;
}

// resets our variables to the default parameters.
reset_variables_to_defaults()
{
    obj_name = "";
    obj_offset = "<0.0, 0.0, 0.0>";
    obj_position = <0.0, 0.0, 0.0>;
    obj_rotation = "<0.0, 0.0, 0.0>";
    obj_is_singleton = FALSE;
    obj_multi_offset = "<0.0, 0.0, 0.0>";
    obj_multi_angle = "<0.0, 0.0, 0.0>";
    rezzed_at_once = 1;
    obj_rezzing_period = 0.0;
    max_run_time = 1200;
    gift_card_name = "";
    owner_only = FALSE;
}

integer completed_previous_section;  // had the last section finished processing?

integer finished_file = FALSE;  // has the config file been consumed?

// returns TRUE while it should continue to be called at the normal rate.
integer run_through_rez_plan()
{
    if (jaunt_responses_awaited > 0) return TRUE;  // not ready yet.
    if (sensor_checks_pending > 0) return TRUE;  // not ready yet.

    if (completed_previous_section) {
//        if (SUPER_NOISY_DEBUG) log_it("decided to read config, prev sec complete now...");
        // read in the next section of config.
        finished_file = read_one_section();
        // reset the current counters of rezzed items for the section.
        current_count = 0;
        current_object_index = 0;
        recent_items_rezzed = 0;
        already_done_items = [];
    }
    // now fire off the rez before we start the next section.
    integer worked = perform_rezzing_action();
    if (!worked) {
        // we can't keep going; there needs to be a pause.
//        if (SUPER_NOISY_DEBUG) log_it("still working on previous section.");
        completed_previous_section = FALSE;
        return TRUE;
    } else {
//        if (SUPER_NOISY_DEBUG) log_it("completed running previous section.");
        completed_previous_section = TRUE;
    }
    
    // check whether we rezzed anything at all, and if we're done.
    if (finished_file) {
//log_it("seeing file as finished");
        if (obj_rezzing_period == 0.0) {
            // this one is not periodic, so we finish out its cycle.
            if (DEBUGGING) log_it("shutting down rezzer; no objects left to create.");
            is_enabled_now = FALSE;
        }
        global_config_index = 0;  // reset reading position for config.
        attempt_to_go_home();  // try to go home after the run.
        // periodic plans will just cause a snooze now.
        return FALSE;
    }
    return TRUE;
}

// resets all our variables and starts reading the rez plan.
crank_rezzer_up()
{
    is_enabled_now = TRUE;  // turn the device on.

    main_home = llGetPos();  // set our home location now.
    main_rotate = llGetRot();  // set our home rotation also.
    when_last_enabled = llGetUnixTime();  // restart the run counter.
    global_config_list = [];  // no config yet.
    global_singleton_checker = "";  // we are not working on a sensor check for anything.
    sensor_checks_pending = 0;  // no sensors are awaited.
    recent_items_rezzed = 0;  // nothing rezzed recently.
    jaunt_responses_awaited = 0;  // no jumps in progress.
    rezzed_overall = 0;  // nothing has been rezzed yet.
    gift_stack = [];  // no gifts waiting yet.

    request_configuration();  // try to find a notecard and read our config.

    // announce that we're open for business.
    if (DEBUGGING) log_it("rezzer started... [free mem=" + (string)llGetFreeMemory() + "]");
}

//////////////
// from hufflets...
//
integer debug_num = 0;

// in the case of rezzeroni, we want to say things out loud for anyone
// to hear, since often the script is open to access by anyone.
// when debugging, we want to say to the owner also, so that problems can be removed.

// a debugging output method.  can be disabled entirely in one place.
log_it(string to_say)
{
    debug_num++;
    if (!SUPER_NOISY_DEBUG) {
        // say this on open chat.
        llSay(0, "[" + (string)debug_num + "] " + to_say);
    } else {
        // tell this to the owner.    
        llOwnerSay(llGetScriptName() + "[" + (string)debug_num + "] " + to_say);
    }
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
//still too noisy...
//    if (SUPER_NOISY_DEBUG)
//        log_it("set '" + variable_name + "' = '" + to_check + "'");
    string chewed_content = to_check;
//this is patching in CR.  shouldn't that be separate?
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

// end hufflets
//////////////

integer trigger_stop_message = FALSE;
    // a simple logging switch; first time in to awaiting commands,
    // we do not want to say a run ended.  but every time after that,
    // we do want to.

default {
    state_entry() { if (llSubStringIndex(llGetObjectName(), "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default
{
    state_entry() {
        auto_retire();
        if (DEBUGGING) log_it("<state default>");
        // we immediately jump into the command handling state.
        // this state is really only for cleaning up older scripts with the auto-upgrade.
        state awaiting_commands;
    }
}

state awaiting_commands
{
    state_entry() {
        if (DEBUGGING) log_it("<state awaiting_commands>");
        llSetTimerEvent(0.0);
        if (trigger_stop_message)
            if (DEBUGGING) log_it("rezzer finished.  [free mem=" + (string)llGetFreeMemory() + "]");
        trigger_stop_message = TRUE;  // always say it next time.
        reset_variables_to_defaults();  // clean out any old notes.
        llListen(0, "", NULL_KEY, "");
    }

    touch_start(integer count) {
        // see if we're blocking others right now.
        if (owner_only && (llDetectedKey(0) != llGetOwner())) return;
        state reading_rez_plan;
    }

    // is someone speaking to the object?
    listen(integer channel, string name, key id, string message) {
        if (owner_only && (llDetectedKey(0) != llGetOwner())) return;
        if (handle_hearing_voices(channel, name, id, message)) state reading_rez_plan;
    }
}

state reading_rez_plan
{
    state_entry() {
        if (DEBUGGING) log_it("<state reading_rez_plan>");
        is_enabled_now = FALSE;

//        // we reset this list only here, since during periodic we'll keep resetting all
//        // our other variables.  this status must persist between runs though.
//        already_done_items = [];  // nothing's been done yet.

        // reset our state so the rez plan will start being read.
        completed_previous_section = TRUE;
        finished_file = FALSE;

        // get the timer cranking so we can process the configuration.
        crank_rezzer_up();
    }
    
    // process the response from the noteworthy library.
    link_message(integer which, integer num, string msg, key id)
    {
        if (handle_link_message(which, num, msg, id))
            state running_rez_plan;
    }

    touch_start(integer count) {}  // do nothing in this state, but keep touch event alive.

}

state running_rez_plan
{
    state_entry() {
        if (DEBUGGING) log_it("<state running_rez_plan>");
        llListen(0, "", NULL_KEY, "");
        llSetTimerEvent(BASE_TIMER_PERIOD);
    }
    
    state_exit() { llSetTimerEvent(0); }

    // we got clicked.
    touch_start(integer count) {
        if (owner_only && (llDetectedKey(0) != llGetOwner())) return;
        // disable the rezzing process.
        is_enabled_now = FALSE;
        llSetTimerEvent(BASE_TIMER_PERIOD);
        // now we must allow the timer to handle the return process.
    }

    // is someone speaking to the object?
    listen(integer channel, string name, key id, string message) {
        if (owner_only && (llDetectedKey(0) != llGetOwner())) return;
        if (handle_hearing_voices(channel, name, id, message)) is_enabled_now = FALSE;
        llSetTimerEvent(BASE_TIMER_PERIOD);
    }
    
    timer() {
        if (handle_timer()) state awaiting_commands;
    }

    object_rez(key id) {
        if (SUPER_NOISY_DEBUG) log_it("heard object rez: id=" + (string)id);
        if (!llGetListLength(gift_stack)) return;  // no gifts to hand out.
        string new_gift = llList2String(gift_stack, 0);
        gift_stack = llDeleteSubList(gift_stack, 0, 0);
        if (find_in_inventory(new_gift, INVENTORY_NOTECARD) < 0) {
            // missing the present.  this seems remarkable.
            log_it("could not find gift specified: " + new_gift);
            return;
        }
        llGiveInventory(id, new_gift);
    }

    // process the response from the noteworthy library.
    link_message(integer which, integer num, string msg, key id)
    { handle_link_message(which, num, msg, id); }

    // a sensor result came in--process either no match or the match.    
    no_sensor() { global_found_the_singleton = FALSE; sensor_checks_pending--; }
    sensor(integer num_detected) { global_found_the_singleton = TRUE; sensor_checks_pending--; }
}
