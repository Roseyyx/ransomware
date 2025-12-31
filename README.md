# ransomware

Very basic and simple ransomware written in C for a school project.

Requires to have the mysql tables setup yourself like this:

Database: leertaak, tables: ID (int primary), machineHash (varchar), salt (varchar), haspaid (tinyint)

server creates masterkey based on machineHash which consist of the machineId hashed with sha256 -> add salt to hash -> masterkey -> send to client

Uses Jansson for json parsing and libcurl for web requests

database key: R!@database12
