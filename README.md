# Small DB using mmap()

This repo contains a primitive DB supporting `create`, `insert` and `select` queries.
An example of the storage structure is given in the `data/` folder.
The `.metadata` file contains the required metadata for the table (names and types of each of the attributes) and the `.db` file contains the tuples. <br /><br />
The query used to build the example is: `create table sample (id int, name string, registered bool)`. <br />
To insert tuples into the DB: `insert into sample (1, harry, true), (2, samuel, true), (3, william, false)`. <br />
To select, either specify the attributes or use `*` (`select * from sample`).
