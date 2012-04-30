
// huffware script: html onna prim, by fred huffhines.
//
// sets the land's media to a web site.
//
//   this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
//   do not use it in objects without fully realizing you are implicitly accepting that license.
//

integer initted = FALSE;

//////////////
// huffware script: auto-retire, by fred huffhines, version 1.9.
// distributed under BSD-like license.
//   partly based on the self-upgrading scripts from markov brodsky and jippen faddoul.
// the function auto_retire() should be added *inside* a version numbered script that
// you wish to give the capability of self-upgrading.
//   this script supports a notation for versions embedded in script names where a 'v'
// is followed by a number in the form "major.minor", e.g. "grunkle script by ted v8.2".
// when the containing script is dropped into an object with a different version, the
// most recent version eats any existing ones.
//   keep in mind that this code must be *copied* into your script you wish to add
// auto-retirement capability to.
//
// example usage of the auto-retirement script:
//
// default {
//    state_entry() {
//        auto_retire();  // make sure newest addition is only version of script.
//    }
// }
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
                if ((float)inv_version_string < (float)version_string) {
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
    if (llSubStringIndex(to_chop_up, " ") < 0) return [];  // no space found, not a valid name to work on.
        
    string basename = to_chop_up;  // script name with no version attached.
    
    integer posn;
    // minimum script name is 2 characters plus version.
    for (posn = llStringLength(to_chop_up) - 1;
        (posn >= 2) && (llGetSubString(to_chop_up, posn, posn) != " ");
        posn--) {
        // find the space.  do nothing else.
    }
    if (posn < 2) return [];  // no space found.
    string full_suffix = llGetSubString(to_chop_up, posn, -1);
    // ditch the space character for our numerical check.
    string chop_suffix = llGetSubString(full_suffix, 1, llStringLength(full_suffix) - 1);
    // strip out a 'v' if there is one.
    if (llGetSubString(chop_suffix, 0, 0) == "v")
        chop_suffix = llGetSubString(chop_suffix, 1, llStringLength(chop_suffix) - 1);
    // if valid floating point number and greater than zero, that works for our version.
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
    state_entry()
    {
        auto_retire();

    }
    
    on_rez(integer parm) {
        llResetScript();
    }
    
    touch_start(integer count) {
        if ( llParcelMediaQuery([PARCEL_MEDIA_COMMAND_TEXTURE]) == [] ) {
            llSay(0, "Lacking permission to set/query parcel media. This object has to be owned by/deeded to the land owner.");
            return;
        }
        if (!initted) {
            string texture_name = llGetInventoryName(INVENTORY_TEXTURE, 0);
            // sets the media texture to the first texture found in our inventory.
            llParcelMediaCommandList( [
                PARCEL_MEDIA_COMMAND_URL, "http://gruntose.com",
                PARCEL_MEDIA_COMMAND_TYPE, "text/html",
                PARCEL_MEDIA_COMMAND_TEXTURE, llGetInventoryKey(texture_name),
                PARCEL_MEDIA_COMMAND_PLAY ] );
            llSetTexture(texture_name, ALL_SIDES);
            initted = TRUE;
        }
        
        // zap it again.
        llParcelMediaCommandList([PARCEL_MEDIA_COMMAND_STOP]);
        llParcelMediaCommandList([PARCEL_MEDIA_COMMAND_PLAY]);
    }
}

