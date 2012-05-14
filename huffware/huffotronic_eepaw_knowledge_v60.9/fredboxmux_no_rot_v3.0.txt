
// huffware script: fredboxmux (no rotation), by fred huffhines.
//
// a memory saving kludge script; it is the combination of the canonical huffware
// scripts: (1) non-script giver, (2) rotanium rotato, (3) text label.
// our theory is that by having only two scripts per display object (this script and
// the updater client) instead of four, we'll save some cpu on our overburdened but
// beloved server (serene).
// given that this script is intended to replace those three scripts, it will eat them
// on startup to avoid having redundant services in the object.
// update for april 24 2011: added a question menu before doing the copying to user's
// inventory, since opensim can do a weird thing that sets all folders to "loading...".
// this now keeps the objects from being super annoying when one didn't mean to click it.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

// non-script giver:
//
// gives all objects, notecards, etc contained in an object when that object is touched.
// does not give out scripts, since these are generally not something that should be handed
// to the customer.
//
// rotanium rotato:
//
// causes the object to rotate according to the parameters set below.
// this can use herky-jerky timed rotation with llSetRot or it can use
// smooth rotation with llTargetOmega.
//
// text label:
//
// a super simple script for giving an object readable text.

integer DEBUGGING = FALSE;  // set to true for noisier runs.

integer DO_ROTATION = FALSE;  // is this the rotating version or not?

integer USE_SENSORS = TRUE;  // should we look for avatars so we can display the label?

float SMOOTH_TIMER_FREQUENCY = 7.0;
    // fastest possible rate of change for the smooth rotater, in seconds.  the smooth
    // rotater doesn't need to hit the timer all that often, but this is faster than it
    // needs to be for rotation, since it is also used as the rate at which the avatar sensor
    // fires.

float TIME_TO_CLEAR_TITLE = 8.0;
    // how many seconds before the title that was set for an avatar or due to a touch will
    // disappear again.

vector LABEL_COLOR = <0.3, 0.9, 0.4>;
    // color of the text above object.

float ACTIVE_LABEL_DISTANCE = 5.0;
    // how far away can avatars cause the label to light up?

integer ONLY_GIVE_TO_OWNER = TRUE;
    // if this is true, then only the owner will receive a copy of the items.

integer GIVE_UNCOPYABLES = FALSE;
    // this flag is dangerous when true, because it means that uncopyable objects will still
    // be handed to the target.  if that target refuses the non-copyable items, then the items
    // will be lost forever.  that is not so good if you've sold the person a non-copy item.

string EXCLUDED_NOTECARD = "product description";
    // a special case; if there is a giftorse configuration card, we won't hand that out.

float SMOOTH_CHANCE_FOR_ADJUSTING = 0.28;
    // we won't always change the smooth rotation, even though our timer is pretty slow.
    // this value is the percentage of the time that we do actually change rotation (divided
    // by 100).

float SMOOTH_ROTATION_GAIN_MAX = 0.0490873852122;
    // the gain is how fast we will rotate in radians per second.
    // PI / 2 is about 90 degrees per second, which seems way too fast.
    // 0.196349540849 is about PI / 16, 0.0981747704244 is about PI / 32,
    // and 0.0490873852122 is about PI / 64.

string object_label = "default";  // change this if you want a specific name.

string old_name;  // tracks the last known name so we know if we need to update title.
float old_opacity = 3.14;  // tracks the last opacity setting for the label.

integer label_changed = TRUE;
    // this remembers if the label has stayed put or not.  we use it to decide
    // whether we need to check the label for carriage returns.

vector current_add_in = <0.0, 0.0, 0.4>;
    // randomly assigned to if RANDOMIZE_ROTATION is true.

float current_gain = -0.05;
    // speed of smooth rotation; will randomly change if RANDOMIZE_ROTATION is true.

float MIN_ADDITION = 0.01;
    // smallest amount of change we will ever have.
float MAX_ADDITION = 7.0;
    // largest amount of change we will ever have.

key first_toucher;  // tracks who clicked on the object to get contents.

float label_opacity = 1.0;  // how opaque should text be?  1.0 is solid, 0.0 transparent.

// takes out the 3 scripts that have been combined into the mux.  otherwise, it's
// more of a pain to update all the boxes with this thing when it's ready to go.
remove_redundant_scripts()
{
    integer posn;
    string self = llGetScriptName();
    // zoom across the scripts to see if we have any in the inventory that
    // are slated for removal.
    for (posn = llGetInventoryNumber(INVENTORY_SCRIPT) - 1; posn >= 0; posn--) {
        string curr_script = llGetInventoryName(INVENTORY_SCRIPT, posn);
        if (curr_script != self) {
            if ( (llSubStringIndex(curr_script, "non-script giver") == 0)
                || (llSubStringIndex(curr_script, "rotanium rotato") == 0)
                || (llSubStringIndex(curr_script, "text label") == 0) ) {
                // this one is a match!  zap it.
                llRemoveInventory(curr_script);
            }
            
        }
    }
}

// the avatar has said it's okay to hand out all the stuff to her/him.
really_give_contents() { give_out_contents(first_toucher); }

// give out pictures, notecards, objects, etc. that are hiding in the object.
give_out_contents(key give_to)
{
    list all_to_give = [];  // the set we will hand over in a batch.
    list uncopyables = [];  // the list we have to do individually.
    // find out how many items there are.
    integer count = llGetInventoryNumber(INVENTORY_ALL);
    // iterate across all the items and add them to the gift list if appropriate.
    integer indy;
    for (indy = 0; indy < count; indy++) {
        string item_name = llGetInventoryName(INVENTORY_ALL, indy);
        integer type = llGetInventoryType(item_name);
        if ( (type != INVENTORY_SCRIPT) 
            && ( (type != INVENTORY_NOTECARD) || (item_name != EXCLUDED_NOTECARD) ) ) {
            // it's okay to add this item; it's not a script and we are not skipping the special notecard.
            integer mask = MASK_OWNER;
            if (!ONLY_GIVE_TO_OWNER) mask = MASK_EVERYONE;
            integer perms = llGetInventoryPermMask(item_name, mask);
            if (perms & PERM_COPY) {
                // a normal object that we can hand out.
                all_to_give += item_name;
            } else {
                uncopyables += item_name;
            }
        }
    }
    // hand the customer the whole set as one big chunk, named after the object.
    llGiveInventoryList(give_to, llGetObjectName(), all_to_give);

    // handle any problematic items.  we cannot copy these objects into a category folder,
    // so we can either not try to copy them (a lot safer) or we can try to deliver them
    // normally as individual items.  the latter choice is more dangerous, because if the
    // owner discards these items rather than keeping them, the items will be lost forever!
    if (llGetListLength(uncopyables) > 0) {
        string plural = " ";
        string is_verb = "is ";
        string third_noun_subj = "it ";
        string third_noun_obj = "it ";
        if (llGetListLength(uncopyables) > 1) {
            plural = "s ";
            is_verb = "are ";
            third_noun_subj = "they ";
            third_noun_obj = "them ";
        }
        
        string uncopyable_message = "will be left inside the object.  To get " + third_noun_obj
            + ", please copy " + third_noun_obj + "\nmanually from this object into your inventory.";
        if (GIVE_UNCOPYABLES) {
            uncopyable_message = "will be moved over to your inventory."
            + "\nPlease look in your main folders for "
            + third_noun_obj + "(e.g., in Objects or Textures).";
        }
        
        string failure_message = "The item" + plural
            + "[" + llDumpList2String(uncopyables, "; ") + "]\n"
            + is_verb + "not copyable; " + third_noun_subj
            + uncopyable_message;
            
        if (llGetOwner() == give_to) {
            // the object can be moved to inventory, but not with the category method.
            llOwnerSay(failure_message);
        } else {
            // this seems like a weird case; it will probably just fail anyhow?
            // if the item's not copyable and you're not the owner of this object,
            // how can we give it to you?
            llInstantMessage(give_to, failure_message);
        }
        
        // now that we've announced this weird situation, handle it appropriately.
        if (GIVE_UNCOPYABLES) {
            for (indy = 0; indy < llGetListLength(uncopyables); indy++) {
                string item_name = llList2String(uncopyables, indy);
                llGiveInventory(give_to, item_name);
            }
        }  // otherwise leave them be.
    }
}

// causes the object to rotate using whatever the current settings are.
smooth_rotate_using_our_settings()
{
    // make sure we are using the rotational values we were asked to.
    llTargetOmega(current_add_in, current_gain, 1.0);
}

// sets the gain and add in to random choices.
randomize_values()
{
    current_gain = randomize_within_range(0.001, SMOOTH_ROTATION_GAIN_MAX, TRUE);
    current_add_in = random_vector(MIN_ADDITION, MAX_ADDITION, TRUE);
}

// performs the timed rotation that has been configured for us.
rotate_as_requested()
{
    // our slack timer went off, so randomize the rotation if requested.
    if (llFrand(1.0) >= (1.0 - SMOOTH_CHANCE_FOR_ADJUSTING) ) {
        randomize_values();
        smooth_rotate_using_our_settings();
    }
}

initialize_fredboxmux()
{
    // make sure we pick a good random channel.
    menu_system_channel = -1 * (integer)randomize_within_range(200, 10000000, FALSE);

    // if needed, we will set our initial random rotation.
    randomize_values();
        
    // do a first rotate, so we move right at startup.  otherwise we won't move
    // until after our first timer hits.
    if (DO_ROTATION) rotate_as_requested();

    // now set the timer for our mode.
    llSetTimerEvent(SMOOTH_TIMER_FREQUENCY);
    if (DO_ROTATION) smooth_rotate_using_our_settings();
}

set_text()
{
//log_it("old name " + old_name + " -- curr name " + llGetObjectName());
    if (old_name != llGetObjectName()) {
        // we're out of synch on the object name.
        label_changed = TRUE;
    }
    if (old_opacity != label_opacity) {
        // here we're out of synch on last opacity used.
        label_changed = TRUE;
    }
    if (label_changed) {
        // reset the object title to a decorated version of object name if it says "default".
        string new_label = object_label;
        if (new_label == "default") new_label = llGetObjectName();
    
        integer indy;
        integer keep_going = TRUE;
        while (keep_going) {
            indy = find_substring(new_label, "\\n");
            if (indy < 0) {
                keep_going = FALSE;
            } else {
                new_label = llGetSubString(new_label, 0, indy - 1)
                    + "\n" + llGetSubString(new_label, indy + 2, -1);
            }
        }
        old_name = llGetObjectName();
        old_opacity = label_opacity;
        label_changed = FALSE;  // we have dealt with it now.
//log_it("setting text: " + new_label);
        llSetText(new_label, LABEL_COLOR, label_opacity);
    }
    if (label_opacity != 0) {
        // if we set a lit-up title, clear it again pretty soon.
        llSetTimerEvent(TIME_TO_CLEAR_TITLE);
    }
}

//////////////
// code borrowed from menutini to raise a menu asking if they actually meant to get all
// the contents.  an opensim inventory bug makes all the folders look foolish if we
// do any inventory giving accidentally.
//////////////

// global variables...

list _private_global_buttons;  // holds onto the active set of menu options.
string _private_global_av_key;  // the key for the avatar who clicks the menu.
string _private_global_title;  // holds onto current title text.

integer _menu_base_start = 0;  // position in the items of the current menu.

integer listening_id = 0;
    // the current id of our listening for the menu.  it's an id returned by LSL
    // that we need to track so we can cancel the listen.

integer menu_system_channel = -123;
    // messages come back to us from this channel when user clicks the dialog.
    // this is set later and the default is meaningless.

string global_menu_name = "";
    // hangs onto the current menu's name.

//hmmm: note; to manage multiple concurrent menus on different channels,
//      we must make these into lists.  then the timeouts should apply
//      individually to these instead of overall (if we even do timeouts;
//      it's nicer if menus never stop being active).

string NEXT_MENU_TEXT = "Next >>";
    // what the next item will say for showing next menu page.
    
//integer TIMEOUT_FOR_MENU = 42;
    // timeout for the menu in seconds.

// displays the menu requested.  it's "menu_name" is an internal name that is
// not displayed to the user.  the "title" is the content shown in the main area
// of the menu.  "commands_in" is the list of menu items to show as buttons.
// the "menu_channel" is where the user's clicked response will be sent.  the
// "listen_to" key is the avatar expected to click the menu, which is needed to
// listen to his response.
show_menu(string menu_name, string title, list buttons,
    integer menu_channel, key listen_to)
{
    // save our new parms.
    global_menu_name = menu_name;
    _private_global_title = title;
    _private_global_buttons = buttons;
    menu_system_channel = menu_channel;
    _private_global_av_key = listen_to;
    if (DEBUGGING) {
        log_it("menu name: " + global_menu_name);
        log_it("title: " + _private_global_title);
        log_it("buttons: " + (string)buttons);
        log_it("channel: " + (string)menu_system_channel);
        log_it("listen key: " + (string)listen_to);
    }

    integer add_next = FALSE;  // true if we should add a next menu item.

    // the math here incorporates current button position.
    integer current = _menu_base_start;
    integer max_buttons = llGetListLength(buttons) - current;

    if (max_buttons > 12) {
        // limitation of SL: menus have a max of 12 buttons.
        max_buttons = 12;
        add_next = TRUE;
    } else if (llGetListLength(buttons) > 12) {
        // we already have been adding next.  let's make sure this gets
        // a wrap-around next button.
        add_next = TRUE;
    }
    // chop out what we can use in a menu.
    list trunc_buttons = llList2List(buttons, current, current + max_buttons - 1);
    if (add_next) {
        // we were asked to add a menu item for the next screen.
        trunc_buttons = llList2List(trunc_buttons, 0, 10) + NEXT_MENU_TEXT;
    }

    listening_id = llListen(menu_channel, "", listen_to, "");
    list commands;
    integer i;
    // take only the prefix of the string, to avoid getting a length complaint.
    for (i = 0; i < llGetListLength(trunc_buttons); i++) {
        string curr = llList2String(trunc_buttons, i);
        integer last_pos = 23;  // default maximum, highest possible is 24.
        if (llStringLength(curr) - 1 < last_pos) last_pos = llStringLength(curr) - 1;
        curr = llGetSubString(curr, 0, last_pos);
        commands += curr;
    }
    llDialog(listen_to, title, commands, menu_channel);
}

// shuts down any connection we might have had with any active menu.  we will not
// send any responses after this point (although we might already have responded when
// the user clicked the menu).
clear_menu()
{
    llListenRemove(listening_id);
}

// process the response when the user chooses a menu item.
process_menu_response(integer channel, string name, key id, string message)
{
  if (channel != menu_system_channel) return;  // not for us.
  
    if (message == NEXT_MENU_TEXT) {
        // this is the special choice, so we need to go to the next page.
        _menu_base_start += 11;
        if (_menu_base_start > llGetListLength(_private_global_buttons)) {
            // we have wrapped around the list.  go to the start again.
            _menu_base_start = 0;
        }
        show_menu(global_menu_name, _private_global_title,
            _private_global_buttons, menu_system_channel,
            _private_global_av_key);
        return;  // handled by opening a new menu.
    }
    
    string calculated_name;
    integer indy;
    // first try for an exact match.
    for (indy = 0; indy < llGetListLength(_private_global_buttons); indy++) {
        string curr = llList2String(_private_global_buttons, indy);
        if (curr == message) {
            // correct the answer based on the full button string.
            calculated_name = curr;
        }
    }
    if (calculated_name == "") {
        // try an imprecise match if the exact matching didn't work.
        for (indy = 0; indy < llGetListLength(_private_global_buttons); indy++) {
            string curr = llList2String(_private_global_buttons, indy);
            if (is_prefix(curr, message)) {
                // correct the answer based on the full button string.
                calculated_name = curr;
            }
        }
    }
    if (calculated_name == "yes") {
        // only send a response if that menu choice made sense to us.
        really_give_contents();
        clear_menu();
    }
}

// end from menutini.
//////////////

//////////////
// start of hufflets...

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

//////////////

integer debug_num = 0;

// a debugging output method.  can be disabled entirely in one place.
log_it(string to_say)
{
    debug_num++;
    // tell this to the owner.    
    llOwnerSay(llGetDate() + ": " + llGetScriptName() + "[" + (string)debug_num + "] " + to_say);
//llWhisper(0, llGetDate() + ": " + llGetScriptName() + "[" + (string)debug_num + "] " + to_say);
    // say this on an unusual channel for chat if it's not intended for general public.
//    llSay(108, llGetDate() + ": " + llGetScriptName() + "[" + (string)debug_num + "] " + to_say);
    // say this on open chat that anyone can hear.  we take off the bling for this one.
//    llSay(0, to_say);
}

// returns the index of the first occurrence of "pattern" inside
// the "full_string".  if it is not found, then a negative number is returned.
integer find_substring(string full_string, string pattern)
{ return llSubStringIndex(llToLower(full_string), llToLower(pattern)); }

// returns TRUE if the "prefix" string is the first part of "compare_with".
integer is_prefix(string compare_with, string prefix)
{ return (llSubStringIndex(compare_with, prefix) == 0); }

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

//end hufflets...
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
        remove_redundant_scripts();
        initialize_fredboxmux();
        label_opacity = 1;  // start out showing the label.
        set_text();
    }
    
    on_rez(integer start_parm) { state default; }
    
    changed(integer change) {
        if (change & CHANGED_INVENTORY) {
            // we show the label at least a bit when the contents change.
            label_opacity = 1;
            set_text(); 
        }
    }
    
    touch_start(integer num) {
        label_opacity = 1.0;
        set_text();
        first_toucher = llDetectedKey(0);
        // are we only supposed to give stuff to the owner?
        if (ONLY_GIVE_TO_OWNER && (first_toucher != llGetOwner()) ) {
            first_toucher = NULL_KEY;
            return;  // bail out.
        }
        show_menu("askreally", "Would you like a copy of this object's contents?",
            ["yes", "no"], -18264, first_toucher);
    }

    listen(integer channel, string name, key id, string message)
    { process_menu_response(channel, name, id, message); }

    timer() {
        llSetTimerEvent(0);
        if (DO_ROTATION) rotate_as_requested();
        if (USE_SENSORS) {
            llSensor("", NULL_KEY, AGENT, ACTIVE_LABEL_DISTANCE, PI);
        } else {
            if (label_opacity != 0.0) {
                label_opacity = 0.0;
                set_text();
            }            
        }
        llSetTimerEvent(SMOOTH_TIMER_FREQUENCY);
    }

    sensor(integer count) {
        if (label_opacity != 1.0) {
            label_opacity = 1.0;
            set_text();
        }
    }
    
    no_sensor() {
        if (label_opacity != 0.0) {
            label_opacity = 0.0;
            set_text();
        }
    }
}

