string hower = "Hyper Jump\n* Wilder Westen *";     // Hover text
vector color = <0,0,0>;                 // color of the hover text
string sim = "grid-ww.talentraspel.de:9000:Wilder Westen";     // ip/name + port of the target

vector pos = <147,158,25>;              // position


key user;

default
{
    state_entry() { if (llSubStringIndex(llGetObjectName(),  "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default
{
    state_entry() {
        llSetText(hower,color,1);
    }
    
    touch_start(integer total_number) {
        user = llDetectedKey(0); 
        llMapDestination (sim, pos, <1,1,1>);
    }
}

