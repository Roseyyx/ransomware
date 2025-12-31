<?php

require_once "connect.php";

if ($_SERVER["REQUEST_METHOD"] == "POST") {
  $input_machineId = hash("sha256", $_POST["machineId"]);
  $exists = false;
  $sql = "SELECT * FROM clients WHERE machineHash = ?";

  if ($stmt = mysqli_prepare($link, $sql)) {
    mysqli_stmt_bind_param($stmt, "s", $input_machineId);

    if (mysqli_stmt_execute($stmt)) {
      $result = mysqli_stmt_get_result($stmt);
      if (mysqli_num_rows($result) == 0) {
        $exists = false;
      } else if (mysqli_num_rows($result) >= 1) {
        $exists = true;
        mysqli_stmt_close($stmt);
        mysqli_close($link);

        http_response_code(409);
        exit();
      } else {
        mysqli_stmt_close($stmt);
        mysqli_close($link);

        http_response_code(400);
        exit();
      }
    }
  }

  mysqli_stmt_close($stmt);
  if ($exists) {
    mysqli_close($link);

    http_response_code(409);
    exit();
  }

  $randomSalt = bin2hex(random_bytes(16));
  $combined = $input_machineId . $randomSalt;
  $masterKey = hash("sha256", password_hash($combined, PASSWORD_DEFAULT));
  $paid = 0;

  $sql = "INSERT INTO clients (masterKey, machineHash, salt, hasPaid) VALUES (?,?,?,?)";

  if ($stmt = mysqli_prepare($link, $sql)) {
    mysqli_stmt_bind_param($stmt, "sssi", $masterKey, $input_machineId, $randomSalt, $paid);
    if (mysqli_stmt_execute($stmt)) {
      mysqli_stmt_close($stmt);
      mysqli_close($link);

      http_response_code(200);
      exit();
    }
  }

  //http_response_code(200);
  mysqli_stmt_close($stmt);
  mysqli_close($link);

  http_response_code(400);
  exit();
}

?>

<!DOCTYPE html>

<html>

<head></head>

<body>
  <form action="<?php echo htmlspecialchars(basename($_SERVER['REQUEST_URI'])); ?>" method="post">
    <label>machineId</label>
    </br>
    <input type="text" name="machineId">
    </br>
    <input type="submit" value="Submit">
  </form>
</body>

</html>
