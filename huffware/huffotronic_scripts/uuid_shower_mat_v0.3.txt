
// huffware script: uuid shower, by fred huffhines
//
// a simple script that shows an avatar their universally unique ID.
//
//   this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
//   do not use it in objects without fully realizing you are implicitly accepting that license.

default
{
    state_entry() { if (llSubStringIndex(llGetObjectName(),  "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default
{
    state_entry() {}

    touch_start(integer total_number)
    {
        llSay(0, "Hello " + llDetectedName(0) + ", your UUID is " + (string)llDetectedKey(0));
    }
}
