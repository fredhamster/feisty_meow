
// huffware script: hamster sound effects, by fred huffhines
//
// a randomizing sound player, originally used to replace a sound
// playing script with bad perms in a freebie hamster.
//
//   this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
//   do not use it in objects without fully realizing you are implicitly accepting that license.
//

list all_sounds;

default
{
    state_entry() { if (llSubStringIndex(llGetObjectName(),  "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default {
    state_entry() {
        llStopSound();
        integer indy;
        for (indy = 0; indy < llGetInventoryNumber(INVENTORY_SOUND); indy++) {
            all_sounds += [ llGetInventoryName(INVENTORY_SOUND, indy) ];
        }
    }

    touch_start(integer cnt) {
        all_sounds = llListRandomize(all_sounds, 1);
        llTriggerSound(llList2String(all_sounds, 0), 1.0);
    }
    
    changed(integer change) {
        if (change & CHANGED_INVENTORY) llResetScript();  // reset when sounds might have changed.
    }
}

