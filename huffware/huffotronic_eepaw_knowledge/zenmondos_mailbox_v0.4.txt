
// huffware script: zen mondo's mailbox, modified by fred huffhines.
//
// original attributions are below.
//
// my changes are licensed via:
//   this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
//   do not use it in objects without fully realizing you are implicitly accepting that license.
//

//////////////////////////////////////
// ZenMondo's Mailbox by ZenMondo Wormser
//
// Displays Online Status, as well as accepting
// notecard "mail" to be be delivered to owner.
//
// Cleans Up after itself, deletes non-notecards.
//
//
// LICENSE:
//
//  This script is given for free, and may NOT be 
//  resold or used in a commercial product.
//  You may copy and distribute this script for free.
//  When you redistribute or copy this script, you must
//  do so with FULL PERMS (modify, copy, and transfer),
//  and leave this notice intact.
//
//  You are free to modify this code, but any derivitive
//  scripts must not be used in a commercial product or
//  or sold, and must contain this license.  
//
//  This script is provided free for learning 
//  purposes, take it apart, break it, fix it, 
//  learn something.  If you come up with something 
//  clever, share it.
//
//  Questions about codepoetry (scripting) can always be addressed
//  to me, ZenMondo Wormser.
//
///////////////////////////////////////// 

key online_query;

key name_query;

string user_name;


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


default {
    state_entry() { if (llSubStringIndex(llGetObjectName(), "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default
{
    state_entry()
    {
        auto_retire();
        llAllowInventoryDrop(TRUE);
        name_query = llRequestAgentData(llGetOwner(), DATA_NAME);
        llSetText("Setting Up, Ready in one minute.", <0,1,1>, 1.0);
        llSetTimerEvent(60);
        
        
    }

    timer()
    {
        online_query = llRequestAgentData(llGetOwner(),DATA_ONLINE);          
    }
    
    dataserver(key queryid, string data)
    {
        
        if(queryid == name_query)
        {
            user_name = data;
            //llSay(0, data + " " + user_name);
        }
        
        if(queryid == online_query)
        {
            integer online = (integer) data;
        
            if(online)
            {
                llSetText(user_name + " is ONLINE\nDrop a NoteCard into me\nto Send " + user_name + " a message.", <0,1,0>, 1.0);
                return;   
            }
        
            else
            {
                llSetText(user_name + " is OFFLINE\nDrop a NoteCard into me\nto Send " + user_name + " a message.", <1,0,0>, 1.0);
                return;
            }
       
        }     
    }
    
    changed(integer mask)
    {
        if(mask & (CHANGED_ALLOWED_DROP | CHANGED_INVENTORY))
        {
            integer num_notes = llGetInventoryNumber(INVENTORY_NOTECARD);
            
            if(num_notes > 0)
            {
                string note_name = llGetInventoryName(INVENTORY_NOTECARD, 0);
                            
                llSay(0, "Sending Notecard, '" + note_name +"' please stand by.");
            
                llGiveInventory(llGetOwner(), note_name);
                
                llInstantMessage(llGetOwner(), "A NoteCard has been sent to you: " + note_name);
                llSay(0, "The Notecard, " + note_name + " has been sent. Thank you.");
                
            
                llRemoveInventory(note_name);
                
                num_notes = llGetInventoryNumber(INVENTORY_NOTECARD);
                
                while(num_notes > 0) // They dropped more than one notecard. Clean it up
                {   
                    note_name = llGetInventoryName(INVENTORY_NOTECARD, 0);
                      
                    llSay(0, "Deleting " + note_name + ". It was not submitted.  Try Dropping one note at a time.");
                    
                    llRemoveInventory(note_name);
                    
                    num_notes = llGetInventoryNumber(INVENTORY_NOTECARD);
                    
                }
                
            }
            
            else //Not a Notecard
            {
               //find out what was dropped and remove it.  
                
                
                list inventory;
                integer num_inv = llGetInventoryNumber(INVENTORY_ALL); // Should be 2
                integer counter = 0;
                while(counter < num_inv)
                {
                    inventory += [llGetInventoryName(INVENTORY_ALL, counter)];
                    counter ++;   
                }
                
                // WHat we expect to find
                list this_script = [llGetScriptName()];
                
                //Delete this script (which belong in the inventory) from the list
                integer index = llListFindList(inventory, this_script);
                inventory = llDeleteSubList(inventory, index, index);
                
                
                index = llGetListLength(inventory);
                
                
                //Just in case they snuck in more than one inventory item
                while (index >= 1)
                {                
                    llSay(0, "That was not a notecard. Removing " + llList2String(inventory, 0));
                    llRemoveInventory(llList2String(inventory, 0));
                    inventory = llDeleteSubList(inventory, 0, 0);
                    index = llGetListLength(inventory);   
                } 
            }
        }
    }
    
    on_rez(integer start_param)
    {
        llResetScript();
    }
}
