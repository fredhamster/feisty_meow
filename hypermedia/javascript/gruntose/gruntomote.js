
<script language="JavaScript1.2">
<!--

////////////////////////////////////////////////////////////////////////////

// Author: Fred T. Hamster.  Open Source; distributed under GNU Public License.

////////////////////////////////////////////////////////////////////////////

// creates the remote control window.

function launch_remote() {

control = window.open("", "Gruntomote", "height=360,width=420,resizable=yes");

// href specified later for bug in netscape2.

control.location.href = "gruntose_remote_control.html";

// opener must be set for a different bug in netscape.

if (control.opener == null) control.opener = window;

}

//-->
</script>
