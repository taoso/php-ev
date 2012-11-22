--TEST--
Check for constructors of the classes derived from EvWatcher
--FILE--
<?php
function cb() {
	return FALSE;
}

$loop = new EvLoop();
$io_watcher = new EvIo(STDIN, EV_READ, $loop, "cb");
var_dump($io_watcher);


?>
--EXPECTF--
