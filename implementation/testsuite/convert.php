<?php
$div = floatval($argv[2]);
$fn = fopen($argv[1],"r");
  
  while(! feof($fn))  {
	$result = fgets($fn);
	echo floatval($result) / $div;
	echo "\n";
  }

  fclose($fn);
