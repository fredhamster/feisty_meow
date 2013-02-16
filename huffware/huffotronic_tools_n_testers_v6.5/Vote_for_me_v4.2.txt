
// huffware script: vote for me, by fred huffhines.
//
// original author: ChuChu Ricardo January 14, 2010
// numerous mods: Fred Huffhines, March 2010  [including but not limited to refactoring
//    existing functions and fixing payment behavior]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//   this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
//   do not use it in objects without fully realizing you are implicitly accepting that license.
//

// global constants...

string LINDEN_MARK = "L$";  // the money symbol for linden dollars.
integer MINIMUM_FREE_MEMORY = 3400;  // the least memory we can get by with.

// a set of menu items we'll be displaying on the voter menu.
string ITEM_VOTEME = "Vote for Me";
string ITEM_STATUS = "Show Info";
string ITEM_NOT_NOW = "Not Now";
// items for the owner menu only.
string ITEM_RESET = "Reset";
string ITEM_GIVE_NOTE = "Give Note";
string ITEM_NO_NOTE = "No Notecard";
string ITEM_GIVE_OBJ = "Give Gift";
string ITEM_NO_OBJ = "No Gift";

integer ONE_MINUTE;  // number of seconds in a minute.  initialized later.
integer ONE_HOUR;  // number of seconds in an hour.  initialized later.
integer FULL_DAY;  // number of seconds in a day.  initialized later.

// these are the textures we'll show when it's time to register the average vote.
list TEXTURE_UUIDS = [
    "8dfc709a-c37d-6103-2670-d287e871f0e7",  // zero average.
    "14278369-1db4-4497-13ef-5dc75a1017f0",  // lowest rating (red, many bars).
    "8a891c33-1070-6c6b-148b-919651323b5e",
    "25f8618d-181d-9af3-720e-01a3a0fa1861",
    "3f89b392-b70a-8a63-f09f-b51f6d91b437",
    "a6d02c4e-7b8c-0fd9-47a5-08cb61159bfa",
    "b7fb64db-5658-547d-560d-3ca86ff77e37",
    "bc8057e8-61f2-1919-74f9-59ea5b731bc7",
    "ec54863f-665c-9dc0-ab64-051fe96750af",
    "2acaa661-db7a-1035-d4e4-0ed50304887c",
    "53f1465d-7f39-d1cc-6ade-4610ec053646",
    "c868135f-0ad1-20ea-a85b-0c48a780ea7f",
    "78215a59-c52b-1886-0cae-bce11592e9b1",
    "ffa2ed36-df97-4889-4903-06dca758336d",
    "f1a625cb-4346-91b2-8e48-9290be22ec90",
    "6301df4e-d3bf-2b41-9738-a6e4b7f10715",
    "a3f566fa-0e2e-68dc-92ac-a60258717525",
    "83088c7a-8397-404d-1285-54def3ff179e",
    "cde91186-7365-02c1-fe79-740cc82bc01a",
    "08d22575-0848-b7c4-1dfb-665216165831"  // highest rating (green, no bars)
];

// the secret channel where the user can change the text shown on title.
integer COMMAND_CHANNEL = 9;

// global variables...

key last_owner;  // the last person who owned this object.
string persistent_message;  // additional text for the title bar.
integer voter_dialog_channel;  // where the voter (non-owner) menu will be heard.
integer owner_dialog_channel;  // where the owner's big menu will be heard.
integer vote_price = 10;  // amount of lindens paid per vote, with default value.
integer give_notecards = TRUE;  // should we hand out notecards for a vote?
integer give_objects = TRUE;  // should we hand out objects for a vote?

// these four variables need to reset daily...
list voter_ids;  // the identities of the voters.
list vote_magnitudes;  // the value (on the likes-me scale) of each vote cast.
float average_score;  // Average score computed so far.
integer last_reset_time;  // the last time we did a vote reset.

// perform initialization tasks.
setup_voteme()
{
    // initialize our "constants" here, since LSL doesn't like generated constants.
    ONE_MINUTE = 60;
    ONE_HOUR = 60 * ONE_MINUTE;
    FULL_DAY = 24 * ONE_HOUR;

    last_owner = llGetOwner();  // remember so we can know when changing hands.
    last_reset_time = llGetUnixTime();  // track when we cranked up.

    // default add-in for titler is the avatar's name.
    persistent_message = llKey2Name(llGetOwner());

    llSetText("VoteME: "+ persistent_message, <1,1,1>, 1.0);

    set_faces_to_texture((key)llList2String(TEXTURE_UUIDS, 0));

    llRequestPermissions(llGetOwner(), PERMISSION_DEBIT);

    // set up the menu dialog channel so we will hear what they picked.
    owner_dialog_channel = -(integer)(llFrand(999999) + 250);
    voter_dialog_channel = owner_dialog_channel + 1;
    llListen(owner_dialog_channel, "", llGetOwner(), "");
    llListen(voter_dialog_channel, "", NULL_KEY, "");

    // listen on our special command channel.
    llListen(COMMAND_CHANNEL, "", llGetOwner(), "");

    llSetTimerEvent(ONE_MINUTE);
}

// looks for the voter to see if they've already voted today.
// true is returned if the "id" is found.
integer seek_voter(key id) { return llListFindList(voter_ids, [id]) >= 0; }

// adds a record for the voter with key "id", if they are not already present.
// true is returned if this is a new voter.
integer add_voter(key id)
{
    if (llGetFreeMemory() < MINIMUM_FREE_MEMORY) {
        llOwnerSay("omitting vote due to low memory.");
        return FALSE;
    }
    // if the entry's not there already, add it.
    if (seek_voter(id)) return FALSE;  // that one was there.
    // if we got to here, then the voter was not there yet.
    voter_ids += [ id ];
    return TRUE;
}

// creates a string out of the time in "decode_time" which is expected to
// be compatible with llGetWallClock() values.
string time_string(integer decode_time)
{
    integer hours = decode_time / ONE_HOUR;
    decode_time -= hours * ONE_HOUR;
    integer minutes = decode_time / ONE_MINUTE;
    decode_time -= minutes * ONE_MINUTE;
    integer seconds = decode_time;
    return (string)hours + ":" + (string)minutes + ":" + (string)seconds;
}

// sets all the x/y faces of the object to the texture key specified.
// the top and bottom are unchanged.
set_faces_to_texture(key texture_id)
{
    integer i;
    for (i = 1; i <= 4; i++) {
        llSetTexture(texture_id, i);
    }
}

// pops up the menu for the owner's use.
show_owner_menu()
{
    string msg0 = "You can choose the amount you want voters to pay,\nor configure giving out notecards and/or gifts to your voters.";
    list menu0_btns = [
        // first row at bottom.
        ITEM_GIVE_NOTE, ITEM_NO_NOTE, ITEM_STATUS,
        ITEM_GIVE_OBJ, ITEM_NO_OBJ, ITEM_RESET,
        LINDEN_MARK + "10", LINDEN_MARK + "20", LINDEN_MARK + "50",
        // last row at top.
        LINDEN_MARK + "100"
    ];

    llDialog(llGetOwner(), msg0, menu0_btns, owner_dialog_channel);
}

// lets the avatar know we can't process their vote right now.
apologize_but_refuse_vote()
{
    llSay(0, "Sorry, You have already voted for " + llKey2Name(llGetOwner()) + " today."); 
}

key last_toucher;  // the last person that clicked the voter object.
//move this!

// handles a particular avatar that has touched the voter.
process_toucher(integer which)
{
    string voter_id = llDetectedKey(which);
    string voter_name = llDetectedName(which);
    // see if this was the owner touching the object.
    if (voter_id == llGetOwner()) {
        show_owner_menu();
        return;  // done with owner touch handling.
    }
    // handle when someone other than the owner touches the voter.    
    if (!seek_voter(voter_id)) {
        // new voter.
        if (last_toucher != voter_id) {
            llSay(0, "Hello " + voter_name + "... please Vote for Me.");
            last_toucher = voter_id;
            if (give_notecards == TRUE) {
                string card_name = llGetInventoryName(INVENTORY_NOTECARD, 0);
                if (card_name != "") llGiveInventory(voter_id, card_name);
            }
        }
        string msg1 = "Please Choose to Vote for me.\nYou can also choose to see my current voting status.";
        list menu1_btns = [ ITEM_VOTEME, ITEM_STATUS, ITEM_NOT_NOW ];
        llDialog(voter_id, msg1, menu1_btns, voter_dialog_channel);
    } else {
        // they had already voted, so we don't give them the whole menu.
        string msg1 = "Thanks for your vote!  You can check my status, but you cannot vote again until tomorrow.";
        list menu1_btns = [ ITEM_STATUS ];
        llDialog(voter_id, msg1, menu1_btns, voter_dialog_channel);
    }
}

// processes when an avatar touches the voter.
handle_touches(integer total_number)
{
    // run through all the people that touched the voter this time.
    integer i;
    for (i = 0; i < total_number; i++) { process_toucher(i); }
}

// deals with people throwing money at us.
handle_monetary_gift(key giver, integer amount)
{
    if (giver == llGetOwner()) {
        // fine, genius, you gave yourself some money.  you don't get to vote.
        return;
    }
    if (amount != vote_price) {
        // refund the amount paid, since this was not the right vote cost.
        llSay(0, "Oops!  That's not the right amount for the vote; SL is hosing up or something.  Here's a refund.");
        llGiveMoney(giver, amount);
        return;
    }
    if (seek_voter(giver)) {
        // they've already voted so they're not allowed again.
        apologize_but_refuse_vote();
        llGiveMoney(giver, amount);
        return;
    }
    add_voter(giver);
    
    llSay(0, "Thanks for the " + LINDEN_MARK + (string)amount + ", " + llKey2Name(giver)
        + ".  Your voting menu will open shortly.");

    string msg3 = "Voting Key:\n1-means you like me a little\n10-means you really adore me\nThank You!";
    list menu3_btns = [ "1", "2", "3", "4", "5", "6", "7", "8", "9", "10" ];
    llDialog(giver, msg3, menu3_btns, voter_dialog_channel);
}

// announces how the owner is doing so far.
show_status()
{
    integer count = llGetListLength(voter_ids);
    llSay(0, "My name is " + llKey2Name(llGetOwner()) + " and I have received "
        + (string)count + " votes.\n"
        + "My average score per vote is " + (string)average_score + ".");

integer since_reset = llGetUnixTime() - last_reset_time;
llOwnerSay("last reset " + time_string(since_reset) + " H:M:S ago.");

}

// processes responses from menus and deals with spoken commands.
hear_voices(integer channel, string speaker_name, key id, string message)
{
    // see if this is from the owner about what to show on the voter.
    if ( (channel == COMMAND_CHANNEL) && (id == llGetOwner()) ) {
//hmmm: undocumented!
        persistent_message = message;
        llOwnerSay("Changed title bar to add \"" + persistent_message + "\".");
        llSetText("VoteME: " + persistent_message, <1,1,1>, 1.0);
        return;
    }

    // check if the message comes from the owner's dialog.
    if (channel == owner_dialog_channel) {
        if (message == ITEM_STATUS) {
            show_status();
            return;
        } else if (message == ITEM_RESET) {
//hmmm: is this in the menu???
            llOwnerSay("Resetting now...");
            llResetScript();
        } else if (llGetSubString(message, 0, 1) == LINDEN_MARK) {
            vote_price = (integer)llGetSubString(message,2,-1);
            llOwnerSay("Voting for you will now cost avatars "
                + LINDEN_MARK + (string)vote_price + ".");
            llSetPayPrice(PAY_HIDE, [vote_price, PAY_HIDE, PAY_HIDE, PAY_HIDE]);
        } else if (message == ITEM_GIVE_OBJ) {
            llOwnerSay("Gift giving enabled.");
            give_objects = TRUE;
        } else if (message == ITEM_NO_OBJ) {
            llOwnerSay("Gift giving disabled.");
            give_objects = FALSE;
        } else if (message == ITEM_GIVE_NOTE) {
            llOwnerSay("Notecard giving enabled.");
            give_notecards = TRUE;
        } else if (message == ITEM_NO_NOTE) {
            llOwnerSay("Notecard giving disabled.");
            give_notecards = FALSE;
        }
        // assume for the choices that fall through that they want to see the menu again.
        show_owner_menu();
        return;  // done handling our own dialog.
    }

    // see if this is the dialog for the voting person.
    if (channel == voter_dialog_channel) {
        if (message == ITEM_VOTEME) {
            string msg2 = "To show your confidence in me, pay me "
                + LINDEN_MARK + (string)vote_price + ".\n"
                + "Then rate me from 1 to 10 after you pay.\n"
                + "1 = You like me a little bit\n10 = You really adore me\n"
                + "Ready?  Right-click this object and pick \"Pay\".  Thanks very much!";
            list menu2_btns = [ "Understood" ];
            llDialog(id, msg2, menu2_btns, voter_dialog_channel);
        } else if (message == ITEM_STATUS) {
            show_status();
        } else if (message == ITEM_NOT_NOW) {
            llSay(0, "Thanks for the interest anyhow, " + llKey2Name(id) + ".");
        } else if ((integer)message > 0 && (integer)message <= 10) {
            if (give_notecards == TRUE) {
                string name = llGetInventoryName(INVENTORY_NOTECARD, 0); 
                if (name != "") llGiveInventory(id, name);
            }
            if (give_objects == TRUE) {
               string name = llGetInventoryName(INVENTORY_OBJECT, 0); 
               if (name != "") llGiveInventory(id, name);
            }
            register_vote((integer)message);
        }
    }
}

// deals with our timer clicks.
handle_timer()
{
    integer wall_clock = (integer)llGetWallclock();

//llOwnerSay("tick: " +  time_string(wall_clock));

    // see if it seems like time to reset.  we'll do this when the clock has just passed
    // midnight (that is, we're in the first hour after midnight), or if the device had
    // possibly been taken out of commission a long time ago (meaning more time has passed
    // since the last reset than has elapsed today so far).
    integer reset_it = FALSE;
    integer time_since_reset = llGetUnixTime() - last_reset_time;
    // is it the first hour of midnight, and we didn't just reset?
    if ( (wall_clock < ONE_HOUR) && (time_since_reset > ONE_HOUR) ) reset_it = TRUE;
    // or, is it a really long time since we did reset?  longer than when the hour turned?
    if (time_since_reset > wall_clock) reset_it = TRUE;

    if (reset_it) {
        // aha, we've decided it's that time.
        llOwnerSay("Resetting voters and average, since the clock rolled over.");
        voter_ids = [];
        vote_magnitudes = [];
        average_score = 0;
        last_reset_time = llGetUnixTime();
        register_vote(0);  // just recalculate average and show texture.
    }
}

// tracks that they've made a vote.
register_vote(integer this_vote_value)
{
    // zero is a special value that just makes us recalculate the average.
    if (this_vote_value != 0) vote_magnitudes += [ this_vote_value ];

//move this to an averager method.
    integer sum = 0;
    integer n = llGetListLength(vote_magnitudes);
    integer i = 0;
    for(i = 0;i<n;i++) {
        sum = sum + (integer)llList2String(vote_magnitudes,i);
    }
    if (n != 0) average_score = (float)sum/(float)n;
    else average_score = 0;
        
    // Score on scale of 0 to 19
    float new_score = average_score * 2;
    integer tex_num = (integer)new_score;  //Ean's Texture Number

    if (tex_num >= 20) tex_num = 19;  //We dont have all green texture
        
    set_faces_to_texture((key)llList2String(TEXTURE_UUIDS, tex_num));
}

default {
    state_entry() { if (llSubStringIndex(llGetObjectName(), "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

// main state machine.
state real_default
{
    state_entry() { setup_voteme(); }

    on_rez(integer parm) {
        if (last_owner != llGetOwner()) {
            llOwnerSay("Now configuring for " + llKey2Name(llGetOwner()));
            llResetScript();
        }
    }
    
    run_time_permissions(integer perm) {
        if (perm & PERMISSION_DEBIT) {
//llOwnerSay("got permission for debits, will set price to " + (string)vote_price);
            llSetPayPrice(PAY_HIDE, [vote_price, PAY_HIDE, PAY_HIDE, PAY_HIDE]);
        } else {
            llOwnerSay("We were denied debit permission and cannot function without it.  Use the menu and select \"" + ITEM_RESET + "\" to fix this.");
        }
    }

    timer() { handle_timer(); }

    touch_start(integer total_number) { handle_touches(total_number); }

    money(key giver, integer amount) { handle_monetary_gift(giver, amount); }

    listen(integer channel, string name, key id, string message)
    { hear_voices(channel, name, id, message); }
}


//feature ideas:
// have the voter track the highest paying guy?
//   actually no, the owner sets that limit.  rats.
//   but what about the highest voting guy!?  the guy/gal who thought the most could be tracked.

//hmmm: need a helper notecard function for owner!

