
// huffware script: tester for inventory exchanger, by fred huffhines.
//
// makes sure that the inventory exchanger is working, although this currently
// needs some manual inspection to be sure that the items are properly exchanged
// with the child prims.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

// requires inventory exchanger version 2.0 or better.
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

// returns the name of the inventory exchange script, if one is present.  we don't worry
// about there being more than one present; that's a different script's problem rather
// than one from the inventory exchanger.
string get_exchangers_name()
{
    integer found = find_basename_in_inventory("inventory exchanger", INVENTORY_SCRIPT);
    if (found < 0) {
        return "";
    }
    return llGetInventoryName(INVENTORY_SCRIPT, found);
}

// tell the root prim's inventory exchanger what to monitor.
post_exchangeable_assets()
{
    llMessageLinked(LINK_THIS, INVENTORY_EXCHANGER_HUFFWARE_ID, REGISTER_ASSETS_COMMAND,
        wrap_parameters([
            (string)INVENTORY_LANDMARK + ASSET_FIELD_SEPARATOR + "ALL",
            (string)INVENTORY_SOUND + ASSET_FIELD_SEPARATOR + "ALL",
            (string)INVENTORY_SCRIPT + ASSET_FIELD_SEPARATOR + get_exchangers_name(),
            (string)INVENTORY_TEXTURE + ASSET_FIELD_SEPARATOR +  "ALL"
        ]));
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
    // say this on an unusual channel for chat if it's not intended for general public.
//    llSay(108, llGetScriptName() + "[" + (string)debug_num + "] " + to_say);
    // say this on open chat that anyone can hear.  we take off the bling for this one.
//    llSay(0, to_say);
}

// joins a list of parameters using the parameter sentinel for the library.
string wrap_parameters(list to_flatten)
{ return llDumpList2String(to_flatten, HUFFWARE_PARM_SEPARATOR); }

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
    state_entry()
    {
        llSetTimerEvent(14);  // pretty fast update cycle will push the exchange events.
        post_exchangeable_assets();
    }

    // this tester has no link message responsibilities.  once it tells the root prim's
    // inventory exchanger what to do, that's all it needs to say.
//    link_message(integer link_num, integer service, string cmd, key parms) {}
    
    timer() {
        // repost all the assets we care about.  this ensures that we're up to date if the
        // inventory changes.
        post_exchangeable_assets();
    }
}

