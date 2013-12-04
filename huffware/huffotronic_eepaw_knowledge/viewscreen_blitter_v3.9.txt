
// huffware script: viewscreen blitter, by fred huffhines.
//
// listens for web addresses to show and puts them on the screen of this prim.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

// important switches...

integer IS_OPENSIM = TRUE;
    // selects the major mode of operation for the viewscreen.

integer DEBUGGING = TRUE;
    // if this is set to true, more diagnostic info is printed than normal.

// global constants...

// API for the viewscreen blitter library...
//////////////
integer VIEWSCREEN_BLITTER_HUFFWARE_ID = 10027;
    // unique ID for the viewscreen services.
string HUFFWARE_PARM_SEPARATOR = "{~~~}";
    // this pattern is an uncommon thing to see in text, so we use it to separate
    // our commands in link messages.
string HUFFWARE_ITEM_SEPARATOR = "{|||}";
    // used to separate lists of items from each other when stored inside a parameter.
    // this allows lists to be passed as single string parameters if needed.
integer REPLY_DISTANCE = 100008;  // offset added to service's huffware id in reply IDs.
//
string SHOW_URL_COMMAND = "#shurl";
    // requests the viewscreen to show a particular URL as its texture.
string RESET_VIEWSCREEN_COMMAND = "#shrz";
    // resets the viewscreen script to the default state.
string SHOW_TEXTURE_COMMAND = "#shtex";
    // displays a texture on the prim.  the first parameter is the texture key or name.
//////////////

// configurable constants...

handle_view_request(string command, string parms)
{
    if (command == SHOW_URL_COMMAND) {
        // the parms is just the URL to display.
        if (DEBUGGING) {
            string to_show = "url is: ";
            integer i;
            for (i = 0; i < llStringLength(parms); i += 512) {
                // show the string a chunk at a time or we will be censored by chat limits.
                integer end = i + 511;
                if (i >= llStringLength(parms) - 1) i = llStringLength(parms) - 1;
                to_show += llGetSubString(parms, i, end);
                llSay(0,  to_show);
                to_show = "";
            }
        }
        if (!IS_OPENSIM) {
            // second life implementation...
            llParcelMediaCommandList( [ PARCEL_MEDIA_COMMAND_STOP,
                PARCEL_MEDIA_COMMAND_TYPE, "text/html",
                PARCEL_MEDIA_COMMAND_URL, parms ] );
            // we separate out the looping player call since we can't rely on the new media
            // setting being ready within the same media call.
            llParcelMediaCommandList( [ PARCEL_MEDIA_COMMAND_PLAY, PARCEL_MEDIA_COMMAND_LOOP ] );
        } else {
            // opensim implementation...
            string extra_parms = "";
            //bgcolor:white,alpha:false,width:1024,height:768,
            string url_texture_key = osSetDynamicTextureURL("", "image", parms, extra_parms, 5000);
//string url_texture_key ="";

//llSay(0, "dyn texture is " + url_texture_key);
            if (url_texture_key != "") {
                llSetTexture(url_texture_key, ALL_SIDES);
            }
        }
    } else if (command == RESET_VIEWSCREEN_COMMAND) {
        llResetScript();
    } else if (command == SHOW_TEXTURE_COMMAND) {
        llSetLinkTexture(LINK_THIS, parms, ALL_SIDES);
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

// end hufflets.
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
        llParcelMediaCommandList( [ PARCEL_MEDIA_COMMAND_STOP ] );
    }

    link_message(integer linknum, integer num, string cmd, key parms) {
        if (num != VIEWSCREEN_BLITTER_HUFFWARE_ID) return;  // not for us.
        handle_view_request(cmd, parms);
    }
}
