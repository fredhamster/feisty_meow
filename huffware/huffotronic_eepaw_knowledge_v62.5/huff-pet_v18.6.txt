
// huffware script: huff-pet, by fred huffhines
//
// this is yet another implementation of a pet script in LSL.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

//to-do zone:

//hmmm: for attack mode, adjust timers and ranges.
//  probably just define the default set and mirror that with the attack set.
//  switch between them during mode change.
//  reset timer and sensor for mode change!

// end to-do zone.

// constants for the pet that one might want to change.

integer PET_CHAT_CHANNEL = 28;
    // the channel on which the pet will listen for commands from the owner.

//integer DEFAULT_PANIC_DISTANCE = 28;
integer DEFAULT_PANIC_DISTANCE = 5;//short leash version for opensim.
    // multiplied by the sensor range to get the distance allowed from the
    // pet to the person it's following before a teleport is invoked.
integer ATTACK_PANIC_DISTANCE = 13;
    // multiplied by the sensor range to get the distance allowed from the
    // enraged pet to the attack target before a teleport is invoked.    

float DEFAULT_HEIGHT_ABOVE_FOLLOWED_OBJECT = 1.6;
    // the height that the pet will float at above whoever it's following.
float ATTACK_HEIGHT_ABOVE_FOLLOWED_OBJECT = 0.0;
    // the height that the pet will float at above the attack target.

float DEFAULT_BASE_VELOCITY = 0.1;
    // the velocity of the pet when just cavorting around.
float ATTACK_BASE_VELOCITY = 0.6;
    // the velocity of the pet when in "angry" mode.

integer DEFAULT_BUMP_SIZE = 48;
    // the default size in meters of the displacement caused by up, down, etc commands.

// the margin values below are the range of motion that the pet is allowed
// on each axis.
float DEFAULT_X_MARGIN = 3.0;
float DEFAULT_Y_MARGIN = 3.0;
float DEFAULT_Z_MARGIN = 1.0;
// the margin for when the pet is attacking.
float ATTACK_X_MARGIN = 0.1;
float ATTACK_Y_MARGIN = 0.1;
float ATTACK_Z_MARGIN = 0.2;

float MAXIMUM_TARGETING_DISTANCE = 2.0;
    // the amount of basic deviation allowed for the pet from its target spot.
    // this is how far it's allowed to roam from the target.
//is that right?

float ATTACK_PUSH_DIST_THRESHOLD = 2.0;
    // how close the pet should be to an attack target before trying to push.
float ATTACK_PUSH_MAGNITUDE = 2147483646.0;  //maxint - 1, dealing with svc-2723.
    // how much the critter should push an attack target.
float ATTACK_PUSH_CHANCE = 0.1;
    // how often (probability from 0.0 to 1.0) an attack target gets pushed.

// other constants that are more advanced and should generally not change...

float TARGETING_SENSOR_RANGE = 96.0;
    // the maximum distance the pet will try to see the target at.
    
float SENSOR_INTERVAL = 0.4;
    // how often the sensor scan will fire off.  this is the fastest we will
    // check for our follow target, in seconds.

float PERIODIC_INTERVAL = 0.42;
    // how frequently our timer event fires.

integer MAXIMUM_SLACKNESS = 28;
    // how many timer hits we'll allow before reverting to the default state.

float VELOCITY_MULTIPLIER = 1.2;
    // the total velocity comes from the base plus an equation that multiplies
    // this value by some function of the distance.

string PET_MENU_NAME = "#woof";  // name for our menu.
string PET_REPLY_MENU = "#aroo";  // replies with data.

// symbolic labels for the different states of pet 'being'.
integer STATE_STAY = 0;
integer STATE_FREE = 1;  // go home.
integer STATE_FOLLOW = 2;
integer STATE_COME = 3;
integer STATE_WANDER = 4;
integer STATE_ATTACK = 9;

list SUPPORTED_COMMANDS = [ "go home", "come", "stay",
    "wander", "follow", "attack",
    "set home", "set name", "status" ];
    // attack = follow an avatar in a menacing way.
    // come = follow owner.
    // follow = follow an avatar.
    // go home = return home, then move about freely.
    // set home = set the home position based on owner location.
    // set name = change the object's name.
    // stay = sit right there.
    // wander = roam a greater distance while following owner.

// requires: jaunting library v3.4 or higher.
//////////////
// do not redefine these constants.
integer JAUNT_HUFFWARE_ID = 10008;
    // the unique id within the huffware system for the jaunt script to
    // accept commands on.  this is used in llMessageLinked as the num parameter.
string HUFFWARE_PARM_SEPARATOR = "{~~~}";
    // this pattern is an uncommon thing to see in text, so we use it to separate
    // our commands in link messages.
string HUFFWARE_ITEM_SEPARATOR = "{|||}";
    // used to separate lists of items from each other when stored inside a parameter.
    // this allows lists to be passed as single string parameters if needed.
integer REPLY_DISTANCE = 100008;  // offset added to service's huffware id in reply IDs.
//////////////
// commands available via the jaunting library:
string JAUNT_COMMAND = "#jaunt#";
    // command used to tell jaunt script to move object.  pass a vector with the location.
string FULL_STOP_COMMAND = "#fullstop#";
    // command used to bring object to a halt.
string REVERSE_VELOCITY_COMMAND = "#reverse#";
    // makes the object reverse its velocity and travel back from whence it came.
string SET_VELOCITY_COMMAND = "#setvelocity#";
    // makes the velocity equal to the vector passed as the first parameter.
string JAUNT_UP_COMMAND = "#jauntup#";
string JAUNT_DOWN_COMMAND = "#jauntdown#";
    // commands for height adjustment.  pass a float for number of meters to move.
string JAUNT_LIST_COMMAND = "#jauntlist#";
    // like regular jaunt, but expects a list of vectors as the first parameter; this list
    // should be in the jaunter notecard format (separated by pipe characters).
    // the second parameter, if any, should be 1 for forwards traversal and 0 for backwards.
//
//////////////

//requires menutini library v4.2 or better.
//////////////
// do not redefine these constants.
integer MENUTINI_HUFFWARE_ID = 10009;
    // the unique id within the huffware system for the jaunt script to
    // accept commands on.  this is used in llMessageLinked as the num parameter.
// commands available via the menu system:
string SHOW_MENU_COMMAND = "#menu#";
    // the command that tells menutini to show a menu defined by parameters
    // that are passed along.  these must be: the menu name, the menu's title
    // (which is really the info to show as content in the main box of the menu),
    // the wrapped list of commands to show as menu buttons, the menu system
    // channel's for listening, and the key to listen to.
    // the reply will include: the menu name, the choice made and the key for
    // the avatar.
//
//////////////


request_full_stop()
{
    llMessageLinked(LINK_THIS, JAUNT_HUFFWARE_ID, FULL_STOP_COMMAND, "");
}

request_jaunt_up(integer distance)
{
    llMessageLinked(LINK_THIS, JAUNT_HUFFWARE_ID, JAUNT_UP_COMMAND, (string)distance);
}

// global variables...

key _OWNER;
integer current_state;
vector home_position;  // the location where the pet lives.

integer pending_target = FALSE;  // is a target still being sought.

integer  _COMMAND_CHANNEL;
string   _COMMAND_MESSAGE   = "How may I assist you?";

integer  _TARGET_ID;
vector TARGET_POSITION;

float    _FREE_RANGE        =  10.0;

string SIT_TEXT = "Meditate";

string SIT_ANIMATION = "yoga_float";

vector SIT_POSITION = <0.2, 0.2, 0.4>;

vector SIT_ROTATION = <0.0, 0.0, 0.0>;

key      SITTING_AVATAR_KEY           = NULL_KEY; 

// options for follow menu that pops up when pet is told to follow someone.
list     _FOLLOW_KEY;
list     _FOLLOW_NAME;  // filled in with nearby avatars.
integer  _FOLLOW_CHANNEL;
string   _FOLLOW_MESSAGE = "Who should I follow?";

integer  seeking_avatars         = FALSE;
key      KEY_OF_TARGET;

//////////////

// from hufflets...

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

///////////////

integer debug_num = 0;

// a debugging output method.  can be disabled entirely in one place.
log_it(string to_say)
{
    debug_num++;
    // tell this to the owner.    
    llOwnerSay((string)debug_num + "- " + to_say);
    // say this on open chat, but use an unusual channel.
//    llSay(108, (string)debug_num + "- " + to_say);
}

///////////////

// info...

// returns a string version of the state 's'.
string name_for_state(integer s) {
    if (s == STATE_STAY) return "stay";
    if (s == STATE_FREE) return "go home";
    if (s == STATE_FOLLOW) return "follow";
    if (s == STATE_COME) return "come";
    if (s == STATE_WANDER) return "wander";
    if (s == STATE_ATTACK) return "attack";
    return "unknown";
}

// menu methods...

list current_buttons;  // holds onto the set of menu options.

integer random_channel() { return -(integer)(llFrand(40000) + 20000); }

string stringize_list(list to_flatten) {
    return llDumpList2String(to_flatten, HUFFWARE_ITEM_SEPARATOR);
}

// pops up a menu to interact with the pet's owner.
show_menu(string menu_name, string title, list buttons, integer channel)
{
    current_buttons = buttons;
    key listen_to = _OWNER;
    llMessageLinked(LINK_THIS, MENUTINI_HUFFWARE_ID, SHOW_MENU_COMMAND,
        menu_name + HUFFWARE_PARM_SEPARATOR
        + title + HUFFWARE_PARM_SEPARATOR + stringize_list(current_buttons)
        + HUFFWARE_PARM_SEPARATOR + (string)channel
        + HUFFWARE_PARM_SEPARATOR + (string)listen_to);
}

// causes a state change to make the pet stay.
enter_stay_state()
{
    current_state = STATE_STAY;
    llSensorRemove();
    stop_pet();
}

// handle the response message when the user chooses a button.
react_to_menu(integer sender, integer num, string msg, key id)
{
log_it("react menu: " + msg + " parm=" + (string)id);
    list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
    string menu_name = llList2String(parms, 0);
    string choice = llList2String(parms, 1);

    if ( (num != MENUTINI_HUFFWARE_ID + REPLY_DISTANCE) || (msg != SHOW_MENU_COMMAND) ) {
log_it("why here in react to menu, not for us?");
        return;
    }

    if (menu_name == PET_MENU_NAME) {
log_it("react: snd=" + (string)sender + " num=" + (string)num + " msg=" + msg + " key=" + (string)id);
        if (find_in_list(SUPPORTED_COMMANDS, choice) < 0) {
            llOwnerSay("i don't know what you mean...");
            return;
        }
llOwnerSay("i heard you tell me: menu=" + menu_name + " choice=" + choice);
    
        // handle commands that are major changes in state...
    
        if (choice == "come") {
            llOwnerSay("coming to find you now...");
            current_state = STATE_COME;
            llSetPrimitiveParams([PRIM_PHYSICS, TRUE,
            //]);
            PRIM_PHANTOM, TRUE]);
            llSensorRemove();
            llSensorRepeat("", _OWNER, AGENT, TARGETING_SENSOR_RANGE, PI, SENSOR_INTERVAL);
        }
        
        if (choice == "stay") {
            llOwnerSay("i will stay right here...");
            enter_stay_state();
        }
        
        if (choice == "wander") {
            current_state = STATE_WANDER;
            llOwnerSay("i'm going to wander around and kind of vaguely follow you...");
            llSetPrimitiveParams([PRIM_PHYSICS, TRUE,
            //]);
             PRIM_PHANTOM, TRUE]);
            llSensorRemove();
            llSensorRepeat("", _OWNER, AGENT, TARGETING_SENSOR_RANGE, PI, SENSOR_INTERVAL);
        }
        
        if ( (choice == "follow") || (choice == "attack") ) {
            seeking_avatars = TRUE;
            stop_pet();
            llSensorRemove();
            if (choice == "attack") {
    // we only attack avatars.
    //or not.  since that's boring.  watching a pet attack a physical object is fun.
                llSensor("", NULL_KEY, AGENT | ACTIVE, TARGETING_SENSOR_RANGE, PI);
                current_state = STATE_ATTACK;
            } else {
                // look for both objects and avatars to follow.
                llSensor("", NULL_KEY, AGENT | ACTIVE, TARGETING_SENSOR_RANGE, PI);
                current_state = STATE_FOLLOW;
            }
        }
        
        if (choice == "go home") {
            current_state = STATE_FREE;  // free to roam about the cabin, or wherever home is.
            llOwnerSay("i'm going home now.");
            jaunt_to_location(home_position);
        }
    
        // commands that don't lead to state changes...
    
        if (choice == "status") {
            string seek_addition;
            if (KEY_OF_TARGET != "")
                seek_addition = "was last seeking " + llKey2Name(KEY_OF_TARGET);
            llOwnerSay("my name is " + llGetObjectName() + " and state is '"
                + name_for_state(current_state) + "'.\n"
                + seek_addition);
        }
    
        if (choice == "set home") {
            list pos_list = llGetObjectDetails(llGetOwner(), [OBJECT_POS]);
            home_position = llList2Vector(pos_list, 0);
            llOwnerSay("i'm setting my home to " + (string)home_position);
    //hmmm: use a rounding print to show the position.
        }
        if (choice == "set name") {
            llOwnerSay("to change my name from " + llGetObjectName() + ",\ntell me my new name by typing:\n/" + (string)PET_CHAT_CHANNEL + " name My Cool New Name");
        }
    
    } else if (menu_name == PET_REPLY_MENU) {
log_it("menu-act follow: snd=" + (string)sender + " num=" + (string)num + " msg=" + msg + " id=" + (string)id);
        llSetPrimitiveParams([PRIM_PHYSICS, TRUE,
        //]);
            PRIM_PHANTOM, !(current_state == STATE_ATTACK)]);
        integer choice_indy = find_in_list(_FOLLOW_NAME, choice);
        if (choice_indy < 0) {
//log_it("choice was not found in list");
//log_it("followname list is: " + (string)_FOLLOW_NAME);
        } else {
            string action = "follow";
            if (current_state == STATE_ATTACK) action = "attack";
            llOwnerSay("now " + action + "ing "
                + llList2String(_FOLLOW_NAME, choice_indy) + "...");
            seeking_avatars  = FALSE;
            KEY_OF_TARGET = llList2Key(_FOLLOW_KEY, choice_indy);
            llSensorRemove();
            llSensorRepeat("", KEY_OF_TARGET, AGENT | ACTIVE,
                TARGETING_SENSOR_RANGE, PI, SENSOR_INTERVAL);  
        }
    }
}

// processes the hits that we get back from the sensor.  the information we receive
// is needed for most of the pet states.
handle_sensor(integer num_detected)
{
    if (current_state == STATE_COME) {
        go_to_target(_OWNER, llDetectedPos(0));
        motivate();
    }
    
    if ( (current_state == STATE_FOLLOW) || (current_state == STATE_ATTACK) ) {
        if (seeking_avatars) {
            // reset the list of keys and names that were found previously.
            _FOLLOW_KEY  = [];
            _FOLLOW_NAME = [];
            // show the full set found if it will fit, otherwise just 12.
            integer num_to_show = num_detected;
            if (num_to_show > 12) num_to_show = 12;
            // examine each of the avatars found and put them on the list.
            integer i;
            for (i = 0 ; i < num_to_show; i++) {
                key to_follow = llDetectedKey(i);
                if (to_follow != NULL_KEY) {
                    _FOLLOW_KEY += [to_follow];
                    string str = llDetectedName(i);
                    // trim the menu item if it has hit the maximum limit.
                    if (llStringLength(str) > 24) str = llGetSubString(str, 0, 23);
                    integer name_try = 0;
                    while (find_in_list(_FOLLOW_NAME, str) >= 0) {
                        // this guy is already listed under that name, so change it a bit.
                        str = llGetSubString(str, 0, 22) + (string)name_try++;
                    }
                    _FOLLOW_NAME += [str];
                }
            }
            // now ask who to follow.
            if (llGetListLength(_FOLLOW_KEY)) {
                show_menu(PET_REPLY_MENU, _FOLLOW_MESSAGE, _FOLLOW_NAME, _FOLLOW_CHANNEL);
            }
        } else {
            // not seeking the avatar any more; follow who was chosen.
            go_to_target(KEY_OF_TARGET, llDetectedPos(0));
            motivate();
        }
    }

    if (current_state == STATE_WANDER) {
        if (jaunt_responses_awaited) return;  // skip doing anything while we're still waiting.
        vector pos = llDetectedPos(0);
        float  omg = llFrand(1) * PI * 2;
        float  t_r = llFrand(1) * _FREE_RANGE;
        float  t_x = t_r * llCos(omg);
        float  t_y = t_r * llSin(omg);
        go_to_target(NULL_KEY, pos + <t_x, t_y, 0.0>);
        motivate();
    }

}

handle_timer() {
    if (current_state != STATE_STAY) {
        // make sure a bad jaunt didn't break our physics.
        llSetStatus(STATUS_PHYSICS, TRUE);
    }

    if (jaunt_responses_awaited) {
        // we are not quite there yet.
        if (slackness_counter++ > MAXIMUM_SLACKNESS) {
            // go back to the main state.  we took too long.
log_it("waiting for jaunt timed out.");
///argh?                jaunt_responses_awaited--;
            slackness_counter = 0;
        } else return;  // not time yet for rest of timed actions.
    }
    
    // handle the free state, since we need may to readjust the target.
    if (current_state == STATE_FREE) {
        if (pending_target) return;  // haven't arrived at previous yet.
        vector pos = home_position;
        float  omg = llFrand(1) * PI * 2;
//hmmm: make free range settable
        float  t_r = llFrand(1) * _FREE_RANGE;
        float  t_x = t_r * llCos(omg);
        float  t_y = t_r * llSin(omg);
        go_to_target(NULL_KEY, pos + <t_x, t_y, 0.0>);
        motivate();
    }
}

handle_hearing_voices(integer channel, string name, key id, string message)
{
    if (channel != PET_CHAT_CHANNEL) return;  // not our channel.
//log_it("into handle voice, msg=" + message);
    if (id != llGetOwner()) return;  // not authorized.
    // we found a command.  which specific one?
    if (is_prefix(message, "up")) {
        // upwards bump.
        enter_stay_state();
        string dist = llDeleteSubString(message, 0, 2);
        if (dist == "") dist = (string)DEFAULT_BUMP_SIZE;
        request_jaunt_up((integer)dist);
llOwnerSay("bumping up by " + dist);
    } else if (is_prefix(message, "down")) {
        // downwards bump.
        enter_stay_state();
        string dist = llDeleteSubString(message, 0, 4);
        if (dist == "") dist = (string)DEFAULT_BUMP_SIZE;
        request_jaunt_up(-(integer)dist);
llOwnerSay("bumping down by " + dist);
    } else if (is_prefix(message, "jaunt")) {
        // zip to a specific place in the sim.
        enter_stay_state();
        string where = llDeleteSubString(message, 0, 5);
        if (where == "") {
            llOwnerSay("i can't jaunt to like nowhere dude.");
            return;
        }
        vector loc = (vector)where;
        if (loc == <0.0, 0.0, 0.0>) {
            llOwnerSay("jaunt locations should be in the vector <x, y, z> format, and jaunting to <0, 0, 0> is unsupported.");
            return;
        }
llOwnerSay("jaunting to " + (string)loc);
        jaunt_to_location(loc);
    } else if (is_prefix(message, "name")) {
        // toss the command portion to get our new name.
        string new_name = llDeleteSubString(message, 0, 4);
        if (llStringLength(new_name) > 0) {
            llOwnerSay("wheeee!  my new name is: " + new_name);
            llSetObjectName(new_name);
            show_title();
        } else {
            // no data was given for the name.
            llOwnerSay("my name is still " + llGetObjectName());
        }
    } else {
        // we support a simple translation for a few orders.
        if (message == "free") message = "go home";
        
        // see if we can just flip this into a menu command instead.  we don't
        // really care whether that works or not, since anything that doesn't work is
        // a bogus command.
        llMessageLinked(LINK_THIS, _COMMAND_CHANNEL, SHOW_MENU_COMMAND,
            PET_MENU_NAME + HUFFWARE_PARM_SEPARATOR + message);
    }
}

//////////////

stop_pet()
{
//log_it("stopping pet from moving...");
    llSetPrimitiveParams([PRIM_PHYSICS, FALSE,
    //]);
    PRIM_PHANTOM, TRUE]);
}

go_to_target(key av, vector pos)
{
//log_it("told to go to target: key=" + (string)av + " pos=" + (string)pos);
    TARGET_POSITION = pos;
    if (av != NULL_KEY) {
        vector av_size = llGetAgentSize(av);

        // if it's an object, use a different method to find its height.
        if (av_size.z == 0.0) {
            // use the object's height.
            list box = llGetBoundingBox(KEY_OF_TARGET);
            float object_height = llVecDist(llList2Vector(box, 0), llList2Vector(box, 1));
            av_size.z = object_height;
        }
        // adding to get pet above target.
        TARGET_POSITION += < 0.0, 0.0, av_size.z / 2.0>;
//log_it("adjusted targposn: " + (string)TARGET_POSITION);
    }
    if (current_state == STATE_ATTACK) {
        TARGET_POSITION += < 0.0, 0.0, ATTACK_HEIGHT_ABOVE_FOLLOWED_OBJECT>;
        TARGET_POSITION += <llFrand(2) * ATTACK_X_MARGIN - ATTACK_X_MARGIN,
            llFrand(2) * ATTACK_Y_MARGIN - ATTACK_Y_MARGIN,
            llFrand(2) * ATTACK_Z_MARGIN - ATTACK_Z_MARGIN>;
    } else {
//log_it("normal target calc");
        TARGET_POSITION += < 0.0, 0.0, DEFAULT_HEIGHT_ABOVE_FOLLOWED_OBJECT>;
        TARGET_POSITION += <llFrand(2) * DEFAULT_X_MARGIN - DEFAULT_X_MARGIN,
            llFrand(2) * DEFAULT_Y_MARGIN - DEFAULT_Y_MARGIN,
            llFrand(2) * DEFAULT_Z_MARGIN - DEFAULT_Z_MARGIN>;
    }
    // trim the height a bit to keep the pet on-world.
    if (TARGET_POSITION.z > 4095.0)
        TARGET_POSITION.z = 4095.0;
}

integer jaunt_responses_awaited = 0;
    // the number of pending jumps that we are hoping will happen.

integer slackness_counter;
    // how many snoozes we've had waiting for our destination.

jaunt_to_location(vector target)
{
    // send jaunt request to get us to the specified place.
    llMessageLinked(LINK_THIS, JAUNT_HUFFWARE_ID, JAUNT_COMMAND, (string)target);
    // add one to our counter so we know a jaunt is in progress.
    jaunt_responses_awaited++;
    // reset the overflow counter to recognize a new jaunt.
    slackness_counter = 0;
}

vector previous_position;
    // how far away target was last time.

motivate()
{
    // first, let's get into the right state of existence.
    llSetStatus(STATUS_PHYSICS, TRUE);  // we need to be able to move around here.
    if (current_state == STATE_ATTACK) {
        llSetStatus(STATUS_PHANTOM, FALSE);  // we can bonk into things now.
    } else {
        llSetStatus(STATUS_PHANTOM, TRUE);  // there are no obstructive contacts.
    }

    vector current_pos = llGetPos();
    float distance = llVecDist(TARGET_POSITION, current_pos);
    // a simple linear velocity calculation based on the object's distance.
    float velocity;
    if (current_state == STATE_ATTACK) {
        velocity = ATTACK_BASE_VELOCITY + VELOCITY_MULTIPLIER * (distance / 10.0);
        // beef the velocity up for attack mode.
        velocity *= 10.0;
    } else {
        velocity = DEFAULT_BASE_VELOCITY + VELOCITY_MULTIPLIER * (distance / 10.0);
    }

//hmmm: make that 20 a constant
    integer jump_regardless = FALSE;
    if (llVecDist(current_pos, previous_position) >= 20) {
        // we will always re-target when the distances have changed that much; this could mean
        // the avatar is falling away.
        jump_regardless = TRUE;
    }
float IN_RANGE_CHANCE_TO_BAIL = 0.9;
float ATTACK_IN_RANGE_CHANCE_TO_BAIL = 0.5;
    
float NEAR_RANGE_CHANCE_TO_BAIL = 0.5;
float ATTACK_NEAR_RANGE_CHANCE_TO_BAIL = 0.1;

    if (distance <= MAXIMUM_TARGETING_DISTANCE) {
        // damp out the equation if the target is close enough.
        if (current_state == STATE_ATTACK) velocity = ATTACK_BASE_VELOCITY;
        else velocity = DEFAULT_BASE_VELOCITY;
        float within_range_chance = IN_RANGE_CHANCE_TO_BAIL;
        if (current_state == STATE_ATTACK) within_range_chance = ATTACK_IN_RANGE_CHANCE_TO_BAIL;
        if (llFrand(1.0) <= within_range_chance) return;  // do nothing; close enough.
    } else if (distance <= 2.0 * MAXIMUM_TARGETING_DISTANCE) {
        // we have a bit larger chance of setting a new target if the
        // distance is pretty close still.
        float near_range_chance = NEAR_RANGE_CHANCE_TO_BAIL;
        if (current_state == STATE_ATTACK) near_range_chance = ATTACK_NEAR_RANGE_CHANCE_TO_BAIL;
        if (llFrand(1.0) <= near_range_chance) return;
    }
    previous_position = current_pos;
//log_it("dist=" + (string)distance + " vel=" + (string)velocity);
    float  time = distance / velocity;
    _TARGET_ID = llTarget(TARGET_POSITION, MAXIMUM_TARGETING_DISTANCE);
    pending_target = TRUE;
    // make sure we're in a physics mode before attempting physics changes...
    llSetStatus(STATUS_PHYSICS, TRUE);
    if (SITTING_AVATAR_KEY == NULL_KEY) {
        // when we have nobody riding, we can look wherever we want.
        llLookAt(TARGET_POSITION, 0.7, 0.5);
    } else {
        // if we're holding onto an avatar, we keep them pointed in a reasonable way.
        vector curr_pos = llGetPos();
        vector new_lookat = <curr_pos.x, curr_pos.y, curr_pos.z + 1>;
        llLookAt(new_lookat, 0.7, 0.5);
    }
//log_it("setting move to target: " + (string)TARGET_POSITION);
    llMoveToTarget(TARGET_POSITION, time);

    integer panic_dist = DEFAULT_PANIC_DISTANCE;
    if (current_state == STATE_ATTACK) {
        panic_dist = ATTACK_PANIC_DISTANCE;
    }

    // don't try to jump if we're still awaiting a jump response.    
    if (!jaunt_responses_awaited && (distance > panic_dist) ) {
        // we need to shorten the distance to our buddy now.
        jaunt_to_location(TARGET_POSITION);
    } else if (jump_regardless || (distance > TARGETING_SENSOR_RANGE - 10)) {
        // we are double our panic point, so jump even if still waiting for a reply.
        // however, we don't want to queue up too many jaunts at a time either.
        if (jaunt_responses_awaited <= 2) {
            jaunt_to_location(TARGET_POSITION);
        }
    }

    // push the attack target if we're close enough.
    if ( (current_state == STATE_ATTACK) && (distance < ATTACK_PUSH_DIST_THRESHOLD) ) {
        // only decide to push if they win the lottery here.
        if (llFrand(1.0) < ATTACK_PUSH_CHANCE) {
            llPushObject(KEY_OF_TARGET, ATTACK_PUSH_MAGNITUDE * llRot2Up(llGetRot()), ZERO_VECTOR, FALSE);
        }
    }
    
}

show_title()
{
    llSetText(llGetObjectName(), <0.6, 0.3, 0.8>, 1.0);
}

// processes a link message from some other script.
handle_link_message(integer which, integer num, string msg, key id)
{
//log_it("got msg=" + msg + " id=" + (string)id);
    if (num == JAUNT_HUFFWARE_ID + REPLY_DISTANCE) {
//log_it("link jaunt reply");
        if (msg == JAUNT_COMMAND) {
            jaunt_responses_awaited--;  // one less response being awaited.
            if (jaunt_responses_awaited < 0) {
                log_it("erroneously went below zero for jaunt responses!");
                jaunt_responses_awaited = 0;
            }
            // unpack the reply.
            list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
            integer last_jaunt_was_success = (integer)llList2String(parms, 0);
            vector posn = (vector)llList2String(parms, 1);
//log_it("got a reply for a jaunt request, success=" + (string)last_jaunt_was_success + " posn=" + (string)posn);
        }
        return;
    }
    if (num != MENUTINI_HUFFWARE_ID + REPLY_DISTANCE) return;  // not for us.
//log_it("menu reply");
    react_to_menu(which, num, msg, id);
}

// returns TRUE if the "prefix" string is the first part of "compare_with".
integer is_prefix(string compare_with, string prefix)
{ return (llSubStringIndex(compare_with, prefix) == 0); }

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

initialize()
{
    show_title();
    llSetPrimitiveParams([PRIM_PHYSICS, FALSE,
    //]);
    PRIM_PHANTOM, TRUE]);
    llSitTarget(SIT_POSITION, llEuler2Rot(SIT_ROTATION * DEG_TO_RAD));
    llSetSitText(SIT_TEXT);
    llSetBuoyancy(1.0);
    _OWNER = llGetOwner();
    _FOLLOW_KEY = [];
    _FOLLOW_NAME = [];
    current_state = STATE_FREE;
    TARGET_POSITION = llGetPos();
    llSetTimerEvent(PERIODIC_INTERVAL);
    slackness_counter = 0;
    _COMMAND_CHANNEL = random_channel();
    _FOLLOW_CHANNEL = random_channel();
    _COMMAND_CHANNEL = random_channel();
    llListen(PET_CHAT_CHANNEL, "", llGetOwner(), "");
    home_position = llGetPos();  // start in a known place.
}

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
        initialize();
    }
    
    on_rez(integer param) { llResetScript(); }
    
    touch_start(integer num_detected) {
        show_title();
//change title to show menuing state?
        if (_OWNER == llDetectedKey(0)) {
            // show our menu here.
            show_menu(PET_MENU_NAME, _COMMAND_MESSAGE, SUPPORTED_COMMANDS, _COMMAND_CHANNEL);
        }
    }

    link_message(integer sender, integer num, string msg, key id) {
        handle_link_message(sender, num, msg, id);
    }
    
    sensor(integer num_detected) {
//log_it("sensor found " + llDetectedName(0));
        handle_sensor(num_detected);
    }
    
    no_sensor() {
//use another means to find the avatar?
    }
    
    at_target(integer number, vector targetpos, vector ourpos) {
//log_it("at target");
        llTargetRemove(_TARGET_ID);
        pending_target = FALSE;
        llStopMoveToTarget();
    }
    
    not_at_target() {
//log_it("not at target");

    }
    
    changed(integer change) {
        if (change & CHANGED_LINK) {
            key av = llAvatarOnSitTarget();
            if (SITTING_AVATAR_KEY != NULL_KEY) {
                if (av == NULL_KEY) {
                    llStopAnimation(SIT_ANIMATION);
                    SITTING_AVATAR_KEY = NULL_KEY;
                }
            } else {
                if (av != NULL_KEY) {
                    SITTING_AVATAR_KEY = av;
                    llRequestPermissions(SITTING_AVATAR_KEY, PERMISSION_TRIGGER_ANIMATION);
// we wish we could make the avatar a phantom here, but that's not allowed.
                }
            }
        }
    }

    run_time_permissions(integer perm) {
        key perm_key = llGetPermissionsKey();
        if (perm_key == SITTING_AVATAR_KEY) {
            if (perm & PERMISSION_TRIGGER_ANIMATION) {
                list anms = llGetAnimationList(SITTING_AVATAR_KEY);
                integer i;
                for (i = 0 ; i < llGetListLength(anms) ; i++) {
                    llStopAnimation(llList2Key(anms, i));
                }
                llStartAnimation(SIT_ANIMATION);
            }
        }
    }

    timer() {
        handle_timer();
    }

    listen(integer channel, string name, key id, string message) {
        handle_hearing_voices(channel, name, id, message);
    }

}

// attributions:
//   this script is based (a lot!) on the "pet" script that might
// have been written by kazumasa loon.  there was no attribution
// of author in the script, but the creator was kazumasa.  thanks dude!
//
// that being said, the script was redone a lot by fred huffhines,
// mainly in the following areas:
//
// march or april 2008: added teleport capability to script.  pet will now attempt
//   to keep up with the owner during follow mode by teleporting to her.
//
// may 2008: added ipc menu system.  now menus are dealt with by the huffware
//    menu system, removing a lot of code from this script.

