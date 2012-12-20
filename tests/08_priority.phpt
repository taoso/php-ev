--TEST--
Check for priorities
--FILE--
<?php 
error_reporting(0);

$t0 = new EvTimer(-1, 0, function ($w, $r) { echo "ok 4\n"; });
$t_ = new EvTimer(-1, 0, function ($w, $r) { echo "ok 5\n"; });
$t_->priority = -1;
$t1 = new EvTimer(-1, 0, function ($w, $r) { echo "ok 3\n"; });
$t1->priority = 1;

$i2 = new EvIdle(function ($w, $r) { ev_iteration() == 1 ? "" : "not ", "ok 2\n"; $w->stop(); });
$i2->priority = 10;
$i3 = new EvIdle(function ($w, $r) { ev_iteration() == 3 ? "" : "not ", "ok 7\n"; $w->stop(); });
$i1 = new EvIdle(function ($w, $r) { ev_iteration() == 2 ? "" : "not ", "ok 6\n"; $w->stop(); });
$i1->priority = 1;
$i_ = new EvIdle(function ($w, $r) { ev_iteration() == 4 ? "" : "not ", "ok 8\n"; $w->stop(); });
$i_->priority = -1;

$t0->start();
$t_->start();
$t1->start();
$i2->start();
$i3->start();
$i1->start();
$i_->start();

echo "ok 1\n";
ev_run();
echo "ok 9\n";
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
