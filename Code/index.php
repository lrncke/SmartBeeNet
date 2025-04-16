<!DOCTYPE html>
<html>
<head>
    <title>Bienenstock-Daten</title>
	<link rel="stylesheet" href="style.css">
</head>
<body>
    <h1>Daten aus dem Bienenstock</h1>

    <form method="get" action="">
        <label for="stocknummer">Stocknummer:</label>
        <input type="number" name="stocknummer" id="stocknummer" required><br><br>

        <label for="anzahl">Anzahl der letzten Einträge:</label>
        <input type="number" name="anzahl" id="anzahl" required><br><br>

        <input type="submit" value="Anzeigen">
    </form>

    <hr>

    <?php
    if (isset($_GET['stocknummer']) && isset($_GET['anzahl'])) {
        // Eingaben holen und absichern
        $stocknummer = intval($_GET['stocknummer']);
        $anzahl = intval($_GET['anzahl']);

        // Verbindung zur Datenbank
        $servername = "localhost";
        $username = "bienenuser"; // Oder anderer Benutzer
        $password = "admin";     // Dein Passwort
        $dbname = "bienenprojekt";

        $conn = new mysqli($servername, $username, $password, $dbname);

        if ($conn->connect_error) {
            die("Verbindung fehlgeschlagen: " . $conn->connect_error);
        }

        // SQL-Abfrage: letzte N Einträge für die Stocknummer, absteigend nach Zeit
        $sql = "SELECT * FROM messwerte WHERE stocknummer = $stocknummer ORDER BY zeitstempel DESC LIMIT $anzahl";
        $result = $conn->query($sql);

        if ($result->num_rows > 0) {
            echo "<h2>Letzte $anzahl Einträge für Stocknummer $stocknummer:</h2>";
            echo "<table border='1' cellpadding='5'><tr><th>ID</th><th>Temperatur</th><th>Gewicht</th><th>Zeit</th></tr>";
            while($row = $result->fetch_assoc()) {
                echo "<tr><td>".$row["id"]."</td><td>".$row["temperatur"]."</td><td>".$row["gewicht"]."</td><td>".$row["zeitstempel"]."</td></tr>";
            }
            echo "</table>";
        } else {
            echo "Keine Einträge gefunden für Stocknummer $stocknummer.";
        }

        $conn->close();
    }
    ?>
</body>
</html>
