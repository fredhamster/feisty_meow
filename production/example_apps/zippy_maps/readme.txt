
steps to bringing up this maps demo site...

* need to configure a database in app.php.  see default version: config/app.default.php 

* need an api key from google to have separate hit quotas on mapping API.
  * refer to: https://developers.google.com/maps/documentation/distance-matrix/get-api-key
  * see default version: config/config_google.default.php
  * example of using api key:
    * https://maps.googleapis.com/maps/api/distancematrix/json?origins=Seattle&destinations=San+Francisco&key=YOUR_API_KEY

* need to install the cake geo code.
  * more details at: https://github.com/dereuromark/cakephp-geo/blob/master/docs/Install.md

* need to have jquery available and loaded someplace.  i got a recent minified version of it and stored it in webroot/js and then added this to default.ctp before the fetch('script') call:
  <script src="/js/jquery-3.2.1.min.js"></script>

* need to load the marker clusterer code for map marker clustering to work.  see ClusterMapHelper.php for more info.

* a bunch of icons for map markers can be found here: https://code.google.com/archive/p/google-maps-icons/downloads

* need to make links for the plugins:
  from: https://book.cakephp.org/3.0/en/deployment.html#symlink-assets
  run: bin/cake plugin assets symlink

---

assorted reference notes:

mysql and php using javascript api with maps:
https://developers.google.com/maps/documentation/javascript/mysql-to-maps

how to show things in a form with checkboxes:
https://book.cakephp.org/3.0/en/views/helpers/form.html#creating-inputs-for-associated-data

tagList above is computed this way:
https://book.cakephp.org/3.0/en/orm/retrieving-data-and-resultsets.html#finding-key-value-pairs

events that can be hooked are listed here:
https://developers.google.com/maps/documentation/javascript/reference


