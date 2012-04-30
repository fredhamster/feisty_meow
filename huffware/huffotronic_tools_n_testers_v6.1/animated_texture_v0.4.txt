
// huffware script: animated texture, by fred huffhines.
//
// super simple texture animation, just an example right now.
//
//   this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
//   do not use it in objects without fully realizing you are implicitly accepting that license.
//


default {
    state_entry() { if (llSubStringIndex(llGetObjectName(), "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default {
    state_entry() {
        llSetTextureAnim(ANIM_ON | LOOP, ALL_SIDES, 4, 4, 0, 0, 3);
    } 
}


