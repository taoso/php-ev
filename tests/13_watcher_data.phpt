--TEST--
Check whether modification of uninitialized EvWatcher::data property is correct
--FILE--
<?php
// Create and start timer firing after 2 seconds
$w = new EvTimer(0, .2, function ($w) {
	echo ++$w->data;
	if (!isset($w->data)) {
		$w->stop();
		return;
	}
	$w->data >= 8 and $w->stop();
}, NULL);

Ev::run();
$w->again();
Ev::run();
?>
--EXPECT--
123456789
