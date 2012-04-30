
// huffware script: hour hand, modified by fred huffhines.
// 
// expanded to support time zones by fred huffhines, december 2009.
//
// noticed this in arcadia asylum's great hobo cuckoo clock.
// this script is licensed by Beezle Warburton:
//    "I released those 'into the wild' years ago,do what you like,
//     I just ask that people don't resell the scripts by themselves."
//
// fred's changes licensed by:
//   this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
//   do not use it in objects without fully realizing you are implicitly accepting that license.

// Optional second hand will be a much simpler script that will be
// unrelated to real-time.
//
// note - to reduce load, this script only calculates once a minute, so
// this provides a jump style minute hand, rather than a smoothy rotating one.
// This (and rounding) makes the clock's accuracy +/- 1 minute.
// It's a prim clock anyways, so digital accuracy is just excessive.
//
// It would probably be more elegant to use link messages rather than each
// hand having it's own script, however, this method allows for less caution
// when linking/unlinking parts of the clock.

// To not mess up timekeeping, please keep chime sounds under 4 seconds long.

integer time;
integer hours;
integer minutes;

float anglehours;
float angleminutes;

integer chime;
integer numrings;

string chimesound = "CuckooClock";
float chimevolume = 1.0;

// this is what might be adjusted.
integer LOCAL_TIME_ZONE_OFFSET = -5;  //-5 is EST, -8 is PST.

// the code used in linked messages for us.
integer SECRET_TIME_ZONE_ADJUSTER_ID = -2091823;
string TIME_ZONE_ADJUST_COMMAND = "tzadjust";

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
        // we want to calculate once a minute.
        llSetTimerEvent(60.0);
    }
    timer() {
        llSetTimerEvent(0);
        time = (integer)llGetGMTclock(); // seconds since midnight
        // adjust the hours for the time zone.  this will work for most time zones that
        // go through daylight saving time, like the pacific zone where SL is rooted.
        hours = (time / 3600) + LOCAL_TIME_ZONE_OFFSET;
        // convert from 24 to 12 hour time
        if (hours > 11) {
            hours = hours - 12;
        } else if (hours < 0) {
            hours = hours + 12;
        }
        minutes = (time % 3600) / 60;
        
        numrings = (integer)hours;
        if (numrings == 0) {
            numrings = 12;
        }
        
        // the extra addition here makes a smooth hour hand
        // for example, at 6:30, the hour hand should actually
        // be halfway between 6 and 7 on the clock dial
        
        anglehours = - (((float)hours / 12) + (((float)minutes / 720))) * TWO_PI;
//incompatible with opensim:        llSetSoundQueueing(TRUE);
        llRotateTexture(anglehours, ALL_SIDES);
        if (minutes == 0) {
            llMessageLinked(-1,numrings,"chime",NULL_KEY);
            //for (chime = 0; chime < numrings; chime++) {
            //      llPlaySound(chimesound, chimevolume);
            //      llSleep(3.0);    
            //}            
        }
        llSetTimerEvent(60.0);
    }
    
    link_message(integer sender, integer num, string msg, key id) {
        if ( (num != SECRET_TIME_ZONE_ADJUSTER_ID) || (msg != TIME_ZONE_ADJUST_COMMAND) )
            return;  // not for us.
        // this should be a time zone for us to use.
        LOCAL_TIME_ZONE_OFFSET = (integer)((string)id);
//llSay(0, "got a new time zone of " + (string)LOCAL_TIME_ZONE_OFFSET );
        // handle this update as soon as possible.
        llSetTimerEvent(0.1);
    }
}

