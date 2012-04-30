

// huffware script: die on demand, by fred huffhines.
//
// a super simple script that merely makes an object subject to self-termination
// if it is told a secret phrase.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

//////////////
// API for the die on demand feature.  saying the message below on the channel
// referenced will cause any listening object to zap itself out of the grid.
string DIE_ON_DEMAND_MESSAGE = "die-on-demand";
integer DIE_ON_DEMAND_CHANNEL = 4826;
//////////////

default
{
    state_entry()
    {
        llListen(DIE_ON_DEMAND_CHANNEL, "", NULL_KEY, "die-on-demand");
    }
    
    listen(integer channel, string name, key id, string message) {
        if ( (channel ==  DIE_ON_DEMAND_CHANNEL)
            && (llGetOwnerKey(id) == llGetOwnerKey(llGetKey()))
            && (message == DIE_ON_DEMAND_MESSAGE) ) {
            llWhisper(0, "removing object.");
            llDie();
        }
    }
}
