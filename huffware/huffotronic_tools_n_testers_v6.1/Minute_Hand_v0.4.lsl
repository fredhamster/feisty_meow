
// huffware script: minute hand, modified by fred huffhines.
// 
// noticed this in arcadia asylum's great hobo cuckoo clock.
// this script is licensed by Beezle Warburton:
//    "I released those 'into the wild' years ago,do what you like,
//     I just ask that people don't resell the scripts by themselves."
//
// fred's changes licensed by:
//   this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
//   do not use it in objects without fully realizing you are implicitly accepting that license.
//

integer time;
integer hours;
integer minutes;

float anglehours;
float angleminutes;

default
{
    state_entry() { if (llSubStringIndex(llGetObjectName(),  "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default
{
    state_entry() {
        // we want to calculate once a minute.
        llSetTimerEvent(60.0);
    }

    timer() {
        time = (integer)llGetGMTclock(); // seconds since midnight
        minutes = (time % 3600) / 60;        
        angleminutes = -(TWO_PI * (float)minutes / 60);
        llRotateTexture(angleminutes, ALL_SIDES);
    }
}
