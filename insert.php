                                                                                                                              
   <?php
   // ============================================
   // PHP Script: insert.php
   // Purpose: Receive JSON data from ESP8266
   // and insert temperature & humidity into MySQL
   // ============================================
   
   // :one: Database connection details
   $servername = "localhost"; // MySQL server
   $username= "root"; // MySQL username
   $password= ""; // MySQL password
   $dbname= "design"; // MySQL database name
   
   // :two: Create a connection
   $conn = new mysqli($servername, $username, $password, $dbname);
   
   // :three: Check connection
   if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
   }
   
   // :four: Read raw POST data (ESP is sending JSON)
   $data = file_get_contents("php://input");
   
   // :five: Decode JSON to PHP array
   $json = json_decode($data, true);
   
   // :six: Validate JSON
   if (!$json || !isset($json['temp']) || !isset($json['hum'])) {
   die("Invalid JSON data");
   }
   
   // :seven: Extract temperature and humidity values
   $temp = floatval($json['temp']);
   $hum = floatval($json['hum']);
   
   // :eight: Prepare SQL INSERT statement using placeholders
   $stmt = $conn->prepare("INSERT INTO sensor_data (temperature, humidity) VALUES (?, ?)");
   
   // :nine: Bind parameters: "dd" = two doubles (temperature, humidity)
   $stmt->bind_param("dd", $temp, $hum);
   
   // :keycap_ten: Execute the statement
   if ($stmt->execute()) {
   echo "OK";// Send response back to ESP
   } else {
   echo "ERROR: " . $stmt->error;
   }
   
   // :one::one: Close statement and connection
   $stmt->close();
   $conn->close();
   ?>