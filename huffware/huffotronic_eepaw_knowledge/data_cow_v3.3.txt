
// huffware script: data cow, by fred huffhines.
//
// a data cow is a script that manages a list of text items.  it allows an object
// to offload the memory burden of managing a list of items, since LSL is very tight
// on memory, even when compiled to mono.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

// global variables.

list the_list;
    // the main list that we manage here.  we support adding to it, retrieving it
    // and modifying it via our API.
list the_names;
    // the short names for each item in our list.  we are basically supporting a
    // symbol table data structure here.

// link message API for the data cow library.
//////////////
// do not redefine these constants.
integer DATA_COW_HUFFWARE_ID = 10017;
    // the unique id within the huffware system for the jaunt script to
    // accept commands on.  this is used in llMessageLinked as the num parameter.
string HUFFWARE_PARM_SEPARATOR = "{~~~}";
    // this pattern is an uncommon thing to see in text, so we use it to separate
    // our commands in link messages.
///string HUFFWARE_ITEM_SEPARATOR = "{|||}";
    // used to separate lists of items from each other when stored inside a parameter.
    // this allows lists to be passed as single string parameters if needed.
integer REPLY_DISTANCE = 100008;  // offset added to service's huffware id in reply IDs.
//////////////
//
string RESET_LIST_COMMAND = "reset_L";
    // flushes out the currently held list.  does not send a reply.
string ADD_ITEM_COMMAND = "add_I";
    // adds items to the list.  this is a list of pairs of name/value, where the name is
    // how the item will be looked up from the list, and the value is the contents to be stored.
    // this command has no reply.
string REMOVE_ITEM_COMMAND = "rm_I";
    // accepts a list of names for items.  all the mentioned ones will be removed from the list.
    // this command also has no reply.
string GET_COW_LENGTH = "get_Lc";
    // returns a single integer which is the length of the cow's list currently.
string GET_ITEM_COMMAND = "get_I";
    // retrieves the item's contents for a given name.  first parm is the name.  if there
    // are other parameters, then they are taken as other items to return also.
    // the return data will be pairs of <name, entry> for each of the names in the request
    // that is not empty.  note that you can use an integer index by prefacing it with a
    // '#' sign.  for example, asking for item "#32" will return index 32 in the list of items.
string TAGGED_GET_ITEM_COMMAND = "get_T";
    // like get item, except the first parameter is an identifier that the caller wants to
    // use to tag this request.  the other parameters are still taken as names.  the response
    // will contain the tag as the first item, then the <name, entry> pairs that were found.
//////////////

// sets up the data cow for business.
initialize()
{
    // flush all contents.
    the_list = [];
    the_names = [];
}

// find a named item in our list, if it exists.
integer lookup_item(string name)
{
    integer indy;
    if (llGetSubString(name, 0, 0) == "#") {
        // our special sign for just using an index.
        indy = (integer)llGetSubString(name, 1, -1);
        if ( (indy >= 0) && (indy < llGetListLength(the_names)) ) return indy;
    } else {
        for (indy = 0; indy < llGetListLength(the_names); indy++) {
            if (name == llList2String(the_names, indy)) return indy;  // we know where it is now.
        }
    }
    return -1;  // never found it.
}

// places a named item in our list.  if there's an existing item with
// that name, then it's replaced.
integer global_mem_complained = FALSE;
add_item(string name, string content)
{
    if (llGetFreeMemory() < 2000) {
        if (!global_mem_complained) {
            global_mem_complained = TRUE;
            llOwnerSay("out of memory at " + name);
        }
        return;
    }
    integer current = lookup_item(name);
    if (current < 0) {
//llOwnerSay("new item: " + name);
        // this is a new item.
        the_names += name;
        the_list += content;
    } else {
//llOwnerSay("reusing slot " + (string)current + " for " + name);
        // this is an old item to be replaced.
        the_list = chop_list(the_list, 0, current - 1)
            + [ content ]
            + chop_list(the_list, current + 1, llGetListLength(the_list) - 1);
    }
//log_it("mem=" + (string)llGetFreeMemory());
}

// deletes an existing item if possible.  TRUE is returned on success.
integer remove_item(string name)
{
    integer indy = lookup_item(name);
    if (indy < 0) return FALSE;  // not present.
    // found our sad whackee.  we know the name and content should be
    // stored at the exact same index in the two lists.
    the_names = chop_list(the_names, 0, indy - 1)
        + chop_list(the_names, indy + 1, llGetListLength(the_names) - 1);
    the_list = chop_list(the_list, 0, indy - 1)
        + chop_list(the_list, indy + 1, llGetListLength(the_list) - 1);
    return TRUE;  // all done here.
}

// supports our link message API by answering requests from clients.
handle_link_message(integer sender, integer num, string msg, key id)
{
    if (num != DATA_COW_HUFFWARE_ID) return;  // not for us.
//log_it("hlm--mem free=" + (string)llGetFreeMemory());
//log_it("rcvd: " + msg + " " + (string)num + " " + (string)id);
    if (msg == RESET_LIST_COMMAND) {
        initialize();
    } else if (msg == ADD_ITEM_COMMAND) {
        list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
        integer indy;
        for (indy = 0; indy < llGetListLength(parms); indy += 2) {
            add_item(llList2String(parms, indy), llList2String(parms, indy + 1));
        }
    } else if (msg == REMOVE_ITEM_COMMAND) {
        list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
        integer indy;
        for (indy = 0; indy < llGetListLength(parms); indy++) {
            remove_item(llList2String(parms, indy));
        }
    } else if (msg == GET_COW_LENGTH) {
        list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
        string tag = llList2String(parms, 0);
        list to_reply;
        if (tag != "") to_reply = [ tag ];
        to_reply += [ llGetListLength(the_list) ];
        send_reply(LINK_THIS, to_reply, msg);
    } else if ( (msg == GET_ITEM_COMMAND) || (msg == TAGGED_GET_ITEM_COMMAND) ) {
        list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
        integer indy;
        string tag;
        if (msg == TAGGED_GET_ITEM_COMMAND) {
            // extract the tag, if they want that type of command handling.
            tag = llList2String(parms, 0);
            parms = llDeleteSubList(parms, 0, 0);
        }
        list to_reply;
        // make sure we return the tag if they gave us one.
        if (tag != "") to_reply = [ tag ];
        for (indy = 0; indy < llGetListLength(parms); indy++) {
            // iterate through the parameters and reply about any that we find.
            integer found_posn = lookup_item(llList2String(parms, indy));
            if (found_posn >= 0) {
                // this item did exist.
                to_reply += [ llList2String(the_names, found_posn), llList2String(the_list, found_posn) ];
            }
        }
        send_reply(LINK_THIS, to_reply, msg);
    } 
}

//////////////
// from hufflets...
//
//integer debug_num = 0;

// a debugging output method.  can be disabled entirely in one place.
//log_it(string to_say)
//{
//    debug_num++;
//    // tell this to the owner.    
//    llOwnerSay(llGetScriptName() + "[" + (string)debug_num + "] " + to_say);
//    // say this on open chat, but use an unusual channel.
////    llSay(108, llGetScriptName() + "[" + (string)debug_num + "] " + to_say);
//}

// returns the portion of the list between start and end, but only if they are
// valid compared with the list length.  an attempt to use negative start or
// end values also returns a blank list.
list chop_list(list to_chop, integer start, integer end)
{
    integer last_len = llGetListLength(to_chop) - 1;
    if ( (start < 0) || (end < 0) || (start > last_len) || (end > last_len) ) return [];
    return llList2List(to_chop, start, end);
}

// a simple version of a reply for a command that has been executed.  the parameters
// might contain an outcome or result of the operation that was requested.
send_reply(integer destination, list parms, string command)
{
    llMessageLinked(destination, DATA_COW_HUFFWARE_ID + REPLY_DISTANCE,
        command,  llDumpList2String(parms, HUFFWARE_PARM_SEPARATOR));
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
    state_entry() { initialize(); }

    link_message(integer sender, integer num, string msg, key id)
    { handle_link_message(sender, num, msg, id); }
}

