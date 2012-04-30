
// huffware script: welcomebot visitor list, by fred huffhines
//
// originally based on LSL wiki examples.
// how to use this script:
//   1) create an object that you want to track visitors to your area.
//   2) change the parameters below to reflect your own information.
//   3) add the script to your tracker object.
//   4) reply to the script's rez email with the following commands:
//      list: send the current visitor list.
//      scan: scan the area near the greeter object.
//      clear: throw out the visitor list.  sends a copy before deleting.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

//todo:
// how do we reconcile having a config notecard with the giveaway ones?
//   just name it special?
// we want the script not to reset when the parameters change!
//    it should just reset the notecard stuff and other variables.
//    no new setup on email and all that stuff!
// maintain list of last visit times.
//    just as a string?


// put your e-mail address here.
string owner_email = "fred@gruntose.com";

// put the GUIDs of your business partners here.  the owner is automatically
// notified.
list notification_list = [];

// place name used to identify this visitor list.
string greeter_name = "eepaw shop welcomebot";

// the description of where this is.
string welcome_message = "Welcome to the eepaw shop (Eclectic Electric Patterns and Widgets).  Please touch the welcomebot for more information.";

// you can make the bot listen on a different channel if you like...
integer OWNER_CHANNEL = 14;

integer MAX_ITEMS_PER_LINE = 4;  // how many visitors to show on one line of output.

integer EMAIL_RECEPTION_ENABLED = FALSE;  // currently not enabled in opensim.

integer EMAIL_SENDING_ENABLED = FALSE;  // apparently many sims don't allow email sending.

integer MAX_VISITORS_TRACKED = 120;
    // the number of items we keep around in the list of visitors.

float pause_when_mail_pending = 2.0;  // check for mail this frequently when there's a stack.
float pause_no_pending_mail = 42.0;  // if no mail had been waiting, we check more slackly.

// number of seconds between sending the owner a visitor list.
integer auto_list_interval = 43200;  // 43200 is every 12 hours.

// any variables that need to be modified are above this...
///////////////////////////////////////////////////////////

float SENSOR_RANGE = 10.0; // in meters.

float SENSOR_RATE = 30.0; // in seconds.

// Global variables 
string regionName; 
key myKey; 
list visitor_list; 
list current_list; 
string scan_list; 
float maxZ = 0.0; 
float minZ = 0; 
integer MAXZ = 20; 
integer MINZ = 20; 
integer last_automatic_list_sent;  // when we last updated the owner automatically.

// sends an email.  somewhat a thin wrapper.
send_email(string recipient, string subject, string text)
{
    if (EMAIL_SENDING_ENABLED) {
        // some sims do not support email.
        llEmail(recipient, subject, text);
    }
}

cloaked_owner_alert(string text)
{
    // temporarily change the name so emails will come from the expected greeter.
    string hold_name = llGetObjectName();
    llSetObjectName(greeter_name);
    // send the message.
    llOwnerSay(text);
    // restore our name back to the original.
    llSetObjectName(hold_name);
}

cloaked_IM(key who_to_tell, string text)
{
    // temporarily change the name so emails will come from the expected greeter.
    string hold_name = llGetObjectName();
    llSetObjectName(greeter_name);
    // send the message.
    llInstantMessage(who_to_tell, text);
    // restore our name back to the original.
    llSetObjectName(hold_name);
}

// notifies all the people in the list who want to hear about visitors.
notify_folks(string text)
{
    // temporarily change the name so emails will come from the expected greeter.
    string hold_name = llGetObjectName();
    llSetObjectName(greeter_name); 

    list temp_notify = notification_list;
    temp_notify += [llGetOwner()];
        
    integer i;
    for (i = 0; i < llGetListLength(temp_notify); i++) {
        string current_id = llList2String(temp_notify, i);
        if ((key)current_id == llGetOwner()) cloaked_owner_alert(text);
        else cloaked_IM((key)current_id, text);
    }

    // restore our name back to the original.
    llSetObjectName(hold_name);
}

// looks for the name provided in a list.  the index is returned if
// the name is found, otherwise a negative number is returned.
integer find_name_in_list(string name, list avlist) 
{ 
    integer len = llGetListLength(avlist);
    integer i; 
    for (i = 0; i < len; i++) 
    { 
        if (llList2String(avlist, i) == name) 
            return i; 
    } 
    return -1;
} 

// looks for changes in the current locale.
check_region(list detected) 
{
    // we accumulate information about people who have changed state in the variables below.
    string names_entering = ""; 
    string names_leaving = ""; 
    integer num_entering = 0; 
    integer num_leaving = 0; 

    // see if we have somebody new.
    integer len = llGetListLength(detected); 
    integer i; 
    for (i = 0; i < len; i++) { 
        string name = llList2String(detected, i); 
        if (find_name_in_list(name, current_list) < 0) { 
            current_list += name; 
            if (num_entering > 0) names_entering += ", "; 
            names_entering += name; 
            num_entering++; 
        } 
    } 
     
    // see if somebody left.
    len = llGetListLength(current_list); 
    for (i = 0; i < len; i++) { 
        string name = llList2String(current_list, i); 
        if (find_name_in_list(name, detected) < 0) {
            if (num_leaving > 0) names_leaving += ", "; 
            names_leaving += name; 
            num_leaving++; 
        } 
    } 

    // update current list to be current only     
    current_list = detected;

    // create a message describing the state.
    string msg = ""; 
    if (num_entering > 0) msg = "Arrived: " + names_entering;
    if (num_leaving > 0) {
        if (llStringLength(msg) > 0) msg += "\n";
        msg += "Left: " + names_leaving;
    }
    if (msg != "") notify_folks(msg); 
} 

string build_visitor_list()
{
    string text = greeter_name + ":\n"; 
    integer len = llGetListLength(visitor_list); 
    integer i;
    integer items_per_line = 0;
    for (i = len - 1; i >= 0; i--) {
        if (items_per_line != 0) {
            text += "\t";
        }
        text += llList2String(visitor_list, i);
        items_per_line++;
        if (items_per_line >= MAX_ITEMS_PER_LINE) {
            text += "\n";
            items_per_line = 0;
        }
    }
    // handle case for last line.
    if (items_per_line != 0) text += "\n";
    text += "Total=" + (string)len; 
    return text;
}

send_list_in_text()
{
    llOwnerSay(build_visitor_list());
}

send_scan_in_text()
{
    llOwnerSay(greeter_name + " scan\n" + scan_list);
}

send_list_in_email() 
{ 
    string text = build_visitor_list();
    send_email(owner_email, greeter_name + " visitor list", text); 
} 

send_scan_in_email() 
{ 
    send_email(owner_email, greeter_name + " scan", scan_list); 
} 

clear_list()
{
    visitor_list = [];
}

initialize_greeter() 
{ 
    last_automatic_list_sent = llGetUnixTime(); 
     
    scan_list = ""; 
     
    myKey = llGetKey(); 
    regionName = llGetRegionName(); 
     
    vector rezPos = llGetPos(); 
    maxZ = rezPos.z + MAXZ; 
    minZ = rezPos.z - MINZ; 
    string rezInfo = "Key: "+(string)myKey + " @ "+regionName+" "+(string)rezPos+" ("+(string)maxZ+")"; 
    notify_folks(rezInfo);
    send_email(owner_email,  greeter_name + " Rez Information", rezInfo); 
    llSetTimerEvent(pause_no_pending_mail); 
    llSensorRepeat( "", "", AGENT, SENSOR_RANGE, TWO_PI, SENSOR_RANGE);
    // hook up the bot to the radio so it will hear commands.
    llListen(OWNER_CHANNEL, "", llGetOwner(), "");
    llOwnerSay("Listening to commands on channel " + (string)OWNER_CHANNEL
        + " (list, scan, clear)");
}

// performs the requested command.
process_command_locally(string msg)
{
    if (msg == "list") send_list_in_text(); 
    else if (msg == "scan") send_scan_in_text();
    else if (msg == "clear") {
        send_list_in_text();
        clear_list();
    }    
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
        initialize_greeter(); 
    }
     
    on_rez(integer param) { llResetScript(); } 
     
    timer() 
    {
        if (EMAIL_RECEPTION_ENABLED) {
            llGetNextEmail("", ""); // check for email with any subject/sender 
             
            integer curTime = llGetUnixTime(); 
            if (curTime - last_automatic_list_sent < auto_list_interval) return; 
            last_automatic_list_sent = curTime; 
            send_list_in_email(); 
        }
    } 

    email(string time, string address, string subj, string message, integer num_left) 
    { 
        list args = llParseString2List(message, [" "], []); 
        if (llGetListLength(args) < 1) return;  // nothing to do.
        string cmd = llToLower(llList2String(args, 0)); 
//room for a process_command_for_email here.
        if (cmd == "list") send_list_in_email(); 
        else if (cmd == "scan") send_scan_in_email();
        else if (cmd == "clear") {
            send_list_in_email();
            clear_list();
        }

        if (num_left == 0) llSetTimerEvent(pause_no_pending_mail);  // sleep longer before checking.
        else llSetTimerEvent(pause_when_mail_pending);  // short sleep since others are waiting.
    }
    
    listen(integer channel, string name, key id, string msg)
    {
        if ( (channel != OWNER_CHANNEL) || (id != llGetOwner()) ) return;  // not for them.
        process_command_locally(msg);
    }

    // when a visitor touches the welcomebot, hand out the first landmark and
    // first notecard we have stored.
    touch_start(integer count) {
        string item_name = llGetInventoryName(INVENTORY_NOTECARD, 0);  // first notecard.
        if (item_name != "")
            llGiveInventory(llDetectedKey(0), item_name);
        item_name = llGetInventoryName(INVENTORY_LANDMARK, 0);
        if (item_name != "")
            llGiveInventory(llDetectedKey(0), item_name);
    }

    sensor(integer number_detected) 
    { 
        list detected = []; 
        vector pos = llGetPos(); 
        string textVerbose = ""; 
        string textNewVerbose = ""; 
        string textNew = ""; 
        integer other = 0; 
        integer group = 0; 
        integer new = 0; 
        integer outer; 
         
        textVerbose = ""; 
        for (outer = 0; outer < number_detected; outer++) 
        { 
            vector dpos = llDetectedPos(outer); 
            if (llGetLandOwnerAt(dpos)==llGetLandOwnerAt(llGetPos())) 
            { 
                string detected_name = llDetectedName(outer); 
                string detected_key = llDetectedKey(outer); 
                integer info_res = llGetAgentInfo(detected_key); 
                float diff = llVecDist(pos,dpos); 
                integer dist = llRound(diff); 
                string result = detected_name+" ("+detected_key+") " + (string)dist; 
                if(dpos.y>pos.y) result+="N"; 
                else if(dpos.y<pos.y) result+="S"; 
                if(dpos.x>pos.x) result+="E"; 
                else if(dpos.x<pos.x) result+="W"; 
                result += (string)((integer)(dpos.z-pos.z)); 
                result += " "; 
                if(info_res & AGENT_SCRIPTED)   result += "S"; 
                if(info_res & AGENT_FLYING)     result += "F"; 
                if(info_res & AGENT_AWAY)       result += "A"; 
                if(info_res & AGENT_IN_AIR)     result += "H"; 
                if(info_res & AGENT_MOUSELOOK)  result += "M"; 
                if (llDetectedGroup(outer)) result += "G"; 
             
                if ((dpos.x >= 0 && dpos.x <= 256) && 
                    (dpos.y >= 0 && dpos.y <= 256) && 
                    (dpos.z >= minZ) && (dpos.z <= maxZ)) 
                { 
                    if (llDetectedGroup(outer)) group++; 
                    else other++; 

                    // add to the detected list 
                    detected += detected_name; 
                     
                    // see if they are on the visitor list 
                    if (find_name_in_list(detected_name, visitor_list) < 0) {
                        // make sure we haven't gone beyond our limit.
                        if (llGetListLength(visitor_list) >= MAX_VISITORS_TRACKED) {
                            visitor_list = llDeleteSubList(visitor_list, 0, 0);
                        }
                        // add to the visitor list 
                        visitor_list += detected_name;
                         
                        // handle any separators to make more readable 
                        if (new > 0)  { 
                            textNew += ", "; 
                            textNewVerbose += "\n"; 
                        } 
                         
                        // add to the text 
                        textNew += detected_name; 
                        textNewVerbose += detected_name+" ("+detected_key+") @ "+regionName+" "+(string)dpos; 
                        result += "N"; 
                         
                        // increment the count of new visitors 
                        new++; 
                        cloaked_IM(detected_key, welcome_message);
                    } 
                } 
                else 
                { 
                    result += "*"; 
                } 
                textVerbose += result + "\n"; 
            } 
        } 
         
        if (new > 0) {
            // extra verbose logging.
//            notify_folks(textNew + "\n" + textNewVerbose);
        }
        scan_list = textVerbose; 
         
        check_region(detected); 
    } 
     
    no_sensor() 
    { 
        check_region([]);
    } 
}
