
// huffware script: vendor sales manager, by fred huffhines.
//
// handles the vending of one set of items which are contained inside the vendor
// object.  when the customer pays for it, they get a copy in a folder named after
// the object, and the owner gets an instant message describing the purchase.
//
// parts of this script were gratefully snagged from Ilse's Basic Vendor version 1.3, by Ilse Mannonen.
//
// fred's changes are licensed by:
//   this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
//   do not use it in objects without fully realizing you are implicitly accepting that license.
//

// usage notes:
//
// the config notecard is hopefully pretty self explanatory.
// here's an example config for a random gift vendor...
//   price=<18, 37, 0>
//   seller_name=gorp
//   thank_you=Thank you very much fer your purchase!
//   emails=foop@norgrufextorp.cog
//   single=random
//   text=Random Prize Machine\nPay the random price and get a random prize!
//
// here's an example config for a normal vendor (that gives out all of
// the contents to a customer on purchase)...
//   price=82
//   seller_name=gorp
//   thank_you=Thank you very much fer your purchase!
//   emails=foop@norgrufextorp.cog
//   text=Buy an anti-ape machine for your garage!
//
// the split notecard requires a set of lines in the format:
//   KEY|PERCENT
// for example: 0000000000bad|30 would give 30 percent of purchases to
// the bogus key.  be sure to find out the real keys involved.

// initial defaults; these need to be overridden by the notecard.
integer price = 9999;
string str_thank_you = "Default thank you message.  Thanks.";
string seller_name = "Frodo's Rings";
string emails = "";

// if we're allowed to randomly charge in a range, then this records our range.
integer lowest_price = 0;
integer highest_price = 0;

// variables for split notecard.
integer splitline = 0;
key splitrequestid;

// variables for config notecard.
integer configline = 0;
key configrequestid;

// keys and percentages that profits should be split with.
list splitkeys;
list splitpercents;

// this is set to true if the vendor should select a single item
// at random from the contents and give it to the customer.
integer single_random_give = FALSE;

list current_giving_list;  // if there is a transaction occurring, this has the contents to be given.

float REPRICING_PERIOD = 20.0;  // how frequently we redo the price of the object, if it's random.

// lets the potential customer know what is being sold.
describe_product()
{
    if (lowest_price != 0)
        llSay(0, "Cost is random and will be between L$" + (string)lowest_price
            + " and L$" + (string)highest_price + ".");
    else
        llSay(0, "Cost is L$" + (string)price + ".");
    if (single_random_give)
        llSay(0, "This vendor is in random grab-bag mode; purchasers will receive one random object from the contents.");
}

// process the customer's attempt to hand us their money.
take_their_moolah(key customer, integer amount)
{
    // check the amount they paid.
    if ( (amount == price) || ( (lowest_price != 0) && (amount > lowest_price) ) ) {
        integer worked = give_objects_appropriately(customer);
        if (!worked) {
            llSay(0, "Oh my, this is not good.  There were no objects left to give out.  We'll refund your money now.");
            llGiveMoney(customer, amount);
            return;
        }

        // thank the purchaser.
        llSay(0, str_thank_you);
        // IM the seller.
        llInstantMessage(llGetOwner(), "Yay! " + llKey2Name(customer)
            + " just paid me L$" + (string)amount
            + " in " + llGetRegionName() + ".");
        // do the split.
        integer n;
        for (n = 0; n < llGetListLength(splitkeys); n++) {
            float percentage = llList2Float(splitpercents, n) / 100.0;
            integer payout = llRound(price * percentage);
            key payee = llList2Key(splitkeys, n);
            llInstantMessage(llGetOwner(), "Paying L$" + (string)payout
                + " to key " + (string)payee + " in commission.");
            llGiveMoney(payee,payout);
            string text_percentage = (string)((integer)(percentage * 1000.0) / 10);
            llInstantMessage(payee, "Woot!  You have been paid a L$"
                + (string)payout
                + " (" + text_percentage + "%) commission for a sale in "
                + llGetRegionName()
                + "--customer name is " + llKey2Name(customer)
                + ".");
        }
        // send out email alerts if there are any recipients.
        list email_addrs = llParseString2List(emails, ["|"], []);
        for (n = 0; n < llGetListLength(email_addrs); n++) {
            string curr = llList2String(email_addrs, n);
            llEmail(curr, llGetObjectName() + " made a sale in " + llGetRegionName(),
                "Wheee, there was a sale in " + llGetRegionName() + " of these items "
                + dump_list(current_giving_list) + " to a customer named "
                + llKey2Name(customer) + ".");
        }
    } else {
        // oops, something stupid.  give it all back.
        llInstantMessage(customer, "I'm sorry, but you paid L$" + (string)amount + ", and this item costs only L$" + (string)price + ".  We'll refund your money right away.");
        llGiveMoney(customer, amount);
    }
    
    if (lowest_price != 0) {
        // reset the timer to fire soon and change the price.
        llSetTimerEvent(0.5);
    }
}
  
// hands out objects according to the configured process.
integer give_objects_appropriately(key customer)
{
    current_giving_list = [];  // clear out last gifts.
    if (!single_random_give) {
        // this mode is only for the normal non random picker.
        // if it's right, give everything but scripts and config notecards.
        integer m;
        for (m = 0; m < llGetInventoryNumber(INVENTORY_ALL); m++) {
            string itemname = llGetInventoryName(INVENTORY_ALL, m);
            if (llGetInventoryType(itemname) == INVENTORY_NOTECARD) {
                //only add notecards if they don't start with ~
                if (llGetSubString(itemname, 0, 0) != "~") {
                    current_giving_list += [itemname];
                }
            } else {
                if (llGetInventoryType(itemname) != INVENTORY_SCRIPT) {
                    current_giving_list += [itemname];
                }
            }
        }
    } else {
        // aha, the freaky random gift giving model.
        if (llGetInventoryNumber(INVENTORY_OBJECT) >= 1) {
            // we made sure we have something to give out.
            integer which = (integer)randomize_within_range(0, llGetInventoryNumber(INVENTORY_OBJECT) - 1, FALSE);
            if (which >= 0) {
                current_giving_list += [ llGetInventoryName(INVENTORY_OBJECT, which) ];
            }
        }
    }
    
//llOwnerSay("about to give out " + (string)current_giving_list);
    if (llGetListLength(current_giving_list) > 0) {
        if (single_random_give)
            llGiveInventory(customer, llList2String(current_giving_list, 0));
        else
            llGiveInventoryList(customer, seller_name + " - " + llGetObjectName(), current_giving_list);
        return TRUE;
    } else return FALSE;  // oops.
}

// a wrapper for setting the object's text, but handling carriage returns.
set_text(string object_label)
{
    // reset the label to a decorated version of object name if it was default.
    if (object_label == "default") object_label = llGetObjectName();
    integer indy;
    integer keep_going = TRUE;
    while (keep_going) {
        indy = find_substring(object_label, "\\n");
        if (indy < 0) {
            keep_going = FALSE;
        } else {
            object_label = llGetSubString(object_label, 0, indy - 1)
                + "\n" + llGetSubString(object_label, indy + 2, -1);
        }
    }
//log_it("setting text: " + object_label);
    llSetText(object_label, <0.4, 0.7, 0.95>, 1.0);
}

// eats config items that are read from the notecard.
process_configuration_items(string data)
{
    string token = llList2String(llParseString2List(data, ["="], []), 0);
    string value = llList2String(llParseString2List(data, ["="], []), 1);
    if (token == "price") {
        if (is_prefix(value, "<")) {
            // we have a random price range definition.
            vector ranger = (vector)value;
            lowest_price = (integer)ranger.x;
            highest_price = (integer)ranger.y;
            price = lowest_price;
            // we run a timer to keep the price fluctuating for the user.
            if (lowest_price != 0) {
                // zap the timer event really soon.
                llSetTimerEvent(0.5);
            }
        } else {
            // this is hopefully a simple price definition.
            price = (integer)value;
            lowest_price = 0;  // reset so no one gets wrong ideas.
        }
        if (price == 0) {
            llOwnerSay("Failure in setting price; object will be defective until config is fixed.");
        } else {
            llSetPayPrice(PAY_HIDE, [price, PAY_HIDE, PAY_HIDE, PAY_HIDE]);
            llMessageLinked(LINK_SET, price, "price", NULL_KEY);
            describe_product();
        }
    } else if (token == "thank_you") {
        str_thank_you = value;
    } else if (token == "seller_name") {
        seller_name = value;
    } else if (token == "emails") {
        emails = value;
    } else if (token == "single") {
        if (value == "random") {
            single_random_give = TRUE;
            describe_product();
        }
    } else if (token == "text") {
        set_text(value);
//        llSetText(value, <.4, .9, .7>, 1.0);        
    }
    configline++;
    configrequestid = llGetNotecardLine("~CONFIG", configline);
}

//////////////
// from hufflets...

// returns the index of the first occurrence of "pattern" inside
// the "full_string".  if it is not found, then a negative number is returned.
integer find_substring(string full_string, string pattern)
{ return llSubStringIndex(llToLower(full_string), llToLower(pattern)); }

// returns a printable form of the list.
string dump_list(list to_show)
{
    integer len = llGetListLength(to_show);
    integer i;
    string text;
    for (i = 0; i < len; i++) {
        string next_line = llList2String(to_show, i);
        if (find_substring(next_line, " ") >= 0) {
            // this guy has a space in it, so quote it.
            next_line = "'" + next_line + "'";
        }
        text += next_line;
        if (i < len - 1) text += " ";
    }
    return text;
}

// returns TRUE if the "prefix" string is the first part of "compare_with".
integer is_prefix(string compare_with, string prefix)
{ return find_substring(compare_with, prefix) == 0; }

// returns a number at most "maximum" and at least "minimum".
// if "allow_negative" is TRUE, then the return may be positive or negative.
float randomize_within_range(float minimum, float maximum, integer allow_negative)
{
    if (minimum > maximum) {
        // flip the two if they are reversed.
        float temp = minimum; minimum = maximum; maximum = temp;
    }
    float to_return = minimum + llFrand(maximum - minimum);
    if (allow_negative) {
        if (llFrand(1.0) < 0.5) to_return *= -1.0;
    }
    return to_return;
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

default {
    state_entry() { if (llSubStringIndex(llGetObjectName(), "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default
{
    state_entry()
    {
        auto_retire();  // make sure newest addition is only version of script.
        splitrequestid = llGetNotecardLine("~SPLIT", splitline);
        configrequestid = llGetNotecardLine("~CONFIG", configline);
        llSetPayPrice(PAY_HIDE, [price, PAY_HIDE, PAY_HIDE, PAY_HIDE]);
        llSetText("garp", <0, 0, 0>, 0);
    }
    
    on_rez(integer start_param) { llResetScript(); }
        
    timer() {
        llSetTimerEvent(0);
        price = (integer)randomize_within_range(lowest_price, highest_price, FALSE);
        llSetPayPrice(PAY_HIDE, [price, PAY_HIDE, PAY_HIDE, PAY_HIDE]);
        llSetTimerEvent(REPRICING_PERIOD);
    }    
            
    money(key customer, integer amount) { take_their_moolah(customer, amount); }

    // restart if something was added to the vendor.    
    changed(integer change) {
        if (change & CHANGED_INVENTORY) { llSleep(3.14159265358); llResetScript(); }
    }
    
    touch_start(integer total_number)
    {
        // give the first non-config notecard.
        integer n;
        for (n = 0; n < llGetInventoryNumber(INVENTORY_NOTECARD); n++) {
            if (llGetSubString(llGetInventoryName(INVENTORY_NOTECARD, n), 0, 0) != "~") {
                llGiveInventory(llDetectedKey(0), llGetInventoryName(INVENTORY_NOTECARD, n));
                return;
            }
        }
        describe_product();
    }    

    // read the two configuration notecards.
    dataserver(key queryid, string data)
    {
        if ( (queryid == splitrequestid) && (data != EOF) ) {
            list tmp = llParseString2List(data, ["|"], [""]);
            if (llGetListLength(tmp) > 1) {
                splitpercents += llList2Integer(tmp,1);
                splitkeys += llList2Key(tmp,0);
                if (splitline == 0) {
                    llRequestPermissions(llGetOwner(), PERMISSION_DEBIT);
                }
                splitline++;
                splitrequestid = llGetNotecardLine("~SPLIT",splitline);                
            }
        } else if ( (queryid == configrequestid) && (data != EOF) ) {
            process_configuration_items(data);
        }
    }
}

