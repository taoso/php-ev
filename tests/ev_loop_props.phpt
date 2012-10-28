--TEST--
Check for EvLoop class properties
--FILE--
<?php 
function cb($watcher, $revents) {
	echo __FUNCTION__, "() called", PHP_EOL;
	$watcher->stop();
	return FALSE;
}
//$fd = fopen('listen_to_me', 'r');

$loop       = EvLoop::default_loop();
$loop->data = "ld";
$io_watcher = new EvIo(STDIN, EV_READ, $loop, "cb", "data", 2);
var_dump($io_watcher->getLoop());
var_dump($io_watcher->priority);
var_dump($io_watcher->data);

//$io_watcher->start();
//$loop->run();
//echo "closing fd\n";
//fclose($fd);

?>
--EXPECT--
