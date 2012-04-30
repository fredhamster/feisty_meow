
// huffware script: searchbert armature, by fred huffhines
//
// this script serves two useful functions for searchbert:
// 1) this is searchbert's ease-of-use auto-repair feature.  the script puts searchbert's arms back
// on if they should ever get ripped off.  this happens more frequently than one might expect, but this
// script makes searchbert feel all better again.
// 2) this script also manages the exchange of resources between the root prim and the arms.  if there
// are updates to the search pointer script or others, they'll get synched on a periodic basis.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

//////////////

// global constants.

integer DEBUGGING = FALSE;  // set to true to make the script noisier.

integer SPECIAL_STARTUP_SAUCE = -10408;
    // used to signal that this is a real searchbert arm, so it should evaporate on detach.

integer INVENTORY_UPDATE_INTERVAL = 6000;
    // how often to rescan the inventory so sub-prims get the right contents, in seconds.

integer TOTAL_SEARCH_RODS = 17;  // given our current pattern, this is how many we rez.

// object maintenance variables...

integer objects_rezzed = 0;  // how many kids did we see created?

integer alerted_brainiac = FALSE;  // did we tell the brain that our arms are good?

//////////////

// exported interfaces...

// the armature button pushing API.
// (we have subclassed the simple button pushing API for searchbert armature.)
//////////////
integer BUTTON_PUSHER_HUFFWARE_ID = 10029;
    // a unique ID within the huffware system for this script.
string HUFFWARE_PARM_SEPARATOR = "{~~~}";
    // this pattern is an uncommon thing to see in text, so we use it to separate
    // our commands in link messages.
string HUFFWARE_ITEM_SEPARATOR = "{|||}";
    // used to separate lists of items from each other when stored inside a parameter.
    // this allows lists to be passed as single string parameters if needed.
integer REPLY_DISTANCE = 100008;  // offset added to service's huffware id in reply IDs.
//////////////
string BUTTON_PUSHED_ALERT = "#btnp";
    // this event is generated when the button is pushed.  the number parameter will be
    // the huffware id plus the reply distance.  the id parameter in the link message will
    // contain the name of the button that was pushed.
//////////////
string CHECK_ARMS_BUTTON_NAME = "^checkarms";
    // this is the signal given to the armature script that it should check the
    // number of arms present on searchbert.  if the number is fine, it will
    // push the arms are good button back at the brainiac (using the button
    // push api plus reply distance).
string ARMS_ARE_GOOD_BUTTON_NAME = "^goodarmz";
    // the event sent back by the searchbert armature when all arms are ready
    // to go.
string PROBLEM_WITH_MY_THUMBS_BUTTON_NAME = "^ouch";
    // a problem was noticed with the number of arms and we could not fix it.
    // the brain needs to try reconfiguring again.
//////////////

// imported interfaces...

// inventory exchanger API.
//////////////
// do not redefine these constants.
integer INVENTORY_EXCHANGER_HUFFWARE_ID = 10021;
    // the unique id within the huffware system for the jaunt script to
    // accept commands on.  this is used in llMessageLinked as the num parameter.
//string HUFFWARE_PARM_SEPARATOR = "{~~~}";
    // this pattern is an uncommon thing to see in text, so we use it to separate
    // our commands in link messages.
//string HUFFWARE_ITEM_SEPARATOR = "{|||}";
    // used to separate lists of items from each other when stored inside a parameter.
    // this allows lists to be passed as single string parameters if needed.
//integer REPLY_DISTANCE = 100008;  // offset added to service's huffware id in reply IDs.
//////////////
// commands available via the library:
string REGISTER_ASSETS_COMMAND = "#regis#";
    // makes the root prim's inventory exchanger track a set of items which each participating
    // child prim should have.  this command is only available to the root prim; child prims
    // cannot register any assets but instead can get updates on the assets.  the parameter
    // required is a list of asset definitions, wrapped by the huffware item separator.  each
    // asset definition is in turn a two part list wrapped with the asset field separator.
string ASSET_FIELD_SEPARATOR = "&&";  // separates the type from the name in our list.
string WILDCARD_INVENTORY_NAME = "ALL";
    // this keyword can be used to register all of the inventory items available of the
    // specified type.  this should be passed as the inventory item name in the second half of
    // an asset definition list.
string WHACK_INVENTORY_SIGNAL = "DEL*";
    // ensures that the child prim doesn't maintain *any* of the inventory of the type specified.
string AVAILABLE_ASSETS_EVENT = "#gotz#";
    // the root prim publishes this event to list out what items it has available.  the child
    // prims need to ask for anything that they're currently missing.  the included parameters
    // are in the same format as the register assets command, but no wildcards will be present;
    // all asset names will be fully expanded.
string REQUEST_UPDATES = "#nupd#";
    // used by the child prim to request an updated version of an asset or assets.  the parameters
    // should follow the register assets command.
string FINISHED_UPDATING = "#donq#";
    // a signal sent from the root prim to the child prim when the root has finished updating
    // the assets requested.  this lets the child know that it can clean out any outdated versions
    // of items which got an update.  there are no parameters to this, because the child prim
    // should have at most one update outstanding at a time.
//////////////

// startup function that gets things going.  this is not called at startup time, it
// has to be activated by the brain script.
initialize_armatures()
{
    objects_rezzed = 0;
    alerted_brainiac = FALSE;
    check_integrity();
}

// starts the process of exchanging the inventory items with our arms.
initialize_exchanger()
{
    llSetTimerEvent(5);  // check on inventory soon.
}

// looks for an inventory item with the same prefix as the "basename_to_seek".
integer find_basename_in_inventory(string basename_to_seek, integer inv_type)
{
    integer num_inv = llGetInventoryNumber(inv_type);
    if (num_inv == 0) return -1;  // nothing there!
    integer indy;
    for (indy = 0; indy < num_inv; indy++) {
        if (is_prefix(llGetInventoryName(inv_type, indy), basename_to_seek))
            return indy;
    }
    return -2;  // failed to find it.
}

// lets the searchbert brain know the arms are all ready.
report_arms_are_good()
{
    if (!alerted_brainiac) {
        // let the brain know we're doing okay on our arm count.
        alerted_brainiac = TRUE;    
        llMessageLinked(LINK_SET, BUTTON_PUSHER_HUFFWARE_ID + REPLY_DISTANCE,
            BUTTON_PUSHED_ALERT, ARMS_ARE_GOOD_BUTTON_NAME);
    }
}

// we ran into a snag, like the user didn't give us permission.  we need the
// controlling script to try this all again.
report_arms_are_bad()
{
    alerted_brainiac = FALSE;
    llSay(0, "I did not get permission to attach my seeker arms.");
    llMessageLinked(LINK_SET, BUTTON_PUSHER_HUFFWARE_ID + REPLY_DISTANCE,
        BUTTON_PUSHED_ALERT, PROBLEM_WITH_MY_THUMBS_BUTTON_NAME);
}

// returns the name of the inventory exchange script, if one is present.  we don't worry
// about there being more than one present; that's a different script's problem.
list get_exchanger_and_pointer_names()
{
    list to_return;
    integer found = find_basename_in_inventory("inventory exchanger", INVENTORY_SCRIPT);
    if (found >= 0) {
        to_return += [ (string)INVENTORY_SCRIPT + ASSET_FIELD_SEPARATOR
            + llGetInventoryName(INVENTORY_SCRIPT, found) ];
    }
    found = find_basename_in_inventory("huff-search pointer", INVENTORY_SCRIPT);
    if (found >= 0) {
        to_return += [ (string)INVENTORY_SCRIPT + ASSET_FIELD_SEPARATOR
            + llGetInventoryName(INVENTORY_SCRIPT, found) ];
    }
    return to_return;
}

// tell the root prim's inventory exchanger what to monitor.
post_exchangeable_assets()
{
    // it's time to post.  we allow a couple seconds leeways since timers are inexact.
    llMessageLinked(LINK_THIS, INVENTORY_EXCHANGER_HUFFWARE_ID, REGISTER_ASSETS_COMMAND,
        // what we really want here is to make sure our pointer script is kept up to date.
        wrap_parameters(get_exchanger_and_pointer_names() ));
}

// our timer handling function; this will move the search engine to the next place on
// the current spiral (or to the start of the next spiral) when it is called.
// if we're not currently running a search, then it will just periodically update the
// sub-prims to make sure they have the latest versions.
handle_timer()
{
    // stop the clock.
    llSetTimerEvent(0);
    // if we haven't updated the inventory assets in a while, then we want
    // to make sure the sub-prims have all the right versions of things.
    post_exchangeable_assets();
    // restart ticking.
    llSetTimerEvent(INVENTORY_UPDATE_INTERVAL);
}

// this function is called when we have acquired permissions to manipulate this object,
// so we can crank up all the search rods to be our seekers.
got_link_permissions()
{
    if ( (llGetNumberOfPrims() - 1) >= TOTAL_SEARCH_RODS) {
        // seems like the arms are already present.
        objects_rezzed = TOTAL_SEARCH_RODS;        
        if (DEBUGGING) log_it("I'm done setting up my arms.");
        // make sure we alert brain.
        check_integrity();
        return;
    }

    // reset our count now that we're going to reattach everything.
    objects_rezzed = 0;

    // postpone the next inventory exchange until we have all our arms on.
    // each link addition takes at least a second of built in delay (currently).
    llSetTimerEvent(TOTAL_SEARCH_RODS + 7);

    llBreakAllLinks();  // drop all current attachments and let them evaporate.

    string search_object = llGetInventoryName(INVENTORY_OBJECT, 0);

    integer which_locator = 2;
    float curr_rotation;
    // calculate the size of the root prim so we can put the search arms in the
    // right locations.
    list sizes = llGetBoundingBox(llGetKey());
    vector min = llList2Vector(sizes, 0);
    vector max = llList2Vector(sizes, 1);
    // not exactly sure if this formula will always be right.  have seen some anomalous
    // results from get bounding box.
    float radius = (max.x - min.x) / 2.0;

    for (curr_rotation = 0.0; curr_rotation < PI / 4.0 + .001;
            curr_rotation += PI / 4.0) {
        float x = 0.0;
        // handle dots on x axis.
        for (x = -0.4 / 0.54 * radius; x < 0.41 / 0.54 * radius; x += 0.2 / 0.54 * radius) {
            if ( (curr_rotation != 0.0) || (llRound(x * 100.0) != 0) ) {
                vector rez_place = <x, 0.0, 0.0>;
                rotation z_45 = llEuler2Rot(<0.0, 0.0, curr_rotation>);
                rez_place *= z_45;
                rez_place += llGetPos();
//log_it("x rez place " + (string)rez_place + " rot=" + (string)curr_rotation);
                which_locator++;
                llRezObject(search_object, rez_place, ZERO_VECTOR, ZERO_ROTATION,
                    SPECIAL_STARTUP_SAUCE);
            }
        }
        float y = 0.0;
        // handle dots on y axis.
        for (y = -0.4 / 0.54 * radius; y < 0.41 / 0.54 * radius; y += 0.2 / 0.54 * radius) {
            if (llRound(y * 100.0) != 0) {
                vector rez_place = <0.0, y, 0.0>;
                rotation z_45 = llEuler2Rot(<0.0, 0.0, curr_rotation>);
                rez_place *= z_45;
                rez_place += llGetPos();
//log_it("y rez place " + (string)rez_place + " rot=" + (string)curr_rotation);
                which_locator++;
                llRezObject(search_object, rez_place, ZERO_VECTOR, ZERO_ROTATION,
                    SPECIAL_STARTUP_SAUCE);
            }
        }
    }
}

handle_new_child(key new_child)
{
    // report (via text label) how many others there are still to hook in.
    integer count;
    integer remaining_to_rez = TOTAL_SEARCH_RODS - objects_rezzed;
    string tag;
    for (count = 0; count < remaining_to_rez; count++) tag += "+";
    llSetText(tag + " " + (string)remaining_to_rez + " " + tag,
        <0.4 + llFrand(0.6), 0.4 + llFrand(0.6), 0.4 + llFrand(0.6)>, 1.0);
    // link the new kid to us.
    // this also delays the script a whole second?!  argh.
    llCreateLink(new_child, TRUE);
    // check if we're done yet.
    if (++objects_rezzed >= TOTAL_SEARCH_RODS) {
        // that was the last one.
        llSetText("", <0.0, 0.0, 0.0>, 0.0);
        if (DEBUGGING) log_it("All of my arms have been rezzed and connected now.");
        // one last check and alert the brain about us being ready.
        check_integrity();
    }
}

// processes a request for our armature services, probably from the search brain.
handle_link_message(integer which, integer num, string msg, key id)
{
    if ( (num == BUTTON_PUSHER_HUFFWARE_ID)
            && (msg == BUTTON_PUSHED_ALERT)
            && (id ==  CHECK_ARMS_BUTTON_NAME) ) {
        // see if we can put our arms on, if they're not already.
        initialize_armatures();
    }
}

// makes sure that searchbert is all assembled properly.
check_integrity()
{
    if (llGetNumberOfPrims() - 1 < TOTAL_SEARCH_RODS) {
        alerted_brainiac = FALSE;
        if (DEBUGGING) log_it("I do not have enough arms on my body.  Please grant me permission to link and delink.");
        // get permissions to take off our arms and add possibly updated ones.
        llRequestPermissions(llGetOwner(), PERMISSION_CHANGE_LINKS);
    } else {
        if (!alerted_brainiac) {
            // let the brain know we're doing okay on our arm count.
            report_arms_are_good();
            alerted_brainiac = TRUE;
        }
    }
}

//////////////
// from hufflets...

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

integer debug_num = 0;
// a debugging output method.  can be disabled entirely in one place.
log_it(string to_say)
{
    debug_num++;
    // tell this to the owner.    
    llWhisper(0, llGetScriptName() + "[" + (string)debug_num + "] " + to_say);
    // say this on an unusual channel for chat if it's not intended for general public.
//    llSay(108, llGetScriptName() + "[" + (string)debug_num + "] " + to_say);
    // say this on open chat that anyone can hear.  we take off the bling for this one.
//    llSay(0, to_say);
}

// joins a list of parameters using the parameter sentinel for the library.
string wrap_parameters(list to_flatten)
{ return llDumpList2String(to_flatten, HUFFWARE_PARM_SEPARATOR); }

string wrap_item_list(list to_wrap)
{ return llDumpList2String(to_wrap, HUFFWARE_ITEM_SEPARATOR); }

// returns the index of the first occurrence of "pattern" inside
// the "full_string".  if it is not found, then a negative number is returned.
integer find_substring(string full_string, string pattern)
{ return llSubStringIndex(llToLower(full_string), llToLower(pattern)); }

// returns TRUE if the "prefix" string is the first part of "compare_with".
integer is_prefix(string compare_with, string prefix)
{ return find_substring(compare_with, prefix) == 0; }

// end hufflets.
//////////////

default {
    state_entry() { if (llSubStringIndex(llGetObjectName(), "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default {
    state_entry() {
        auto_retire();
        initialize_exchanger();
    }
    
    on_rez(integer parm) { llResetScript(); }
    
    object_rez(key new_child) { handle_new_child(new_child); }

    timer() { handle_timer(); }

    link_message(integer which, integer num, string str, key id)
    { handle_link_message(which, num, str, id); }

    run_time_permissions(integer perm) {
        if (perm & PERMISSION_CHANGE_LINKS)
            got_link_permissions();
        else
            report_arms_are_bad();
    }

    changed(integer change) {
        if (change & CHANGED_INVENTORY) {
            llSetTimerEvent(2);  // fire exchange sooner in case scripts changed.
        } else if (change & CHANGED_LINK) {
            check_integrity();
        }
    }

}

