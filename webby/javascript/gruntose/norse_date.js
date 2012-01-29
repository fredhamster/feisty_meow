
<script language="JavaScript1.2">
<!--

////////////////////////////////////////////////////////////////////////////

// Author: Fred T. Hamster.  Open Source; distributed under GNU Public License.

////////////////////////////////////////////////////////////////////////////

// shows the current date.  doesn't attempt any kind of refresh.  the days
// use more archaic forms of weekday names.

// here are some notes:
//   sunday = sun day
//   monday = moon day
//   tuesday = tyr's day: http://en.wikipedia.org/wiki/Tyr
//   wednesday = odin's day: http://en.wikipedia.org/wiki/Odin
//   thursday = thor's day: http://en.wikipedia.org/wiki/Thor
//   friday = freja's day: (or freya or freyja)
//       http://en.wikipedia.org/wiki/Freyja
//   saturday = saturn's day: http://en.wikipedia.org/wiki/Saturn_%28god%29
//       saturn's the odd man out here, since he's a roman deity, but
//       england was ruled by rome for a bit.  those bastards!

function show_norse_date() {
  var now = new Date();
  var today = now.getDay();
  var day_name;
  switch (today) {
    case 0: day_name = "Sun Day"; break;
    case 1: day_name = "Moon Day"; break;
    case 2: day_name = "Tyr Day"; break;
    case 3: day_name = "Odin Day"; break;
    case 4: day_name = "Thor Day"; break;
    case 5: day_name = "Freya Day"; break;
    case 6: day_name = "Saturn Day"; break;
  }

  var hour = now.getHours();
  var pm = false;

  if (hour >= 12) {
    pm = true
    hour -= 12;
  }
  if (hour == 0) hour = 12;

  var minutes = now.getMinutes();

  if (minutes < 10) minutes = '0' + minutes;

  var time_name = hour + ":" + minutes + (pm? "pm" : "am");

  document.write(day_name + " " + time_name);
}

//-->
</script>
