
// huffware script: inventory exchanger, by fred huffhines.
//
// manages inventory between a root prim and its children.  this script should be placed in
// all prims that need to exchange contents.  the root prim drives the exchange process based
// on a registered set of assets that it is told to synchronize in each child.  all the child
// prims that have this script will ensure they are up to date with respect to those assets.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

// inventory exchanger link message API...
//////////////
// do not redefine these constants.
integer INVENTORY_EXCHANGER_HUFFWARE_ID = 10021;
    // the unique id within the huffware system for the jaunt script to
    // accept commands on.  this is used in llMessageLinked as the num parameter.
string HUFFWARE_PARM_SEPARATOR = "{~~~}";
    // this pattern is an uncommon thing to see in text, so we use it to separate
    // our commands in link messages.
//string HUFFWARE_ITEM_SEPARATOR = "{|||}";
    // used to separate lists of items from each other when stored inside a parameter.
    // this allows lists to be passed as single string parameters if needed.
integer REPLY_DISTANCE = 100008;  // offset added to service's huffware id in reply IDs.
// commands available via the library:
string REGISTER_ASSETS_COMMAND = "#regis#";
    // makes the root prim's inventory exchanger track a set of items which each participating
    // child prim should have.  this command is only available to the root prim; child prims
    // cannot register any assets but instead can get updates on the assets.  the parameter
    // required is a list of asset definitions, wrapped by the huffware parm separator.  each
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

// constants...

integer TIMER_PERIOD = 600;  // the rate at which the root sends out updates (in seconds).

integer INVEXCH_SCRIPT_PIN = -343781294;  // used to slam scripts into the target prim.

// global variables...

list assets_available;
    // the set of items that we will provide on demand.  this list has items wrapped with the
    // asset separator, where the unwrapped entries each provide the type (integer) and the
    // name of the item.

// gives out items that are asked for in "requested_assets".  these go from the
// root prim into the kids.
populate_child_prim(integer link_num, list requested_assets)
{
    key asking_child = llGetLinkKey(link_num);  // the kid that needs the update.
    // look at the assets one by one and pass them over.
    integer undy;
    for (undy = 0; undy < llGetListLength(requested_assets); undy++) {
        // unwrap the next asset definition from the requests.
        list asset_def = llParseString2List(llList2String(requested_assets, undy),
            [ASSET_FIELD_SEPARATOR], []);
        integer type = llList2Integer(asset_def, 0);
        string name = llList2String(asset_def, 1);
//log_it("root is giving link " + (string)link_num + " item " + name);
        // fork over the item itself.
        if (type != INVENTORY_SCRIPT) {
            llGiveInventory(asking_child, name);            
        } else {
            llRemoteLoadScriptPin(asking_child, name, INVEXCH_SCRIPT_PIN, TRUE, 0);
        }
    }

    // we send this event when done updating the kid.     
    llMessageLinked(link_num, INVENTORY_EXCHANGER_HUFFWARE_ID, FINISHED_UPDATING, NULL_KEY);
}

// changes the run state of all the scripts besides this one.
knock_around_other_scripts(integer running_state)
{
    // we won't start anything if we're running inside the updater object.
    if (find_substring(llGetObjectName(), "huffotronic") >= 0) return;
    
    integer indy;
    string self_script = llGetScriptName();
    // we set all other scripts to the running state requested.
    for (indy = 0; indy < llGetInventoryNumber(INVENTORY_SCRIPT); indy++) {
        string curr_script = llGetInventoryName(INVENTORY_SCRIPT, indy);
        if (curr_script != self_script) {
            // this one seems ripe for being set to the state requested.
            llSetScriptState(curr_script, running_state);
        }
    }
}

// kicks forward the periodic activities of the exchange process.
handle_timer()
{
    if (llGetLinkNumber() != 1) {
        // child prim activities only.
//        knock_around_other_scripts(TRUE);
        return;
    } else {
        // the rest are root prim timer activities.
        if (llGetListLength(assets_available) == 0) return;  // nothing to do here.
        // the root needs to send out link messages telling kids what good stuff its got.
        llMessageLinked(LINK_ALL_CHILDREN, INVENTORY_EXCHANGER_HUFFWARE_ID,
            AVAILABLE_ASSETS_EVENT, wrap_parameters(assets_available));
    }

}

// given a packed up list of items, this breaks out each asset definition and
// begins using the set as our full list of available items.
consume_asset_registration(string packed_assets)
{
    assets_available = llParseString2List(packed_assets, [HUFFWARE_PARM_SEPARATOR], []);
//hmmm: future enhancement might be to allow multiple clients of the root exchanger.
//      currently we only support one script inside the root prim listing assets for inv exch.
//      that's shown just above where we're just replacing the entire registered assets list.

    integer indy;
    for (indy = 0; indy < llGetListLength(assets_available); indy++) {
        list asset_def = llParseString2List(llList2String(assets_available, indy),
            [ASSET_FIELD_SEPARATOR], []);
        integer type = llList2Integer(asset_def, 0);
        string name = llList2String(asset_def, 1);
        if (name == WILDCARD_INVENTORY_NAME) {
            // argh, we need to patch up the list for a wildcard option.
            // first, whack the item with the wildcard in it.
            assets_available = llDeleteSubList(assets_available, indy, indy);
            // now revert the list index for the missing item.
            indy--;
            // and finally add all the items of that type.  if one of them is named
            // after the wildcard character, well, we disallow that craziness.
            integer inv_pos;
            for (inv_pos = 0; inv_pos < llGetInventoryNumber(type); inv_pos++) {
                string inv_name = llGetInventoryName(type, inv_pos);
                if (inv_name != WILDCARD_INVENTORY_NAME) {
//log_it("added wild: " + inv_name);
                    assets_available += [ llDumpList2String([type, inv_name], ASSET_FIELD_SEPARATOR) ];
                }
            }
            
        }
    }
    
    // make sure we get this new set out to the interested parties.
    handle_timer();    
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
log_it("removing older asset: " + to_whack);
        llRemoveInventory(to_whack);
    }
}

// ensures that only the latest version of any script or object is kept in our inventory.
// has been called destroy_older_versions in other scripts.
clean_outdated_items()
{
    scrub_items_by_type(llGetScriptName(), INVENTORY_ALL);
    // after cleaning up, make sure everything's running.
    knock_around_other_scripts(TRUE);    
}

// locates the index of a specified type of inventory item with a particular name,
// or returns a negative number.
integer find_inventory(integer type, string name)
{
    integer indy;
    for (indy = 0; indy < llGetInventoryNumber(type); indy++) {
        if (name == llGetInventoryName(type, indy)) return indy;
    }
    return -1;
}

// called on child prims to make sure they have everything the root does.
examine_inventory_freshness(string packed_assets)
{
    list missing_goods;  // fill this with the things we don't have in this prim.
    list provisions = llParseString2List(packed_assets, [HUFFWARE_PARM_SEPARATOR], []);
    integer indy;
    // scan through all the items that are available and make sure we have each of them.
    for (indy = 0; indy < llGetListLength(provisions); indy++) {
        string prov = llList2String(provisions, indy);
        list fields = llParseString2List(prov, [ASSET_FIELD_SEPARATOR], []);
//log_it("checking: " + llList2String(fields, 1) + " type=" + (string)llList2Integer(fields, 0));
        // look for the type and name specified.
        integer type = llList2Integer(fields, 0);
        string name = llList2String(fields, 1);
        if (name == WHACK_INVENTORY_SIGNAL) {
            // we see our special signifier, so remove all items of the type specified.
            // yes, this is dangerous.
            integer delo;
            for (delo = llGetInventoryNumber(type) - 1; delo >= 0; delo--) {
                llRemoveInventory(llGetInventoryName(type, delo));
            }
        } else {
            integer found = find_inventory(type, name);
            if (found < 0) {
//log_it("found " + llList2String(fields, 1) + " was missing!");
                // well, we are out of this one.  tell the root we want it.
                missing_goods += [ prov ];
            }
        }
    }
    if (llGetListLength(missing_goods) > 0) {
        // well we found some things that we're supposed to have, but do not.  we will
        // get the root prim to fix us up.
        llMessageLinked(LINK_ROOT, INVENTORY_EXCHANGER_HUFFWARE_ID,
            REQUEST_UPDATES, wrap_parameters(missing_goods));
    }
}

//hmmm: this is not such a good model?  why do the root and
//  the child both need to be running this check on a timer?

// processes inventory exchange commands in the root prim.  this is where assets must
// be present so they can be distributed to the child prims.
process_root_msg(integer link_num, integer service, string cmd, key parms)
{
    if (service != INVENTORY_EXCHANGER_HUFFWARE_ID) return;  // not for us.
//string name="root";
//if (llGetLinkNumber() != 1) name="child";
//log_it(name + " got request " + cmd);
    if (link_num == 1) {
        // this request is also from the root prim, where this script lives currently.
        if (cmd == REGISTER_ASSETS_COMMAND) {
            // root prim accepts a list of assets to provide to the kids.
            consume_asset_registration(parms);
        }
    } else {
        // a request is coming from a child prim.
        if (cmd == REQUEST_UPDATES) {
            // the root prim responds to update requests by handing out presents.
            populate_child_prim(link_num, llParseString2List(parms, [HUFFWARE_PARM_SEPARATOR], []));
        }
    }
}

// implements the child API, mainly to track missing assets and clean out outdated items.
process_child_msg(integer link_num, integer service, string cmd, key parms)
{
    if (service != INVENTORY_EXCHANGER_HUFFWARE_ID) return;  // not for us.
    if (link_num != 1) return;  // we don't listen to any chatter from other children.
    if (cmd == AVAILABLE_ASSETS_EVENT) {
        examine_inventory_freshness(parms);
    } else if (cmd == FINISHED_UPDATING) {
        clean_outdated_items();
    }
}

// main processing function for the link messge API.
handle_link_message(integer link_num, integer service, string cmd, key parms)
{
    if (llGetLinkNumber() == 1) process_root_msg(link_num, service, cmd, parms);
    else process_child_msg(link_num, service, cmd, parms);
}

//////////////
// beginning of hufflets...
//////////////

// diagnostic hufflets...

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

// joins a list of parameters using the parameter sentinel for the library.
string wrap_parameters(list to_flatten)
{ return llDumpList2String(to_flatten, HUFFWARE_PARM_SEPARATOR); }

// joins a list of sub-items using the item sentinel for the library.
//string wrap_item_list(list to_wrap)
//{ return llDumpList2String(to_wrap, HUFFWARE_ITEM_SEPARATOR); }

// returns the index of the first occurrence of "pattern" inside
// the "full_string".  if it is not found, then a negative number is returned.
integer find_substring(string full_string, string pattern)
{ return llSubStringIndex(llToLower(full_string), llToLower(pattern)); }

// returns TRUE if the "prefix" string is the first part of "compare_with".
integer is_prefix(string compare_with, string prefix)
{ return find_substring(compare_with, prefix) == 0; }

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
        auto_retire();
///log_it("\n\n\n\n\n\n\n\n\n** starting now............................");
        // register child prims for script replacement.
        if (llGetLinkNumber() != 1) llSetRemoteScriptAccessPin(INVEXCH_SCRIPT_PIN);
        //new idea; only allow timers to run in root prim.
        if (llGetLinkNumber() == 1) llSetTimerEvent(TIMER_PERIOD);
        knock_around_other_scripts(TRUE);
        // first run happens right at startup.
        if (llGetLinkNumber() == 1) handle_timer();
    }

    timer() {
        llSetTimerEvent(0);  // stop timer.
        handle_timer();
        llSetTimerEvent(TIMER_PERIOD);  // start timer.
    }

    link_message(integer link_num, integer service, string cmd, key parms) {
        handle_link_message(link_num, service, cmd, parms);
    }
}


