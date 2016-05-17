--TEST--
Check for bug #24
--FILE--
<?php
$io = new EvIo(STDIN, Ev::READ | Ev::WRITE, function ($w) {
	var_dump(is_resource($w->fd));
	$w->stop();
	Ev::stop();
});
Ev::run();
?>
--EXPECTF--
bool(true)
