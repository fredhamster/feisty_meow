
// huffware script: minnow, modifications by fred huffhines.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//
// original attributions:
//   Fixit Galatea generic boat script
//   This script uses the basic direction keys for boat operation and requires
//   the final prim to be oriented at rotation <0, 0, 0>.

//****************************************************************************
//****************************************************************************
//****************************************************************************
//* The following items can be edited in this script. See the comment for each
//* item to see how it work when it is changed.
//****************************************************************************

//* When the forward key is pressed and the mouse button is pressed at the
//* same time, this speed is used. This number should be between 0.0 and 30.0,
//* with higher numbers giving faster performance.
float fastspeed = 16.0;

//* When the forward key is pressed and the mouse button is NOT pressed, this
//* speed is used. This number should be between 0.0 and whatever the
//* fastspeed value was set at, with higher numbers giving faster performance.
float slowspeed = 8.0;

//* This is the reverse speed. It shouldn't be too fast, but can be between
//* 0.0 and 30.0.
float backspeed = 2.0;

//* This is the vehicle 'hover' height, or how far it sits out of the water.
//* Adjust this as necessary for your boat.
float floatheight = 0.44;

//* If the boat is to ramp up speed slowly, set the following value to '1'.
//* Otherwise, the board will instantly reach full speed when the forward
//* key is pressed.
integer fastforward = 0;  // NOT IMPLEMENTED YET

//* Turning speed, a small turning speed is best
float turnspeed = 1.5;

//* If the boat 'angles' into turns, set the following value to '1'.
//* Otherwise, the boat will do flat turns.
integer angledturn = 1;   // NOT IMPLEMENTED YET

//* Height offset of seat
//vector seatheight = <.2, 0, 0.35>;

vector camera_lives_at = <-7, 0, 3>;
    // where the camera is poised when the avatar is seated.

vector camera_looks_at = <4, 0, 1>;

//****************************************************************************
//****************************************************************************
//****************************************************************************

// Initialize the vehicle as a boat
vehicle_init()
{
    llSetVehicleType(VEHICLE_TYPE_BOAT);

    // least for forward-back, most friction for up-down
    llSetVehicleVectorParam(VEHICLE_LINEAR_FRICTION_TIMESCALE, <10, 1, 3>);

    // uniform angular friction (setting it as a scalar rather than a vector)
    llSetVehicleFloatParam(VEHICLE_ANGULAR_FRICTION_TIMESCALE, 0.1);

    // linear motor wins after about five seconds, decays after about a minute
    llSetVehicleVectorParam(VEHICLE_LINEAR_MOTOR_DIRECTION, <0, 0, 0>);
    llSetVehicleFloatParam(VEHICLE_LINEAR_MOTOR_TIMESCALE, 0.1);
    llSetVehicleFloatParam(VEHICLE_LINEAR_MOTOR_DECAY_TIMESCALE, 0.05);

    // agular motor wins after four seconds, decays in same amount of time
    llSetVehicleVectorParam(VEHICLE_ANGULAR_MOTOR_DIRECTION, <0, 0, 0>);
    llSetVehicleFloatParam(VEHICLE_ANGULAR_MOTOR_TIMESCALE, 0.1);
    llSetVehicleFloatParam(VEHICLE_ANGULAR_MOTOR_DECAY_TIMESCALE, 0.2);

    // hover 
    llSetVehicleFloatParam(VEHICLE_HOVER_HEIGHT, floatheight);
    llSetVehicleFloatParam(VEHICLE_HOVER_EFFICIENCY, 0.2);
    llSetVehicleFloatParam(VEHICLE_HOVER_TIMESCALE, 0.4);
    llSetVehicleFloatParam(VEHICLE_BUOYANCY, 1.0);

    // Slight linear deflection with timescale of 1 seconds
    llSetVehicleFloatParam(VEHICLE_LINEAR_DEFLECTION_EFFICIENCY, 0.1);
    llSetVehicleFloatParam(VEHICLE_LINEAR_DEFLECTION_TIMESCALE, 1);

    // Slight angular deflection 
    llSetVehicleFloatParam(VEHICLE_ANGULAR_DEFLECTION_EFFICIENCY, 0.1);
    llSetVehicleFloatParam(VEHICLE_ANGULAR_DEFLECTION_TIMESCALE, 6);

    // somewhat bounscy vertical attractor 
    llSetVehicleFloatParam(VEHICLE_VERTICAL_ATTRACTION_EFFICIENCY, 0.5);
    llSetVehicleFloatParam(VEHICLE_VERTICAL_ATTRACTION_TIMESCALE, 1);

    // weak negative damped banking
    llSetVehicleFloatParam(VEHICLE_BANKING_EFFICIENCY, 1.0);
    llSetVehicleFloatParam(VEHICLE_BANKING_MIX, 1.0);
    llSetVehicleFloatParam(VEHICLE_BANKING_TIMESCALE, 0.1);

    // default rotation of local frame
    llSetVehicleRotationParam(VEHICLE_REFERENCE_FRAME,
        llEuler2Rot(<0, 0, 0>));

    // remove these flags.
//disabled due to opensim problems running this call.
//    llRemoveVehicleFlags(VEHICLE_FLAG_HOVER_TERRAIN_ONLY 
//                         | VEHICLE_FLAG_LIMIT_ROLL_ONLY
//                         | VEHICLE_FLAG_HOVER_GLOBAL_HEIGHT);

    // set these flags 
//    llSetVehicleFlags(VEHICLE_FLAG_NO_DEFLECTION_UP 
//                      | VEHICLE_FLAG_HOVER_WATER_ONLY 
//                      | VEHICLE_FLAG_HOVER_UP_ONLY 
//                      | VEHICLE_FLAG_LIMIT_MOTOR_UP);
}

// Setup everything
all_setup()
{
    // Display version number
    llWhisper(0, "running script " + llGetScriptName());
    llSetStatus(STATUS_PHYSICS, FALSE);

    // Set sit direction (forward) and sight location slightly up and behind
//    llSitTarget(seatheight, llEuler2Rot(<0, 0, 0>));
    llSetCameraAtOffset(camera_looks_at);
    llSetCameraEyeOffset(camera_lives_at);
    llSetSitText("Pilot");

    // Initialize vehicle states
    vehicle_init();

    // Set up listener callback function
    llListen(0, "", llGetOwner(), "");
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

// State (default) event handlers
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
        auto_retire();
        all_setup();
    }

    on_rez(integer start_param)
    {
        llSetStatus(STATUS_PHYSICS, FALSE);
    }

     run_time_permissions(integer permissions)
    {
        // Get user permissions
        if ((permissions & PERMISSION_TAKE_CONTROLS) ==
            PERMISSION_TAKE_CONTROLS)
        {
            llTakeControls(CONTROL_ML_LBUTTON | CONTROL_FWD |
                CONTROL_BACK | CONTROL_LEFT | CONTROL_RIGHT |
                CONTROL_ROT_LEFT | CONTROL_ROT_RIGHT, TRUE, FALSE);
        }
    }

    changed(integer change)
    {
        if (change & CHANGED_LINK)
        {
            key agent = llAvatarOnSitTarget();
            if (agent)
            {
                if (agent != llGetOwner())
                {
                    llSay(0, "This boat isn't yours, but you can buy a copy!");
                    llUnSit(agent);
                }
                else
                {
                    llRequestPermissions(agent, PERMISSION_TAKE_CONTROLS);
                    llSetStatus(STATUS_PHYSICS, TRUE);
                }
            }
            else
            {
                llReleaseControls();
                llSetStatus(STATUS_PHYSICS, FALSE);
            }
        }
    }

    control(key name, integer levels, integer edges) 
    {
        float side = 0.0;
        float forw = 0.0;
        float move = 0.0;
        float turn;

        if (levels & CONTROL_ML_LBUTTON)
        {
            move = fastspeed;
            turn = 1.5 * turnspeed;
//            forw = 2 * PI / 3;
        }
        else if (levels & CONTROL_FWD)
        {
            move = slowspeed;
            turn = 1.0 * turnspeed;
//            forw = PI / 2;
        }
        else if (levels & CONTROL_BACK)
        {
            move = -backspeed;
//            forw = -PI / 3;
//            turn = -1.0 * turnspeed;
        }

        if (levels & (CONTROL_LEFT | CONTROL_ROT_LEFT))
        {
            if (move == fastspeed)
            {
                side = turnspeed;
//                forw = PI;
            }
            else if (move != 0)
            {
                side = turnspeed;
//                forw = PI / 3;
            }
            else
            {
                side = .67 * turnspeed;
//                forw = PI / 4;
                move = 0.1;
            }
        }
        else if (levels & (CONTROL_RIGHT | CONTROL_ROT_RIGHT))
        {
            if (move == fastspeed)
            {
                side = -turnspeed;
//                forw = PI;
            }
            else if (move != 0)
            {
                side = -turnspeed;
//                forw = PI / 3;
            }
            else
            {
                side = -.67 * turnspeed;
//                forw = PI / 4;
                move = 0.1;
            }
        }

        if (move == 0)
        {
            llSetVehicleVectorParam(VEHICLE_LINEAR_MOTOR_DIRECTION, <0, 0, 0>);
        }
        else
        {
            llSetVehicleVectorParam(VEHICLE_LINEAR_MOTOR_DIRECTION,
                  <move, 0, 0>);
            llSetVehicleVectorParam(VEHICLE_ANGULAR_MOTOR_DIRECTION,
                <-side / 5, 0, side>);
        }
    }

    listen(integer channel, string name, key id, string message)
    {
        list params = llParseString2List(message, [" "], [":"]);
        string cmd = llList2String(params, 0);
        integer enable = llList2Integer(params, 1);

        // Commands
        ;
    }

    touch_start(integer total_number)
    {
        key id = llDetectedOwner(total_number - 1);
       // llGiveInventory(id, "Info card");
    }
}
