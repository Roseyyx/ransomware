<?php

require_once "connect.php";

$paid = 0;

if (isset($_GET["id"]) && !empty(trim($_GET["id"]))) {
  $sql = "SELECT * FROM clients WHERE id = ?";

  if ($stmt = mysqli_prepare($link, $sql)) {
    mysqli_stmt_bind_param($stmt, "i", $param_id);

    $param_id = trim($_GET['id']);

    if (mysqli_stmt_execute($stmt)) {
      $result = mysqli_stmt_get_result($stmt);
      if (mysqli_num_rows($result) == 1) {
        $row = mysqli_fetch_array($result, MYSQLI_ASSOC);

        $paid = $row["hasPaid"];
      }
    }
  }

  mysqli_stmt_close($stmt);
  //mysqli_close($link);
}
if (isset($_POST["paid"]) && !empty(trim($_POST["paid"]))) {
  $id = $_GET["id"];

  //$input_paid = $_POST["paid"];

  $input_paid = 0;
  if ($_POST["paid"] == "on") {
    $input_paid = 1;
  } else {
    $input_paid = 0;
  }

  $sql = "UPDATE clients SET hasPaid = ? WHERE id = ?";

  if ($stmt = mysqli_prepare($link, $sql)) {
    mysqli_stmt_bind_param($stmt, "ii", $param_paid, $param_id);

    $param_paid = $input_paid;
    $param_id = $id;

    if (mysqli_stmt_execute($stmt)) {
      header("location: index.php");
      exit();
    } else {
      echo "something went wrong";
    }
  }

  mysqli_stmt_close($stmt);
  mysqli_close($link);
}
?>

<!DOCTYPE html>
<html>

<head>
</head>

<body>
  <form action="<?php echo htmlspecialchars(basename($_SERVER['REQUEST_URI'])); ?>" method="post">
    <label>Heeft betaald</label>
    <?php
    if ($paid == 0) {
      echo "<input type='checkbox' name='paid'>";
    } else {
      echo "<input type='checkbox' name='paid' checked>";
    }
    ?>
    </br>
    <input type="submit" value="Submit">
  </form>
</body>

</html>
