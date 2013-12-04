
// jukebox script.  unknown provenance.
// modified for use in opensim by fred huffhines.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

float INTERVAL = 9.00;

integer IS_OPENSIM = TRUE;
    // this must be set to true to turn off the sound queuing.  opensim does not currently implement it.

float SOUND_OFFSET_PERIOD = 1.1;  // number of seconds to start the sound in advance.

integer LISTEN_CHAN = 2000;
integer SEND_CHAN = 2001;
float VOLUME = 1.0;

integer g_iSound;
integer total_tracks;
integer g_iListenCtrl = -1;
integer g_iPlaying;
integer g_iLinked;
integer g_iStop;
integer g_iPod;
string g_sLink;

// DEBUG
integer g_iWasLinked;
integer g_iFinished;

list song_names;
    // hold onto the names of all the songs so we don't need to keep
    // going back to inventory.

Initialize()
{
    llSetText(llGetObjectName(), <0, 100, 100>, 10);
    
    // reset listeners
    if ( g_iListenCtrl != -1 )
    {
        llListenRemove(g_iListenCtrl);
    }
    g_iListenCtrl = llListen(LISTEN_CHAN,"","","");
    g_iPlaying = 0;
    g_iLinked = 0;

    integer i;
    total_tracks = llGetInventoryNumber(INVENTORY_SOUND);
    for ( i = 0; i < total_tracks; i++ ) {
        string name = llGetInventoryName(INVENTORY_SOUND, i);
        song_names += [ name ];
        llPreloadSound(name);
    }
//llOwnerSay("got song names list: " + (string)song_names);
}



PlaySong()
{
    integer i;

    g_iPlaying = 1;    
    llWhisper(0, "Playing...");
    if (!IS_OPENSIM) {
        llSetSoundQueueing(TRUE);
    }
    llAdjustSoundVolume(1.0);
    llPlaySound(llList2String(song_names, 0), VOLUME);
    llPreloadSound(llList2String(song_names, 1));
    // set the timer interval for just before the track chunk ends.
    llSetTimerEvent(INTERVAL - SOUND_OFFSET_PERIOD);
    g_iSound = 1;
}


StopSong()
{
    g_iPlaying = 0;
    llWhisper(0, "Stopping...");
    llStopSound();
    llAdjustSoundVolume(0);
    llSetTimerEvent(0.0);
}


integer CheckLink()
{
    string sLink;
    
    sLink = llGetLinkName(1);
    g_sLink = sLink;
    if ( llGetSubString(sLink,0,6) == "Jukebox" )
    {
        return TRUE;
    }
    return FALSE;
}


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
        Initialize();
    }
    
    on_rez(integer start_param)
    {
        Initialize();
        if ( start_param )
        {
            g_iPod = start_param - 1;
            if ( g_iPod )
            {
                llRequestPermissions(llGetOwner(),PERMISSION_ATTACH);
            } else {
                // Tell the controller what the CD key is so it can link
                llWhisper(SEND_CHAN,"LINK " + (string)llGetKey());
            }
        }
    }
    
    changed(integer change)
    {
        if ( change == CHANGED_LINK )
        {
            if ( llGetLinkNumber() == 0 )
            {
                StopSong();
                llDie();
            } else {
                if ( g_iStop )
                {
                    llMessageLinked(1,llGetLinkNumber(),"UNLINK","");
                } else {
                    llMessageLinked(1,llGetLinkNumber(),"LINKID","");
                    g_iWasLinked = 1;
                }
            }
        }
    }
    
    attach(key id)
    {
        if ( id == NULL_KEY )
        {
            llDie();
        } else {
            PlaySong();
        }
    }
    
    run_time_permissions(integer perm)
    {
        if ( perm == PERMISSION_ATTACH )
        {
            llAttachToAvatar(ATTACH_LSHOULDER);
            llSetTexture("clear",ALL_SIDES);
        }
    }
    
    touch_start(integer total_number)
    {
        integer i;
        
        for ( i = 0; i < total_number; i++ )
        {
            if ( llDetectedKey(i) == llGetOwner() )
            {
//llWhisper(0,"DEBUG: g_iPlaying=" + (string)g_iPlaying);
//llWhisper(0,"DEBUG: Link=" + (string)llGetLinkNumber());
//llWhisper(0,"DEBUG: g_iWasLinked=" + (string)g_iWasLinked);
//llWhisper(0,"DEBUG: g_iFinished=" + (string)g_iFinished);
                if ( g_iPlaying ) StopSong();
                else PlaySong();
            }
        }
    }
    
    listen(integer channel, string name, key id, string message)
    {
        if ( message == "RESET" )
        {
            if ( llGetLinkNumber() == 0 )
            {
                llDie();
            } else {
                llMessageLinked(1,llGetLinkNumber(),"UNLINK","");
            }
        }
        
        if ( message == "STOP" )
        {
            if ( g_iPod )
            {
                StopSong();
                llDetachFromAvatar();
            }
        }
    }

    link_message(integer sender_num, integer num, string str, key id)
    {
        if ( str == "PLAY" )
        {
            if ( !g_iPlaying )
            {
                PlaySong();
            }
            return;
        }
        
        if ( str == "STOP" )
        {
            g_iStop = 1;
            StopSong();
            llMessageLinked(1,llGetLinkNumber(),"UNLINK","");
        }
        
        if ( str == "VOLUME" )
        {
            VOLUME = (float)num / 10.0;
            llAdjustSoundVolume(VOLUME);
        }
    }
    
    timer()
    {
        if ( g_iPlaying )
        {
//llWhisper(0, "playing song #" + (string)(g_iSound+1) + ": " +  llList2String(song_names, g_iSound));
            llPlaySound(llList2String(song_names, g_iSound),VOLUME);
            if ( g_iSound < (total_tracks - 1) )
            {
                llPreloadSound(llList2String(song_names, g_iSound + 1));
            }
            g_iSound++;
            if ( g_iSound >= total_tracks )
            {
                llSetTimerEvent(INTERVAL + 5.0);
                g_iPlaying = 0;
            } else {
                llSetTimerEvent(INTERVAL - SOUND_OFFSET_PERIOD);
            }                
        } else {
            if ( llGetLinkNumber() != 0 )
            {
                llSetTimerEvent(0.0);
                if ( g_iPod )
                {
                    llWhisper(SEND_CHAN,"FINISH");
                    llDetachFromAvatar();
                } else {
                    llMessageLinked(1,0,"FINISH","");
                    g_iFinished = 1;
                }
            }
        }
    }
}
