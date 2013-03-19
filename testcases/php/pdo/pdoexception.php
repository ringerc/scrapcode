<?php

# Writen for http://stackoverflow.com/questions/15421981/how-postgres-raise-exception-gets-converted-into-pdoexception/15422304#15422304

$pdo = new PDO('pgsql:');

$sql = <<<EOD
CREATE OR REPLACE FUNCTION exceptiondemo() RETURNS void AS $$
BEGIN
  RAISE SQLSTATE 'UE001' USING MESSAGE = 'error message';
END;
$$ LANGUAGE plpgsql
EOD;

$sth = $pdo->prepare($sql);
if (!$sth->execute()) {
	die("Failed to create test function\n");
}

$sql = "SELECT exceptiondemo();";
$sth = $pdo->prepare($sql);

$pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
try {
	$sth->execute();
} catch (PDOException $err) {
	$ei = $err->errorInfo;	
	die("Function call failed with SQLSTATE " . $ei[0] . ", message " . $ei[2] . "\n");

	// Alternate version to just get code:
	//die("Function call failed with SQLSTATE " . $err->getCode() . "\n");
}

?>
