--TEST--
Check for EvWatcher::keepalive() functionality
--FILE--
<?php
//error_reporting(0);

$timer = EvTimer::createStopped(1, 0.3, function ($w, $r) {
	echo "ok 7\n";
	$w->stop();
});
$timer->keepalive();

echo "ok 1\n";
Ev::run();
echo "ok 2\n";

$timer->start();

$timer->keepalive(false);

$timer->again();
$timer->stop();
$timer->start();

$timer2 = new EvTimer(-1, 0, function ($w, $r) {
	echo "ok 4\n";
});
$timer2->keepalive();

echo "ok 3\n";
Ev::run(0);
echo "ok 5\n";

$timer->keepalive();

echo "ok 6\n";
Ev::run();
echo "ok 8\n";
?>
--EXPECT--
ok 1
ok 2
ok 3
ok 4
ok 5
ok 6
ok 7
ok 8
