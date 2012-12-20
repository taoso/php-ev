--TEST--
Check for EvWatcher::keepalive() functionality
--FILE--
<?php 
error_reporting(0);

$timer = EvTimer::createStopped(1, 0.3, function ($w, $r) {
	echo "ok 7\n";
	$w->stop();
});
$timer->keepalive(1);

echo "ok 1\n";
ev_run();
echo "ok 2\n";

$timer->start();

$timer->keepalive(0);

$timer->again();
$timer->stop();
$timer->start();

$timer2 = new EvTimer(-1, 0, function ($w, $r) {
	echo "ok 4\n";
});
$timer2->keepalive(0);

echo "ok 3\n";
ev_run();
echo "ok 5\n";


$timer->keepalive(1);

echo "ok 6\n";
ev_run();
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
