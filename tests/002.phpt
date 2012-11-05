--TEST--
Check for EvIo class
--FILE--
<?php 
function cb($watcher, $revents) {
	echo __FUNCTION__, "() called", PHP_EOL;
	$watcher->stop();
	return FALSE;
}
$loop       = EvLoop::default_loop();
$io_watcher = new EvIo(STDIN, EV_READ, $loop, "cb");
$io_watcher->start();
$loop->run();
?>
--EXPECT--
