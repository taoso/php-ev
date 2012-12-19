--TEST--
Check for EvTimer functionality
--FILE--
<?php 
error_reporting(0);

$fudge = 0.02;
$id = 1;

$base = ev_now();
$prev = ev_now();

for ($i = 1; $i <= 5; ++$i) {
	$t = $i * $i * 1.735435336;
	$t -= (int) $t;

	$timer = new EvTimer($t, 0, function ($w, $r)
		use (&$id, &$prev, $base, $i, $t, $fudge) {
			$now = ev_now();

			EvLoop::default_loop()->verify();

			echo $now + $fudge >= $prev ? "" : "not ", "ok ", ++$id,
				" # t0 $i $now + $fudge >= $prev\n";
			echo $now + $fudge >= $base + $t ? "" : "not ", "ok ", ++$id,
				" # t1 $now + $fudge >= $base + $t\n";

			if (! ($id % 3)) {
				$t *= 0.0625;
				$w->set($t, 0);
				$w->start();
				$t = $now + $t - $base;
			}

			$prev = $now;
		});
	$timer->start();

	$t = $i * $i * 1.375475771;
	$t -= (int) $t;

	$periodic = new EvPeriodic($base + $t, 0, NULL, function ($w, $r)
		use (&$id, &$prev, $base, $i, $t) {
			$now = ev_now();

			EvLoop::default_loop()->verify();

			echo $now >= $prev ? "" : "not ", "ok ", ++$id,
				" # p0 $i $now >= $prev\n";
			echo $now >= $base + $t ? "" : "not ", "ok ", ++$id,
				" # p1 $i $now >= $base + $t\n";

			if (! ($id % 3)) {
   				$t *= 1.0625;
         		$w->set($base + $t, 0);
         		$w->start();
			}

			$prev = $now;
		});
	$periodic->start();
}

echo "ok 1\n";
ev_run();
echo "ok 32\n";
?>
--EXPECTF--
ok 1
ok 2 %s
ok 3 %s
ok 4 %s
ok 5 %s
ok 6 %s
ok 7 %s
ok 8 %s
ok 9 %s
ok 10 %s
ok 11 %s
ok 12 %s
ok 13 %s
ok 14 %s
ok 15 %s
ok 16 %s
ok 17 %s
ok 18 %s
ok 19 %s
ok 20 %s
ok 21 %s
ok 22 %s
ok 23 %s
ok 24 %s
ok 25 %s
ok 26 %s
ok 27 %s
ok 28 %s
ok 29 %s
ok 30 %s
ok 31 %s
ok 32
