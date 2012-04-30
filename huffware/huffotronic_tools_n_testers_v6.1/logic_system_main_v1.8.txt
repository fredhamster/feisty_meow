
// logic system master script
// copyright john jamison, imagilearning?
// created by xyz
// modified by fred huffhines / chris koeritz.
//
//

// defaults...
vector TEXT_COLOR = <0.6, 0.84, 0.8>;  // label color.
float RADARTIME = 10.0;  // how frequently radar fires.
float RANGE = 50.0;  // range of the sensor sweeps.
integer MAX_AVATAR_AWOLS = 5;
    // number of times the avatar is allowed to be missed by sensor.
    // we have seen some unreliability of sensors, so a single miss is not a definite problem.
list RESET = ["Restart", "CANCEL"];  // used in a menu someplace?
string Choicetimer = "2";  // how long they get to make their choice?

// global variables...
integer current_awols;
key user = NULL_KEY;
key lquery = NULL_KEY;
list dialog;
string CHOICE1;
string CHOICE2;
string CHOICE3;
string CHOICE4;
string CHOICE5;
string CHOICE6;
string CHOICE7;
string CHOICE8;
string CHOICE1TEXT;
string CHOICE2TEXT;
string CHOICE3TEXT;
string CHOICE4TEXT;
string CHOICE5TEXT;
string CHOICE6TEXT;
string CHOICE7TEXT;
string CHOICE8TEXT;
integer CHOICE1POINTS;
integer CHOICE2POINTS;
integer CHOICE3POINTS;
integer CHOICE4POINTS;
integer CHOICE5POINTS;
integer CHOICE6POINTS;
integer CHOICE7POINTS;
integer CHOICE8POINTS;
integer TotalScore;
string DIALOG = " ";
string CurrentNoteCard;
string resettext = " ";
list CHOICE;
integer line = 0;
integer eof = 0;
integer CHANNEL;

initialize()
{
    CHOICE = [ CHOICE1TEXT, CHOICE2TEXT, CHOICE3TEXT, CHOICE4TEXT ];
    llSetText("[System Ready]", TEXT_COLOR, 1);
    eof = 0;
}

// leave the current setup and completely restart the script.
reboot(string reason)
{
llSay(0, "restarting now because " + reason);
    llResetScript();
}

// starts looking for our registered user.
start_sensor_sweeps()
{
    llSensorRemove();  // clear any existing sensor.
//llSay(0, "sweeping for " + user + " aka " + llKey2Name(user));
    llSensorRepeat("", user, AGENT, RANGE, 2 * PI, RADARTIME);  // look for our customer.
}

// what this did before made no sense...
// but now it serves to reset the device if the avatar wandered off.
handle_sensor(integer numDetected)
{
    integer proceed = FALSE;
    integer i;
    for (i = 0; i < numDetected; i++) {
        if (llDetectedKey(i) == user) proceed = TRUE;
    }
    if (proceed) return;  // keep running like we were.
    else reboot("we no longer saw our user here.");  // reboot, since we don't see our user.
}

// processes commands coming from other scripts or other parts of the object.
handle_link_message(string str, key id)
{
    if (str == "user") {
        user = id;
        llSetText ("System in use by " + llKey2Name (id), TEXT_COLOR, 1);
        CurrentNoteCard = llGetObjectDesc();
        CHANNEL = llRound (llFrand (-394304));
        llListen (CHANNEL, "", NULL_KEY, "");
        start_sensor_sweeps();
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
}

// handles notecard data as it is read.
process_incoming_data(key query_id, string data)
{
    list split_data = llParseString2List (data, [":"], []);
    string cmd = llToLower(llList2String(split_data, 0));

//hmmm: terribly repetitive code here.  clean up, shorten.
    if (cmd == "choicetimer") {
        Choicetimer = llList2String (split_data, 1);
    }
    if (cmd == "chat") {
        llSay(0, llList2String (split_data, 1));
        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (cmd == "whisper") {
        llWhisper(0, llList2String (split_data, 1));
        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (cmd == "givenotecard") {
        llGiveInventory (user, llList2String (split_data, 1));
        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (cmd == "channelsay") {
        llSay((integer) llList2String (split_data, 1), llList2String (split_data, 2));
        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (cmd == "channelshout") {
        llShout ((integer) llList2String (split_data, 1),
                 llList2String (split_data, 2));
        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (cmd == "channelwhisper") {
        llSay((integer) llList2String (split_data, 1), llList2String (split_data, 2));
        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (cmd == "resettext") {
        resettext = llList2String (split_data, 1);

    }
    if (cmd == "end") {
///        CurrentNoteCard = llGetObjectDesc();
///        line = 0;
        reboot("notecard said to end processing.");
    }

    if (cmd == "giveobject") {
        llGiveInventory (user, llList2String (split_data, 1));
        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
///TALK TO VB AS THIS NEXT PART NEEDS A MEDIA RELAY FOR IT TO WORK
    if (cmd == "videourl") {
        if (llList2String (split_data, 3) != "") {
            llParcelMediaCommandList ([PARCEL_MEDIA_COMMAND_URL,
               llList2String (split_data, 1) + ":" +
               llList2String (split_data, 2) + ":" +
               llList2String (split_data, 3) +
               llList2String (split_data, 4)]);
            line++;
            lquery = llGetNotecardLine (CurrentNoteCard, line);
        }
        else {
            llParcelMediaCommandList ([PARCEL_MEDIA_COMMAND_URL,
                llList2String (split_data, 1) + ":" +
               llList2String (split_data, 2)]);
            line++;
            lquery = llGetNotecardLine (CurrentNoteCard, line);
        }
        //llSay(-840,llList2String(dat,1));
        //llSay(-848,llList2String(dat,1));
    }
    if (cmd == "mediatexture") {
        llParcelMediaCommandList ([PARCEL_MEDIA_COMMAND_TEXTURE,
                                   (key) llList2String (split_data, 1)]);
        //llSay(-849,llList2String(dat,1));
        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (cmd == "sleep") {
        llSleep (llList2Float (split_data, 1));
        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (cmd == "audiourl") {
llSay(0, "got to audio url");
        if (llList2String (split_data, 3) != "") {
            llSetParcelMusicURL (llList2String (split_data, 1) + ":" +
                llList2String (split_data, 2) + ":" +
                llList2String (split_data, 3) + llList2String (split_data, 4));
            line++;
            lquery = llGetNotecardLine (CurrentNoteCard, line);
        }

        else {
            llSetParcelMusicURL (llList2String (split_data, 1) + ":" +
                llList2String (split_data, 2));
            line++;
            lquery = llGetNotecardLine (CurrentNoteCard, line);
        }
        //llSay(-840,llList2String(dat,1));
    }
    //THIS IS OPTION A//
    if (cmd == "weburl") {
        llLoadURL (user, llList2String (split_data, 1),
                   llList2String (split_data, 2) + llList2String (split_data,
                                                            3) +
                   llList2String (split_data, 4));

        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }


    if (cmd == "dialog") {
        DIALOG = llList2String (split_data, 1);
        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (cmd == "choice1") {
        CHOICE1TEXT = llList2String (split_data, 1);
        CHOICE1 = llList2String (split_data, 2);
        dialog = dialog +[llList2String (split_data, 1)];

        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);

    }
    if (cmd == "choice2") {
        CHOICE2TEXT = llList2String (split_data, 1);
        CHOICE2 = llList2String (split_data, 2);
        dialog = dialog +[llList2String (split_data, 1)];
        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (cmd == "choice3") {
        CHOICE3TEXT = llList2String (split_data, 1);
        CHOICE3 = llList2String (split_data, 2);
        dialog = dialog +[llList2String (split_data, 1)];
        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (cmd == "choice4") {
        CHOICE4TEXT = llList2String (split_data, 1);
        CHOICE4 = llList2String (split_data, 2);

        dialog = dialog +[llList2String (split_data, 1)];
        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (cmd == "choice5") {
        CHOICE5TEXT = llList2String (split_data, 1);
        CHOICE5 = llList2String (split_data, 2);
        dialog = dialog +[llList2String (split_data, 1)];

        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);

    }
    if (cmd == "choice6") {
        CHOICE6TEXT = llList2String (split_data, 1);
        CHOICE6 = llList2String (split_data, 2);
        dialog = dialog +[llList2String (split_data, 1)];

        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);

    }

    if (cmd == "choice7") {
        CHOICE7TEXT = llList2String (split_data, 1);
        CHOICE7 = llList2String (split_data, 2);
        dialog = dialog +[llList2String (split_data, 1)];

        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);

    }

    if (cmd == "choice8") {
        CHOICE8TEXT = llList2String (split_data, 1);
        CHOICE8 = llList2String (split_data, 2);
        dialog = dialog +[llList2String (split_data, 1)];

        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);

    }

    if (cmd == "choice1points") {
        CHOICE1POINTS = (integer) llList2String (split_data, 1);
        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);

    }

    if (cmd == "choice2points") {
        CHOICE2POINTS = (integer) llList2String (split_data, 1);
        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (cmd == "choice3points") {
        CHOICE3POINTS = (integer) llList2String (split_data, 1);
        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (cmd == "choice4points") {
        CHOICE4POINTS = (integer) llList2String (split_data, 1);
        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (cmd == "choice5points") {
        CHOICE5POINTS = (integer) llList2String (split_data, 1);
        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (cmd == "choice6points") {
        CHOICE6POINTS = (integer) llList2String (split_data, 1);
        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (cmd == "choice7points") {
        CHOICE7POINTS = (integer) llList2String (split_data, 1);
        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (cmd == "choice8points") {
        CHOICE8POINTS = (integer) llList2String (split_data, 1);
        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (cmd == "email") {
        llEmail (llList2String (split_data, 1),
                 llKey2Name (user) + " Scored " + (string) TotalScore +
                 " On " + llGetObjectDesc (),
                 llKey2Name (user) + " Scored " + (string) TotalScore +
                 " On " + llGetObjectDesc () + "Score As Of " +
                 CurrentNoteCard);

        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (cmd == "imuser") {
        llInstantMessage (user, llList2String (split_data, 1));
    }

    if (cmd == "im") {
        llInstantMessage (llList2String (split_data, 1), llList2String (split_data, 2));
    }
    if (cmd == "deadusertime") {
        RADARTIME = (integer) llList2String (split_data, 1);
    }

    if (cmd == "deadrange") {
        RANGE = (integer) llList2String (split_data, 1);
        start_sensor_sweeps();
    }

    if (cmd == "finalscore") {
        llSay (0,
               "You Scored " + (string) TotalScore + "/" +
               llList2String (split_data, 1));
    }
    if (cmd == "playsounduuid") {
        llPreloadSound (llList2String (split_data, 1));
        llPlaySound (llList2String (split_data, 1),
                     (integer) llList2String (split_data, 2));
    }

    if (cmd == "debug") {
        llSay (2147483647,
               "System Debug Info follows " + "/n " +
               (string) "key user = " + (string) user +
               (string) " key lquery = " + (string) lquery +
               (string) " integer DEADTIME = " + (string) RADARTIME +
               "string CHOICE1 = " + CHOICE1 + "string CHOICE2 = " +
               CHOICE2 + "string CHOICE3 = " + CHOICE3 +
               "string CHOICE4 = " + CHOICE4 + "string CHOICE1TEXT = " +
               CHOICE1TEXT + "string CHOICE2TEXT = " + CHOICE2TEXT +
               "string CHOICE3TEXT= " + CHOICE3TEXT +
               "string CHOICE4TEXT= " + CHOICE4TEXT +
               "integer CHOICE1POINTS" + (string) CHOICE1POINTS +
               "integer CHOICE2POINTS" + (string) CHOICE2POINTS +
               "integer CHOICE3POINTS" + (string) CHOICE3POINTS +
               "integer CHOICE4POINTS" + (string) CHOICE4POINTS +
               "integer Choicetimer = " + (string) Choicetimer +
               "integer TotalScore =" + (string) TotalScore +
               "string DIOALOGE = " + DIALOG +
               "string CurrentNoteCard = " + CurrentNoteCard +
               "string resettext = " + resettext +
               "list RESET = [Restart,CANCEL]" + "list CHOICE = " +
               (string) CHOICE + "integer line = " + (string) line);

    }

    if (data == EOF) {
        if (eof != 2 | eof == 1) {
            eof = 2;
            llSetTimerEvent ((float) Choicetimer);
        }
    } else {
        line++;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
}

// process what the device hears spoken nearby.
hear_voices(integer channel, string name, key id, string message)
{
//hmmm: bunkum.  do this as a list.
    if (message == CHOICE1TEXT) {
        CurrentNoteCard = CHOICE1;
        dialog =[];
        eof = 0;
        line = 0;
        TotalScore = TotalScore + CHOICE1POINTS;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (message == CHOICE2TEXT) {
        CurrentNoteCard = CHOICE2;
        dialog =[];
        eof = 0;
        line = 0;
        TotalScore = TotalScore + CHOICE2POINTS;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (message == CHOICE3TEXT) {
        CurrentNoteCard = CHOICE3;
        dialog =[];
        eof = 0;
        line = 0;
        TotalScore = TotalScore + CHOICE3POINTS;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (message == CHOICE4TEXT) {
        CurrentNoteCard = CHOICE4;
        dialog =[];
        eof = 0;
        line = 0;
        TotalScore = TotalScore + CHOICE4POINTS;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (message == CHOICE5TEXT) {
        CurrentNoteCard = CHOICE5;
        dialog =[];
        eof = 0;
        line = 0;
        TotalScore = TotalScore + CHOICE5POINTS;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (message == CHOICE6TEXT) {
        CurrentNoteCard = CHOICE6;
        dialog =[];
        eof = 0;
        line = 0;
        TotalScore = TotalScore + CHOICE6POINTS;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (message == CHOICE7TEXT) {
        CurrentNoteCard = CHOICE7;
        dialog =[];
        eof = 0;
        line = 0;
        TotalScore = TotalScore + CHOICE7POINTS;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (message == CHOICE8TEXT) {
        CurrentNoteCard = CHOICE8;
        dialog =[];
        eof = 0;
        line = 0;
        TotalScore = TotalScore + CHOICE8POINTS;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
    if (message == "Restart") {
        eof = 0;
        line = 0;
        lquery = llGetNotecardLine (CurrentNoteCard, line);
    }
}

//////////////

// ensure stops running if kept in updater object.
default
{
    state_entry() { if (llSubStringIndex(llGetObjectName(),  "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

// main state engine.
state real_default
{
    state_entry () {
        initialize();
    }

    on_rez (integer i) {
        reboot("object just rezzed.");
    }

    no_sensor () {
        current_awols++;
//llSay(0, "sensor missed avatar.");
        if (current_awols > MAX_AVATAR_AWOLS) {
            reboot("avatar was missed by sensor sweep too many times.");
        }
    }

    sensor (integer numDetected) {
        current_awols = 0;
//llSay(0, "sensor saw avatar.");
        handle_sensor(numDetected);
    }

    link_message (integer sender_num, integer num, string str, key id) {
        handle_link_message(str, id);
    }

    touch_start (integer total_number) {
        integer i;
        for (i = 0; i < total_number; i++) {
            if (llDetectedKey(i) == user) {
                llDialog (user, resettext, RESET, CHANNEL);
                return;
            }
        }
    }

    listen (integer channel, string name, key id, string message) {
        hear_voices(channel, name, id, message);
    }

    dataserver (key query_id, string data) {
        process_incoming_data(query_id, data);
    }

    collision_start(integer total_number)
    {
        if (user == NULL_KEY) {
//llSay(0, "sending startup link message.");
            handle_link_message("user", llDetectedKey(0));
        }
    }

    timer () {
        llDialog (user, DIALOG, dialog, CHANNEL);
        llSetTimerEvent (0);
    }
}
