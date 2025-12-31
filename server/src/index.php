<!DOCTYPE html>
<html>

<head>
</head>

<body>
  <table>
    <thead>
      <tr>
        <th>#</th>
        <th>Master Key</th>
        <th>Machine Hash</th>
        <th>Salt</th>
        <th>Heeft betaald</th>
        <th>Acties</th>
      </tr>
    </thead>
    <?php
    require_once "./connect.php";

    $sql = "SELECT * FROM clients";
    if ($result = mysqli_query($link, $sql)) {
      if (mysqli_num_rows($result) > 0) {
        while ($row = mysqli_fetch_array($result)) {
          echo "<tbody>";
          echo "<td>" . $row['id'] . "</td>";
          echo "<td>" . $row["masterKey"] . "</td>";
          echo "<td>" . $row['machineHash'] . "</td>";
          echo "<td>" . $row['salt'] . "</td>";
          echo "<td>" . ($row['hasPaid'] ? "Ja" : "Nee") . "</td>";
          echo "<td>";
          echo '<a href="update.php?id=' . $row['id'] . '" title="Update Record"><span>Update</span></a>';
        }
        echo "</tbody>";
      }
    }
    ?>
  </table>
</body>

</html>
