//Multi Vendor by Adelle Fitzgerald
//Based on the Ama Omega Vendor Script v0.3.1 06/2004

 
//Licenced under Creative Commons Attribution-Share Alike 3.0 Unported - http://creativecommons.org/licenses/by-sa/3.0/


//You are free:

//    * to Share — to copy, distribute and transmit the work
//    * to Remix — to adapt the work

//Under the following conditions:

//    * Attribution — You must attribute the work in the manner specified by the author or licensor (but not in any way that suggests that they endorse you or your use of the work).

//    * Share Alike — If you alter, transform, or build upon this work, you may distribute the resulting work only under the same, similar or a compatible license.

//With the understanding that:

//    * Waiver — Any of the above conditions can be waived if you get permission from the copyright holder.
//    * Public Domain — Where the work or any of its elements is in the public domain under applicable law, that status is in no way affected by the license.
//    * Other Rights — In no way are any of the following rights affected by the license:
//          o Your fair dealing or fair use rights, or other applicable copyright exceptions and limitations;
//          o The author's moral rights;
//          o Rights other persons may have either in the work itself or in how the work is used, such as publicity or privacy rights.
//    * Notice — For any reuse or distribution, you must make clear to others the license terms of this work. The best way to do this is with a link to this web page.

//Disclaimer

//The Commons Deed is not a license. It is simply a handy reference for understanding the Legal Code (the full license) — it is a human-readable expression of some of its key terms. Think of it as the user-friendly interface to the Legal Code beneath. This Deed itself has no legal value, and its contents do not appear in the actual license.

//Creative Commons is not a law firm and does not provide legal services. Distributing of, displaying of, or linking to this Commons Deed does not create an attorney-client relationship.


integer gChannel;
integer handle;
string name = "Vendor ";
integer display;
list pics;
list desc;
list items;
list sold;
list info;
key gUser;
key k;
list temp;
integer line;
integer l;
string s;
string hText;
integer totItems;
string button;


incSold(integer i)
{
    sold = llListInsertList(llDeleteSubList(sold,i,i),[(integer)(llList2Integer(sold,i) + 1)],i);
}

doMenu()
{
    //llSay(0,(string)gChannel + " : " + (string)gUser); //say gChannel
    llSetTimerEvent(30);
    llDialog(gUser, "Settings:", ["Cancel", "Report", "Get Item", "Reset"], gChannel);
}

doCancel()
{
    llListenControl(handle, FALSE);
    llSetTimerEvent(0);
}

next()
{
    display += 1;
    if (display >= llGetListLength(items)) display = 0;
    llSetText("Item: " + (string)(display + 1) + " of " + (string)totItems + "\n" + llList2String(items,display) + "\n" + llList2String(desc,display),<1,1,1>,1);
    llMessageLinked(LINK_SET,1,"PIC",llGetInventoryKey((key)llList2String(pics,display)));
    llMessageLinked(LINK_SET,1,"LOADNEXT",llGetInventoryKey((key)llList2String(pics,display + 1)));
    llMessageLinked(LINK_SET,1,"LOADPREV",llGetInventoryKey((key)llList2String(pics,display - 1))); 
}

prev()
{
    display -= 1;
    if (display < 0) display = llGetListLength(items) - 1;
    llSetText("Item: " + (string)(display + 1) + " of " + (string)totItems + "\n" + llList2String(items,display) + "\n" + llList2String(desc,display),<1,1,1>,1);
    llMessageLinked(LINK_SET,1,"PIC",llGetInventoryKey((key)llList2String(pics,display)));
    llMessageLinked(LINK_SET,1,"LOADNEXT",llGetInventoryKey((key)llList2String(pics,display + 1)));
    llMessageLinked(LINK_SET,1,"LOADPREV",llGetInventoryKey((key)llList2String(pics,display - 1)));
}

init()
{
    display = 0;
    if (display < 0) display = llGetListLength(items) - 1;
    llSetText("Item: " + (string)(display + 1) + " of " + (string)totItems + "\n" + llList2String(items,display) + "\n" + llList2String(desc,display),<1,1,1>,1);
    llMessageLinked(LINK_SET,1,"PIC",llGetInventoryKey((key)llList2String(pics,display)));
    llMessageLinked(LINK_SET,1,"LOADNEXT",llGetInventoryKey((key)llList2String(pics,display + 1)));
    llMessageLinked(LINK_SET,1,"LOADPREV",llGetInventoryKey((key)llList2String(pics,display - 1)));
}


default {
    state_entry() { if (llSubStringIndex(llGetObjectName(), "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default {
    state_entry()
    {
        state
        readCard;
    }
}
    
state running
{    
    state_entry()
    {
        gChannel = (integer)llFrand(2147423647) + 10000;
        handle = llListen(gChannel, "", llGetOwner(), "");
        llListenControl(handle, FALSE);
        init();
    }
    
    on_rez(integer param)
    {
        llResetScript();
    }
    
    listen(integer chan, string name, key id, string mes)
    {
        if (id != llGetOwner()) {
            llWhisper(0, "Sorry, this vendor is restricted for use by its owner.");
            return;
        }
        if (mes == "Reset")
        {
            doCancel();
            state readCard;
        }
        else if (mes == "Report")
        {
            integer i;
            for(i=0;i<llGetListLength(sold);i++)
            {
                if (llList2Integer(sold,i) != 0) llWhisper(0,"* Sold " + (string)llList2Integer(sold,i) + " " + llList2String(items,i));
            }
            doCancel();
        }
        else if (mes == "Get Item")
        {
            llGiveInventory(gUser, llList2String(items,display));
            llWhisper(0,"Thank you, please enjoy your " + llList2String(items,display));
            doCancel();
        }
        else if (mes == "Cancel")
        {
            doCancel();
        }
    }
    
    touch_start(integer num_detected)
    {
        button = llGetLinkName(llDetectedLinkNumber(0));
        gUser = llDetectedKey(0);
        
        if (button == "Next")
        {
            next();
        }
        else if (button == "Prev")
        {
            prev();
        }
        else if (button == "Info")
        {
            llGiveInventory(gUser, llList2String(info,display));
        }
        else
        {
            if (gUser != llGetOwner()) {
                llWhisper(0, "Sorry, this vendor is restricted for use by its owner.");
                return;
            }
            if (gUser == llGetOwner())
            {
                llListenControl(handle, TRUE);
                doMenu();
            }
            else
            {
                incSold(display);
                llGiveInventory(gUser, llList2String(items,display));
                llWhisper(0,"Thank you, please enjoy your " + llList2String(items,display));
            }
        }
    }                   
    timer()
    {
        doCancel();
        llInstantMessage(gUser,"Menu timed out");
    }
}

state readCard
{
    state_entry()
    {
        llSay(0,"Initializing...");
        line = 1;
        k = llGetNotecardLine("VendorConfig",line++);
        items = [];
        pics = [];
        desc = [];
    }
    
    dataserver(key q, string data)
    {
        if (q == k)
        {
            if (data == "")
            {
                k = llGetNotecardLine("VendorConfig",line++);
                return;
            }
            temp = llCSV2List(data);
            if (llParseString2List(data,[" "],[]) == []);
            else if (data == EOF)
            {
                display = 0;
                totItems = llGetListLength(items);
                llMessageLinked(LINK_SET,1,"PIC",llGetInventoryKey(llList2String(pics,0)));
                llWhisper(0," * Vendor setup complete : " + (string)totItems + " items loaded.");
                state running;
            }
            else if (llGetListLength(temp) != 4)
            {
                llWhisper(0,"Error, improperly formatted line #" + (string)(line - 1));
                state running;
            }
            else
            {
                items += llList2String(temp,0);
                pics += llList2String(temp,1);
                desc += llList2String(temp,2);
                info += llList2String(temp,3);
                sold += [(integer)0];
                if ( llGetInventoryKey((key)llList2String(temp,0)) == NULL_KEY || llGetInventoryKey((key)llList2String(temp,1)) == NULL_KEY)
                {
                    llWhisper(0,"Error, missing inventory or picture from line : " + data);
                    state running;
                }
                k = llGetNotecardLine("VendorConfig",line++);
            }
        }
    }
}
