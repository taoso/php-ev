--TEST--
Check for get_gc property handler
--FILE--
<?php
// Basic behavior
$p = new EvPeriodic(0, 0.02, null, function ($w, $rev) {
	static $n = 0;
	if (++$n > 10) {
		$w->stop();
		return;
	}
	echo "{$w->data}{$n} ", ($rev & Ev::PERIODIC ? 'ok' : 'x'), PHP_EOL;
}, 'a');
Ev::run();

// Offset, no interval
$p = new EvPeriodic(time() + 0.1, 0, null, function ($w, $rev) {
	echo "{$w->data} ", ($rev & Ev::PERIODIC ? 'ok' : 'x'), PHP_EOL;
}, 'b');
Ev::run();

// Reschedule callback
$p = new EvPeriodic(0, 0, function ($w, $now) {
	static $n = 0;
	return (++$n < 10 ? $now + 0.1 : 60);
}, function ($w, $rev) {
	static $n = 0;
	if (++$n > 10) {
		$w->stop();
		return;
	}
	echo "{$w->data}{$n} ", ($rev & Ev::PERIODIC ? 'ok' : 'x'), PHP_EOL;
}, 'c');
Ev::run();
?>
--EXPECT--
a1 ok
a2 ok
a3 ok
a4 ok
a5 ok
a6 ok
a7 ok
a8 ok
a9 ok
a10 ok
b ok
c1 ok
c2 ok
c3 ok
c4 ok
c5 ok
c6 ok
c7 ok
c8 ok
c9 ok
c10 ok
