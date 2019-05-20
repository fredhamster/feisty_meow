show databases;
#create database zipcodes;
use zipcodes;

describe categories;

select * from categories where image is null;

describe locations;

select * from locations where id > 3000;

# useful floating point compare technique.
select * from locations where abs(lat - '18.17') <= 1e-6;

select * from locations where id = 607;

select * from locations ;
