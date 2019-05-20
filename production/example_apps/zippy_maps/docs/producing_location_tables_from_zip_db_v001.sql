# note: this code was appropriate before migration AddLatLongColumnsToLocationsTable happened in july 2017.
# if one wanted to take a kake zip style db to the map demo db, there would be a few changes (such as getting rid of the
# combining of lat and long into a single field).

# a bunch of helper bits.  real important code is at the end.
use zipcodes;
select database();
show tables;
select * from kake_zip_code;
select lat, lon, locationtext, country from kake_zip_code ;
select location, concat(lat, ',', lon), concat(locationtext, ' ', zip_code, ', ', country) from kake_zip_code where lat != 0 and lon != 0 limit 500;
describe locations;
describe categories;
describe categories_locations;

# testing select with concats.
select location, concat(locationtext, ' ', zip_code), concat(lat, ',', lon) from kake_zip_code where lat != 0 and lon != 0 limit 500;

select * from kake_zip_code limit 500;
select * from locations;
select * from categories;
select * from categories_locations;
delete from locations where id != 0;
ALTER TABLE locations AUTO_INCREMENT = 1;
delete from categories where id != 0;
ALTER TABLE categories AUTO_INCREMENT = 1;
delete from categories_locations where id != 0;
ALTER TABLE categories_locations AUTO_INCREMENT = 1;

select * from locations where location like '%, AK %';

# these are the important actions...

# the real business gets done by these inserts.
insert into locations (id, name, location, latlong, created, modified) select null, concat(location, '-', zip_code), concat(locationtext, ' ', zip_code), concat(lat, ',', lon), now(), now() from kake_zip_code where lat != 0 and lon != 0 limit 500000;
insert ignore into categories (id, name, created, modified, image) select null, concat(country, '-', state_prefix), now(), now(), null from kake_zip_code where lat != 0 and lon != 0 limit 500000;
# big one, getting the categories done automatically...
insert ignore into categories_locations (id, location_id, category_id, created, modified) 
 select null, locations.id as location_id, categories.id as category_id, now(), now() from locations inner join categories where locations.name like concat('%', categories.name, '%') limit 500000;
 
