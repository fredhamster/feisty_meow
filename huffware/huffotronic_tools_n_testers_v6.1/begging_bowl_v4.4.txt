
// huffware script: begging bowl, by fred huffhines.
//
// handles gifts from visitors at our shop.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

// constants.

integer BEGGING_INTERVAL = 8;
    // the period (in seconds) of our asking the avatar for money.
    // keeping this reasonably high ensures that the begging jar isn't
    // too noisy if someone is whanging away at it to play the sound or
    // see the explosion.

// variables, some with initializers.

integer last_begging_time;  // when we last pleaded for additional funding.

integer tip_amount;  // the amount given to us just recently.
integer total_tips = 638;
    // 108 -- aug 30 2008 or so.  214 -- nov 28 2008.  342 -- dec 28 2008.
    // 486 -- mar 21 2009.  589 -- may 12 2009.  632 -- aug 02 2009.
    // 638 -- aug 31 2009.
key tippers_key = NULL_KEY;  // the key for the avatar that has tipped us.
string last_tipper;  // the name of the last tipper.

show_label()
{
    string msg = "{ eepaw tipster }\ntouch for more info...\n.\nL$"
        + (string)total_tips + " donated so far--thanks!";
    if (last_tipper != "")
        msg += "\nLast recvd L$" + (string)tip_amount + " from " + last_tipper + ", yay!";
    llSetText(msg, <0.8, 0.6, 0.9>, 1);
}

initialize_begging_bowl()
{
    show_label();
    llOwnerSay("ready to receive tips from customers...");
    llParticleSystem([]);
    last_begging_time = llGetUnixTime();  // reset just to have some kindo value.
    show_label();
}

thank_tipper_and_give_gifts(string name)
{
    llSay(0, "eepaw shop and its workers thank you for the tip, " + name + "!");
    last_tipper = name;  // store this for our text.
    string first_name = llDeleteSubString(name, llSubStringIndex(name, " "), -1);
    llInstantMessage(tippers_key, first_name + ", thank you very much for your tip of L$"
        + (string)tip_amount + ", from all of us at eepaw shop.");
    llOwnerSay("received tip from " + name + " of L$" + (string)tip_amount + ".");
    if (llGetInventoryNumber(INVENTORY_SOUND)) {
        // we will only play one sound currently.
        llPlaySound(llGetInventoryName(INVENTORY_SOUND, 0 ), 1.0);
    }
    
    // give out pictures, notecards and objects that are hiding in the object.
    integer indy;
    list all_to_give;  // the full set of gifts.    
    for (indy = 0; indy < llGetInventoryNumber(INVENTORY_TEXTURE); indy++) 
        all_to_give += llGetInventoryName(INVENTORY_TEXTURE, indy);
    for (indy = 0; indy < llGetInventoryNumber(INVENTORY_NOTECARD); indy++) 
        all_to_give += llGetInventoryName(INVENTORY_NOTECARD, indy);
    for (indy = 0; indy < llGetInventoryNumber(INVENTORY_OBJECT); indy++)
        all_to_give += llGetInventoryName(INVENTORY_OBJECT, indy);
    for (indy = 0; indy < llGetInventoryNumber(INVENTORY_OBJECT); indy++)
        all_to_give += llGetInventoryName(INVENTORY_SOUND, indy);
    // pick a semi-meaningful folder name.
    string gift_folder_name = "eepaw thanky! (you gave us L$"
        + (string)tip_amount + " on " + llGetDate() + ")";
    // then gift it over.
    llGiveInventoryList(tippers_key, gift_folder_name, all_to_give);
    // record the tip.    
    total_tips += tip_amount;
    llOwnerSay("tips so far: L$" + (string)total_tips + ".");
    show_label();  // refresh our text label.
}

handle_being_touched(integer num) {
    // make sure we've waited enough time before begging again.
    if (llAbs(llGetUnixTime() - last_begging_time) > BEGGING_INTERVAL) {
        llSay(0,
            "\nIt would be awesome if you could tip our hardworking widget gnomes.\n"
            + "If you right-click this object, you can 'Pay' it a tip, which will\n"
            + "go directly to the eepaw shop personnel.  Thanks much!");
        // update our last begging time so we don't beg too often.
        last_begging_time = llGetUnixTime();
    }
    // explode the textures just a little bit, if we have any on hand.
    integer indy;
    for (indy = 0; indy < llGetInventoryNumber(INVENTORY_TEXTURE); indy++) {
        llMakeExplosion(14, .4, 0.5, 4, 0.8,
            llGetInventoryName(INVENTORY_TEXTURE, indy), <0.0, 0.0, 0.0>);
    }
    // play the last sound that we happen to have, if any at all.
    llPlaySound(llGetInventoryName(INVENTORY_SOUND, llGetInventoryNumber(INVENTORY_SOUND) - 1), 1.0);
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
        initialize_begging_bowl();
    }

    touch_start(integer num) {
        handle_being_touched(num);
    }

    money(key id, integer payment) {
        tip_amount = payment;
        tippers_key = id;
        llRequestAgentData(id, DATA_NAME);
    }

    dataserver(key query, string name) { thank_tipper_and_give_gifts(name); }
}
