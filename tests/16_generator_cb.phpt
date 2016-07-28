--TEST--
Check for accepting Generator::send as a watcher callback
--FILE--
<?php
function gen () {
	for (;;) {
		$w = yield;
		var_dump($w->data);
	}
}
$gen = gen();
$timer = new EvTimer(0.1, 0, [$gen, 'send'], 'user data');
Ev::run();
?>
--EXPECT--
string(9) "user data"
