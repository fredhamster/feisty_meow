
// huffware script: freeview modified by fred huffhines (see original license below).
//
// my changes are licensed this way:
//   this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
//   do not use it in objects without fully realizing you are implicitly accepting that license.
//
// fred's changes include:
// + assorted tweaks that i have since forgotten the details about.
// + set the freeview to startup as a picture viewer, since that's my most common usage.

//////////////
// original author info and licensing:
//FreeView 1.2 WebGuide (revision 4) - By CrystalShard Foo
//Multifunctional Picture viewer and Video control script with webguide support
//This script is distributed for free and must stay that way. 
//      *** If you wish to give/sell a product using this script,  ***
//      ***     THEN THE SCRIPT MUST REMAIN FULL-PERM AND FREE.    ***
//      ***   Failure to do so will REVOKE your right to use it!   ***
//Help for using this script can be obtained at: http://www.slguide.com/help
//Feel free to modify this script and post your improvement. Leave the credits intact but feel free to add your name at its bottom.
//Whats new:
//- Now using FULL_BRIGHT instead of PRIM_MATERIAL_LIGHT for the screen display
//- Added an ownership-change code to handle cases where FreeView gets deeded to group post Video Init.
//- Renamed WebGuide to TV-Guide to reflect what this thing does better.
//- Added a 'Fix Scale' button to Picture mode to help against user texture-scale changes.
//- Additional minor help-tips and code improvements
//Enjoy!
//////////////


//Constants
integer PICTURE_ROTATION_TIMER = 30;   //In whole seconds

integer DISPLAY_ON_SIDE = ALL_SIDES; //Change this to change where the image will be displayed

key VIDEO_DEFAULT = "71b8ff26-087d-5f44-285b-d38df2e11a81";  //Test pattern - Used as default video texture when one is missing in parcel media
key BLANK = "5748decc-f629-461c-9a36-a35a221fe21f"; //Blank texture - Used when there are no textures to display in Picture mode
string NOTECARD = "bookmarks";  //Used to host URL bookmarks for video streams

integer VIDEO_BRIGHT = TRUE;    //FULL_BRIGHT status for Video
integer PICTURE_BRIGHT = TRUE;  //FULL_BRIGHT status for Picture

integer REMOTE_CHANNEL = 9238742;

integer EXTERNAL_TOUCH_CHANNEL = 1327;
    // used by other prims to tell the viewer prim that the avatar has clicked on them.

integer mode = 0;           //Freeview mode.
                            //Mode 0 - Power off
                            //Mode 1 - Picture viewer
                            //Mode 2 - Video

integer listenHandle = -1;      //Dialog menu listen handler
integer listenUrl = -1;         //listen handler for channel 1 for when a URL is being added
integer listenTimer = -1;       //Timer variable for removing all listeners after 2 minutes of listener inactivity
integer listenRemote = -1;      //listen handler for the remote during initial setup
integer encryption = 0;
integer numberofnotecardlines = 0;  //Stores the current number of detected notecard lines.
integer notecardline = 0;       //Current notecard line

integer loop_image = FALSE;     //Are we looping pictures with a timer? (picture mode)
integer current_texture = 0;    //Current texture number in inventory being displayed (picture mode)
integer chan;                   //llDialog listen channel
integer notecardcheck = 0;
key video_texture;              //Currently used video display texture for parcel media stream

string moviename;
string tempmoviename;
key notecardkey = NULL_KEY;
key tempuser;                   //Temp key storge variable
string tempurl;                 //Temp string storge variable

integer isGroup = TRUE;
key groupcheck = NULL_KEY;
key last_owner;
key XML_channel;

pictures()      //Change mode to Picture Viewer
{
    //Initilize variables
    
    //Change prim to Light material while coloring face 0 black to prevent light-lag generation.
    llSetPrimitiveParams([PRIM_BUMP_SHINY, DISPLAY_ON_SIDE, PRIM_SHINY_NONE, PRIM_BUMP_NONE, PRIM_COLOR, DISPLAY_ON_SIDE, <1,1,1>, 1.0, PRIM_MATERIAL, PRIM_MATERIAL_PLASTIC, PRIM_FULLBRIGHT, DISPLAY_ON_SIDE, PICTURE_BRIGHT]);

    integer check = llGetInventoryNumber(INVENTORY_TEXTURE);
     
    if(check == 0)
    {
        report("No pictures found.");
        llSetTexture(BLANK,DISPLAY_ON_SIDE);
        return;
    }
    else    
        if(current_texture > check)
            //Set to first texture if available
            current_texture = 0;
            
    display_texture(current_texture);
}

video()         //Change mode to Video
{
    //Change prim to Light material while coloring face 0 black to prevent light-lag generation.
    llSetPrimitiveParams([PRIM_BUMP_SHINY, DISPLAY_ON_SIDE, PRIM_SHINY_NONE, PRIM_BUMP_NONE, PRIM_COLOR, DISPLAY_ON_SIDE, <1,1,1>, 1.0, PRIM_MATERIAL, PRIM_MATERIAL_PLASTIC, PRIM_FULLBRIGHT, DISPLAY_ON_SIDE, VIDEO_BRIGHT, PRIM_TEXTURE, DISPLAY_ON_SIDE, "62dc73ca-265f-7ca0-0453-e2a6aa60bb6f", llGetTextureScale(DISPLAY_ON_SIDE), llGetTextureOffset(DISPLAY_ON_SIDE), llGetTextureRot(DISPLAY_ON_SIDE)]);
    
    report("Video mode"+moviename+": Stopped");
    if(finditem(NOTECARD) != -1)
        tempuser = llGetNumberOfNotecardLines(NOTECARD);
    video_texture = llList2Key(llParcelMediaQuery([PARCEL_MEDIA_COMMAND_TEXTURE]),0);
    if(video_texture == NULL_KEY)
    {
        video_texture = VIDEO_DEFAULT;
        llParcelMediaCommandList([PARCEL_MEDIA_COMMAND_TEXTURE,VIDEO_DEFAULT]);
        llSay(0,"No parcel media texture found. Setting texture to default: "+(string)VIDEO_DEFAULT);
        if(llGetLandOwnerAt(llGetPos()) != llGetOwner())
            llSay(0,"Error: Cannot modify parcel media settings. "+llGetObjectName()+" is not owned by parcel owner.");
    }
    
    llSetTexture(video_texture,DISPLAY_ON_SIDE);
}

off()
{
    report("Click to power on.");
    llSetPrimitiveParams([PRIM_BUMP_SHINY, DISPLAY_ON_SIDE, PRIM_SHINY_LOW, PRIM_BUMP_NONE, PRIM_COLOR, DISPLAY_ON_SIDE, <0.1,0.1,0.1>, 1.0,PRIM_MATERIAL, PRIM_MATERIAL_PLASTIC, PRIM_FULLBRIGHT, DISPLAY_ON_SIDE, FALSE, PRIM_TEXTURE, DISPLAY_ON_SIDE, BLANK, llGetTextureScale(DISPLAY_ON_SIDE), llGetTextureOffset(DISPLAY_ON_SIDE), llGetTextureRot(DISPLAY_ON_SIDE)]);
}

integer finditem(string name)   //Finds and returns an item's inventory number
{
    integer i;
    for(i=0;i<llGetInventoryNumber(INVENTORY_NOTECARD);i++)
        if(llGetInventoryName(INVENTORY_NOTECARD,i) == NOTECARD)
            return i;
    return -1;
}

seturl(string url, key id)  //Set parcel media URL
{
    if(mode != 2)
    {
        video();
        mode = 2;
    }
    moviename = tempmoviename;
    if(moviename)
        moviename = " ["+moviename+"]";
    tempmoviename = "";
    string oldurl = llList2String(llParcelMediaQuery([PARCEL_MEDIA_COMMAND_URL]),0);
    if(oldurl != "")
        llOwnerSay("Setting new media URL. The old URL was: "+oldurl);

    llParcelMediaCommandList([PARCEL_MEDIA_COMMAND_URL,url]);
    if(id!=NULL_KEY)
        menu(id);
    else
    {
        report("Video mode"+moviename+": Playing");
        llParcelMediaCommandList([PARCEL_MEDIA_COMMAND_PLAY]);
    }
       
    if(isGroup)
        llSay(0,"New media URL set.");
    else
        llOwnerSay("New media URL set: "+url);
}

string mediatype(string ext)    //Returns a string stating the filetype of a file based on file extension
{
    ext = llToLower(ext);
    if(ext == "swf")
        return "Flash";
    if(ext == "mov" || ext == "avi" || ext == "mpg" || ext == "mpeg" || ext == "smil")
        return "Video";
    if(ext == "jpg" || ext == "mpeg" || ext == "gif" || ext == "png" || ext == "pict" || ext == "tga" || ext == "tiff" || ext == "sgi" || ext == "bmp")
        return "Image";
    if(ext == "txt")
        return "Text";
    if(ext == "mp3" || ext == "wav")
        return "Audio";
    return "Unknown";
}

browse(key id)      //Image browser function for picture viewer mode
{
    integer check = llGetInventoryNumber(INVENTORY_TEXTURE);
    string header;
    if(check > 0)
        header = "("+(string)(current_texture+1)+"/"+(string)check+") "+llGetInventoryName(INVENTORY_TEXTURE,current_texture);
    else
        header = "No pictures found.";
    llDialog(id,"** Monitor Control **\n Picture Viewer mode\n- Image browser\n- "+header,["Back","Next","Menu"],chan);
    extendtimer();
}

report(string str)
{
    llSetObjectDesc(str);
}

extendtimer()       //Add another 2 minute to the Listen Removal timer (use when a Listen event is triggered)
{
    if(listenHandle == -1)
        listenHandle = llListen(chan,"","","");
    listenTimer = (integer)llGetTime() + 120;
    if(loop_image == FALSE)
        llSetTimerEvent(45);
}

config(key id)      //Configuration menu
{
    extendtimer();
    llDialog(id,"Current media URL:\n"+llList2String(llParcelMediaQuery([PARCEL_MEDIA_COMMAND_URL]),0)+"\nTip: If the picture is abit off, try 'Align ON'",["Set URL","Align ON","Align OFF","Menu","Set Remote"],chan);
}

tell_remote(string str)
{
    llShout(REMOTE_CHANNEL,llXorBase64Strings(llStringToBase64((string)encryption + str), llStringToBase64((string)encryption)));
}

menu(key id)        //Dialog menus for all 3 modes
{
    list buttons = [];
    string title = "** Monitor control **";
    
    extendtimer();

    if(mode != 0)
    {
        if(mode == 1)       //Pictures menu
        {
            title+="\n  Picture Viewer mode";
            buttons+=["Browse"];
            if(loop_image == FALSE)
                buttons+=["Loop"];
            else
                buttons+=["Unloop"];
            buttons+=["Video","Power off","Help","Fix scale"];
        }
        else                //Video menu
        {
            title+="\n Video display mode\n"+moviename+"\nTip:\nClick 'TV Guide' to view the Online bookmarks.";
            buttons+=["Pictures","Configure","Power off","Loop","Unload","Help","Play","Stop","Pause","TV Guide","Bookmarks","Set URL"];
        }
    }
    else
        buttons += ["Pictures","Video","Help"];
    
    llDialog(id,title,buttons,chan);
}

display_texture(integer check)  //Display texture and set name in description (picture mode)
{                               //"Check" holds the number of textures in contents. The function uses "current_texture" to display.
    string name = llGetInventoryName(INVENTORY_TEXTURE,current_texture);
    llSetTexture(name,DISPLAY_ON_SIDE);
    report("Showing picture: "+name+" ("+(string)(current_texture+1)+"/"+(string)check+")");
}
    

next()  //Change to next texture (picture mode)
{       //This function is used twice - by the menu and timer. Therefor, it is a dedicated function.
    current_texture++;
    integer check = llGetInventoryNumber(INVENTORY_TEXTURE);
    if(check == 0)
    {
        llSetTexture(BLANK,DISPLAY_ON_SIDE);
        current_texture = 0;
        report("No pictures found.");
        return;
    }
    if(check == current_texture)
        current_texture = 0;
    
    display_texture(check);
    return;
}

//////////////
// from hufflets...

// returns the index of the first occurrence of "pattern" inside
// the "full_string".  if it is not found, then a negative number is returned.
integer find_substring(string full_string, string pattern)
{ return llSubStringIndex(llToLower(full_string), llToLower(pattern)); }

// returns TRUE if the "prefix" string is the first part of "compare_with".
integer is_prefix(string compare_with, string prefix)
{ return find_substring(compare_with, prefix) == 0; }

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
        llListen(EXTERNAL_TOUCH_CHANNEL, "", NULL_KEY, "");
            // we listen on our touch channel in all cases and in all states.  this allows us
            // to always pass along the user's touch from other prims to run the menus.
        chan = (integer)llFrand(1000) + 1000;   //Pick a random listen channel for the listener
        if(PICTURE_ROTATION_TIMER <= 0)         //Ensure the value is no less or equal 0
            PICTURE_ROTATION_TIMER = 1;
        llListenRemove(listenHandle);
        listenHandle = -1;
        last_owner = llGetOwner();
        groupcheck = llRequestAgentData(llGetOwner(),DATA_NAME);
        off();
        llOpenRemoteDataChannel();
        // fred's changes to start up in picture viewing looper.
        mode = 1;  // picture viewing.
        pictures();  // show the pictures.
        loop_image = TRUE;
        llSetTimerEvent(PICTURE_ROTATION_TIMER);  // keep showing new pics.
    }
    
    on_rez(integer i)
    {
        llSay(0,"Welcome to FreeView - your free, open-source television!");
        llResetScript();
    }

    touch_start(integer total_number)
    {
        //-------------------------------------------------------------------------------
        //Listen only to owner or group member. Edit this code to change access controls.
        if(llDetectedKey(0) != llGetOwner() && llDetectedGroup(0) == FALSE)
            return;
        //-------------------------------------------------------------------------------

        menu(llDetectedKey(0));
    }
    
    changed(integer change)
    {
        if(change == CHANGED_INVENTORY) //If inventory change
        {
            if(mode == 1)   //If picture mode
            {
                integer check = llGetInventoryNumber(INVENTORY_TEXTURE);
                if(check != 0)
                {
                    current_texture = 0;
                    display_texture(check);
                }
                else
                {
                    llSetTexture(BLANK,DISPLAY_ON_SIDE);
                    report("No pictures found.");
                }
            }
            else
                if(mode == 2)   //If video mode
                    if(finditem(NOTECARD) != -1)    //And bookmarks notecard present
                        if(notecardkey != llGetInventoryKey(NOTECARD))
                            tempuser = llGetNumberOfNotecardLines(NOTECARD);    //Reload number of lines
        }
        else if(change == CHANGED_OWNER)
        {
            isGroup = TRUE;
            last_owner = llGetOwner();
            groupcheck = llRequestAgentData(llGetOwner(),DATA_NAME);
            
            if(mode == 2)
            {
                llSay(0,"Detected change in ownership. Attempting to obtain current parcel media texture...");
                video();
            }
        }
    }
    
    listen(integer channel, string name, key id, string message)
    {
        if ( (channel == EXTERNAL_TOUCH_CHANNEL) && is_prefix(message, "touched")) {
            // pretend we got touched by the av.
            message = llDeleteSubString(message, 0, 6);
            menu((key)message);
            return;
        }        
        if(message == "Pictures")
        {
            if(mode == 2)
                llParcelMediaCommandList([PARCEL_MEDIA_COMMAND_STOP]);
            pictures();
            mode = 1;
            menu(id);
            return;
        }
        if(message == "Video")
        {
            video();
            mode = 2;
            menu(id);
            return;
        }
        if(message == "Power off")
        {
            if(mode == 2)
                llParcelMediaCommandList([PARCEL_MEDIA_COMMAND_UNLOAD]);
            off();
            mode = 0;
            return;
        }
        if(message == "Help")
        {
            llSay(0,"Help documentation is available at: http://www.slguide.com/help");
            if(isGroup)
            {
                if(id == NULL_KEY)
                {
                    llSay(0,"FreeView cannot load help pages while set to group without the remote.");
                    llSay(0,"For further assistance, please consult: http://slguide.com/help");
                }
                else
                    tell_remote("HELP"+(string)id+(string)XML_channel);
            }
            else
                llLoadURL(id,"Help pages for FreeView","http://www.slguide.com?c="+(string)XML_channel+"&help=1");
        }
        if(mode == 1)
        {
            if(message == "Browse")
            {
                loop_image = FALSE;
                browse(id);
                return;
            }
            if(message == "Next")
            {
                extendtimer();
                next();
                browse(id);
            }
            if(message == "Back")
            {
                extendtimer();
                current_texture--;
                integer check = llGetInventoryNumber(INVENTORY_TEXTURE);
                if(check == 0)
                {
                    llSetTexture(BLANK,DISPLAY_ON_SIDE);
                    current_texture = 0;
                    report("No pictures found.");
                    return;
                }
                if(current_texture < 0)
                    current_texture = check - 1;
                
                display_texture(check);
                
                browse(id);
                return;
            }
            if(message == "Menu")
            {
                menu(id);
                return;
            }
            if(message == "Loop")
            {
                llSetTimerEvent(PICTURE_ROTATION_TIMER);
                loop_image = TRUE;
                llOwnerSay("Picture will change every "+(string)PICTURE_ROTATION_TIMER+" seconds.");
                return;
            }
            if(message == "Unloop")
            {
                loop_image = FALSE;
                llOwnerSay("Picture loop disabled.");
                return;
            }
            if(message == "Fix scale")
            {
                llSay(0,"Setting display texture to 1,1 repeats and 0,0 offset.");
                llScaleTexture(1, 1, DISPLAY_ON_SIDE);
                llOffsetTexture(0, 0, DISPLAY_ON_SIDE);
                return;
            }
        }
        if(mode == 2)
        {
            if(channel == REMOTE_CHANNEL)
            {
                if(encryption == 0)
                    encryption = (integer)message;
                llListenRemove(listenRemote);
                listenRemote = -1;
                llSay(0,"Remote configured ("+(string)id+")");
            }
                
            if(message == "TV Guide")
            {
                if(isGroup)
                {
                    if(!encryption)
                    {
                        llSay(0,"** Error - This FreeView object has been deeded to group. You must use a Remote control to open the TV Guide.");
                        llSay(0,"You can set up the remote control from the Video -> Configuration menu. Please refer to the notecard for further assistance.");
                        return;
                    }
                    tell_remote((string)id+(string)XML_channel+(string)llGetOwner());
                }
                else
                    llLoadURL(id, "Come to the Guide to Start Your Viewer Playing!", "http://slguide.com/index.php?v=" + (string)llGetKey() + "&c=" + (string)XML_channel + "&o=" + (string)llGetOwner() + "&");
                return;
            }

            string header = "Video mode"+moviename+": ";
            
            if(message == "<< Prev")
            {
                notecardline--;
                if(notecardline < 0)
                    notecardline = numberofnotecardlines - 1;
                tempuser = id;
                llGetNotecardLine(NOTECARD,notecardline);
                return;
            }
            if(message == "Next >>")
            {
                notecardline++;
                if(notecardline >= numberofnotecardlines)
                    notecardline = 0;
                tempuser = id;
                llGetNotecardLine(NOTECARD,notecardline);
                return;
            }
            if(message == "Use")
            {
                if(tempurl == "** No URL specified! **")
                    tempurl = "";
                seturl(tempurl,id);
                return;
            }
                    
            if(message == "Menu")
            {
                menu(id);
                return;
            }
            if(message == "Configure")
            {
                config(id);
                return;
            }
            if(message == "Bookmarks")
            {
                if(notecardcheck != -1)
                {
                    llDialog(id,"Error: No valid bookmark data found in notecard '"+NOTECARD+"'.",["Menu"],chan);
                    return;
                }
                if(finditem(NOTECARD) != -1)                
                {
                    tempuser = id;
                    if(numberofnotecardlines < notecardline)
                        notecardline = 0;
                    llGetNotecardLine(NOTECARD,notecardline);
                }
                else
                    llDialog(id,"Error: No notecard named "+NOTECARD+" found in contents.",["Menu"],chan);
                return;
            }
            
            if(llGetLandOwnerAt(llGetPos()) != llGetOwner())    //If we do not have permissions to actually do the following functions
            {
                llSay(0,"Error: Cannot modify parcel media settings. "+llGetObjectName()+" is not owned by parcel owner.");
                menu(id);
                return; //Abort
            }
            
            if(listenUrl != -1 && channel == 1) //Incoming data from "Set URL" command (user spoke on channel 1)
            {
                llListenRemove(listenUrl);
                listenUrl = -1;
                tempmoviename = "";
                seturl(message,id);
            }
            if(message == "Play")
            {
                report(header+"Playing");
                llParcelMediaCommandList([PARCEL_MEDIA_COMMAND_PLAY]);
                return;
            }
            if(message == "Stop")
            {
                report(header+"Stopped");
                llParcelMediaCommandList([PARCEL_MEDIA_COMMAND_STOP]);
                return;
            }
            if(message == "Pause")
            {
                report(header+"Paused");
                llParcelMediaCommandList([PARCEL_MEDIA_COMMAND_PAUSE]);
                return;
            }
            if(message == "Unload")
            {
                report(header+"Stopped");
                llParcelMediaCommandList([PARCEL_MEDIA_COMMAND_UNLOAD]);
                return;
            }
            if(message == "Loop")
            {
                llParcelMediaCommandList([PARCEL_MEDIA_COMMAND_LOOP]);
                return;
            }
            //URL , Auto-Scale, 
            if(message == "Set URL")
            {
                report(header+"Stopped");
                listenUrl = llListen(1,"",id,"");
                llDialog(id,"Please type the URL of your choice with /1 in thebegining. For example, /1 www.google.com",["Ok"],938);
                return;
            }
            if(message == "Align ON")
            {
                report(header+"Stopped");
                llParcelMediaCommandList([PARCEL_MEDIA_COMMAND_AUTO_ALIGN,TRUE]);
                menu(id);
                return;
            }
            if(message == "Align OFF")
            {
                report(header+"Stopped");
                llParcelMediaCommandList([PARCEL_MEDIA_COMMAND_AUTO_ALIGN,FALSE]);
                menu(id);
                return;
            }
            if(message == "Set Remote")
            {
                llSay(0,"Configuring remote...");
                encryption = 0;
                llListenRemove(listenRemote);
                listenRemote = llListen(REMOTE_CHANNEL,"","","");
                llSay(REMOTE_CHANNEL,"SETUP");
            }
        }
    }
    
    dataserver(key queryid, string data)
    {
        if(queryid == groupcheck)       //Test if object is deeded to group
        {
            groupcheck = NULL_KEY;
            isGroup = FALSE;
            return;
        }
        
        if(queryid == tempuser) //If just checking number of notecard lines
        {
            numberofnotecardlines = (integer)data;
            notecardkey = llGetInventoryKey(NOTECARD);
            notecardcheck = 0;
            llGetNotecardLine(NOTECARD,notecardcheck);
            return;
        }
        if(notecardcheck != -1)
        {
            if(data != EOF)
            {
                if(data == "")
                {
                    notecardcheck++;
                    llGetNotecardLine(NOTECARD,notecardcheck);
                }
                else
                {
                    notecardcheck = -1;
                    return;
                }
            }
            else
                return;
        }

        if(data == "" && notecardline < numberofnotecardlines)    //If user just pressed "enter" in bookmarks, skip
        {
            notecardline++;
            llGetNotecardLine(NOTECARD,notecardline);
            return;
        }
        
        if(data == EOF)
        {
            notecardline = 0;
            llGetNotecardLine(NOTECARD,notecardline);
            return;
        }
        list parsed = llParseString2List(data,["|","| "," |"," | "],[]);    //Ensure no blank spaces before "http://".
        string name = llList2String(parsed,0);
        tempurl = llList2String(parsed,1);
        if(tempurl == "")
            tempurl = "** No URL specified! **";
            
        tempmoviename = name;
                
        llDialog(tempuser,"Bookmarks notecard ("+(string)(notecardline+1)+"/"+(string)numberofnotecardlines+")\n"+name+" ("+mediatype(llList2String(llParseString2List(tempurl,["."],[]),-1))+")\n"+tempurl,["<< Prev","Use","Next >>","Menu"],chan);
    }
    
    remote_data(integer type, key channel, key message_id, string sender, integer ival, string sval)
    {
        if (type == REMOTE_DATA_CHANNEL)
        {
            XML_channel = channel;
        } 
        else if(type == REMOTE_DATA_REQUEST)
        {
            list media_info = llParseString2List(sval, ["|"], []);
            tempmoviename = llList2String(media_info,0);
            seturl(llList2String(media_info,1),NULL_KEY);
            llRemoteDataReply(channel, message_id, sval, 1);
        }
    }
    
    timer()
    {
        if(llGetTime() > listenTimer)       //If listener time expired...
        {
            llListenRemove(listenHandle);   //Remove listeneres.
            llListenRemove(listenUrl);
            llListenRemove(listenRemote);
            listenHandle = -1;
            listenUrl = -1;
            listenRemote = -1;
            listenTimer = -1;
            if(loop_image == FALSE || mode != 1) //If we're not looping pictures or are in picture mode at all
                llSetTimerEvent(0.0);   //Remove timer
        }
        
        if(loop_image == TRUE && mode == 1) //If we're looping pictures and and we're in picture mode...
            next(); //Next picture
    }
}

