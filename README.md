# DECS_KeyValueServerProject
This project is developed to get hands on experience on creating multithreaded server with database and caching, and then perform load test with a load generator and find bottlenecks.

Required Files:

1) httplib.h :-  wget https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
2) libpq-dev :- sudo apt-get install libpq-dev
3) sudo apt install postgresql postgresql-contrib

Steps to setup database:
sudo service postgresql start
sudo service postgresql status
sudo -i -u postgres
psql
postgres=#

CREATE DATABASE kvdb;
CREATE USER kvuser WITH ENCRYPTED PASSWORD 'kvpass';
GRANT ALL PRIVILEGES ON DATABASE kvdb TO kvuser;

\q
exit

//Set md5 authentication instead of peer authentication
sudo nano /etc/postgresql/*/main/pg_hba.conf

//inside this file locate "local   all             all                                     peer" and update it to:
local   all             all                                     md5
//without this change "psql -U kvuser -d kvdb" this will not work.

//grant some privileges to create table
sudo -u postgres psql

GRANT ALL PRIVILEGES ON DATABASE kvdb TO kvuser;
GRANT CREATE ON SCHEMA public TO kvuser;

\q
exit

sudo -u postgres psql -d kvdb

GRANT USAGE ON SCHEMA public TO kvuser;
GRANT CREATE ON SCHEMA public TO kvuser;
ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON TABLES TO kvuser;

ALTER SCHEMA public OWNER TO kvuser;

\q


//Now Create Table

psql -U kvuser -d kvdb
//when prompted for password use 'kvpass'

CREATE TABLE kv_store (
    key TEXT PRIMARY KEY,
    value TEXT
);

//To Check Table
psql -U kvuser -d kvdb -c "SELECT * FROM kv_store;"


// To send create request:
curl -X POST -d "key=name&value=harshit" http://localhost:8080/create

//To send 

//To run:
g++ server.cpp database.cpp cache.cpp -o server -I/usr/include/postgresql -lpq -pthread -std=c++17

