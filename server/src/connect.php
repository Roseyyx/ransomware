<?php
define('DB_SERVER', '172.18.0.2');
define('DB_USERNAME', 'root');
define('DB_PASSWORD', 'R!@database12');
define('DB_NAME', 'leertaak');

$link = mysqli_connect(DB_SERVER, DB_USERNAME, DB_PASSWORD, DB_NAME);

if ($link === false) {
  die("ERROR: Could not connect. " . mysqli_connect_error());
}
