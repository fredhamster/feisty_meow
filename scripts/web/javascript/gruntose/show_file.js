<script language="JavaScript1.2">
<!--

////////////////////////////////////////////////////////////////////////////

// Author: Fred T. Hamster.  Open Source; distributed under GNU Public License.
//
// this is a very silly approach to being able to show a file, even if it would normally
// be miconstrued as an executable.  but it's better just to get the web server configured
// properly so you're showing the scripts in their non-live forms.

////////////////////////////////////////////////////////////////////////////

function show_file(page_loc) {
  var win_locn = window.location.href;
//alert("winlocn " + win_locn);
  var last_slash = win_locn.lastIndexOf("/");
//alert("lastslash " + last_slash);
  if (last_slash < 0) return;
  var dir_locn = win_locn.slice(0, last_slash + 1);
//alert("dirlocn " + dir_locn);
  var new_locn = "view-source:" + dir_locn + page_loc;
//alert("showing " + new_locn);
  window.location = new_locn;
}


//-->
</script>

