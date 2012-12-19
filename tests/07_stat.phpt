--TEST--
Check for EvStat functionality
--FILE--
<?php 
error_reporting(0);

$fh = tempnam(__DIR__, 'ev_tmp_');

$w = new EvStat("$fh", 0.1, function ($w, $r) {
	$prev = $w->prev();

	echo "ok 5\n";
	echo $prev ? "" : "not ", "ok 6\n";
	echo 13 == count($prev) ? "" : "not ", "ok 7\n";
	echo 1 == $prev['nlink'] ? "" : "not ", "ok 8\n";
	echo FALSE == $w->attr() ? "" : "not ", "ok 9\n";
	echo FALSE == $w->stat() ? "" : "not ", "ok 10\n";

   	ev_break();
});
$w->start();


$t = new EvTimer(0.2, 0, function ($w, $r) {
	echo "ok 2\n";
	ev_break();
});
$t->start();

echo $w->stat() ? "" : "not ", "ok 1\n";
ev_run();
echo "ok 3\n";

unlink($fh);

$t = new EvTimer(0.2, 0, function ($w, $r) {
	echo "ok 2\n";
	ev_break();
});
$t->start();

echo "ok 4\n";
ev_run();
echo "ok 11\n";

?>
--CLEAN--
<?php
@unlink($fh);
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
ok 9
ok 10
ok 11
