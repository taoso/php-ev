--TEST--
Check for bug #64788
--FILE--
<?php
$io = new EvIo(STDIN, Ev::READ, function ($watcher, $revents) {
	echo "ok 1";
	$watcher->stop();
	Ev::stop();
});
$dump = var_export($io, 1);
$timer = new EvTimer(0.2, 0, function ($w, $r) use ($io) {
	echo "ok 2";
	$io->stop();
	Ev::stop();
});
Ev::run();
?>
--EXPECTF--
ok %d
