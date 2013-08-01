--TEST--
Check for EvTimer and EvPeriodic created for custom loop instance
--FILE--
<?php 
#error_reporting(0);

$l = new EvLoop();

$fudge = 0.02;

$id = 1;

$base = $l->now();
$prev = $l->now();

$timer = array();
$periodic = array();

for ($i = 1; $i <= /*125*/25; ++$i) {
	$t = $i * $i * 1.735435336;
	$t -= (int) $t;
	$timer[] = $l->timer($t, 0, function ($w, $r)
		use (&$id, &$prev, $base, $i, $t, $fudge) {
			$now = $w->getLoop()->now();

			echo $now + $fudge >= $prev      ? "" : "not ", "ok ", ++$id,
				" # t0 $i $now + $fudge >= $prev\n";
			echo $now + $fudge >= $base + $t ? "" : "not ", "ok ", ++$id,
				" # t1 $i $now + $fudge >= $base + $t\n";

			if (! ($id % 3)) {
				$t *= 0.0625;
				$w->set($t, 0);
				$w->start();
				$t = $now + $t - $base;
			}

			$prev = $now;
		});

	$t = $i * $i * 1.375475771;
	$t -= (int) $t;
	$periodic[] = $l->periodic($base + $t, 0, NULL, function ($w, $r)
		use (&$id, &$prev, $base, $i, $t, $fudge) {
			$now = $w->getLoop()->now();

			echo $now >= $prev      ? "" : "not ", "ok ", ++$id,
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
}

#Ev::run();
print "ok 1\n";
$l->run();
print "ok 152\n";

$timer = null;
$periodic = null;
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
ok 32 %s
ok 33 %s
ok 34 %s
ok 35 %s
ok 36 %s
ok 37 %s
ok 38 %s
ok 39 %s
ok 40 %s
ok 41 %s
ok 42 %s
ok 43 %s
ok 44 %s
ok 45 %s
ok 46 %s
ok 47 %s
ok 48 %s
ok 49 %s
ok 50 %s
ok 51 %s
ok 52 %s
ok 53 %s
ok 54 %s
ok 55 %s
ok 56 %s
ok 57 %s
ok 58 %s
ok 59 %s
ok 60 %s
ok 61 %s
ok 62 %s
ok 63 %s
ok 64 %s
ok 65 %s
ok 66 %s
ok 67 %s
ok 68 %s
ok 69 %s
ok 70 %s
ok 71 %s
ok 72 %s
ok 73 %s
ok 74 %s
ok 75 %s
ok 76 %s
ok 77 %s
ok 78 %s
ok 79 %s
ok 80 %s
ok 81 %s
ok 82 %s
ok 83 %s
ok 84 %s
ok 85 %s
ok 86 %s
ok 87 %s
ok 88 %s
ok 89 %s
ok 90 %s
ok 91 %s
ok 92 %s
ok 93 %s
ok 94 %s
ok 95 %s
ok 96 %s
ok 97 %s
ok 98 %s
ok 99 %s
ok 100 %s
ok 101 %s
ok 102 %s
ok 103 %s
ok 104 %s
ok 105 %s
ok 106 %s
ok 107 %s
ok 108 %s
ok 109 %s
ok 110 %s
ok 111 %s
ok 112 %s
ok 113 %s
ok 114 %s
ok 115 %s
ok 116 %s
ok 117 %s
ok 118 %s
ok 119 %s
ok 120 %s
ok 121 %s
ok 122 %s
ok 123 %s
ok 124 %s
ok 125 %s
ok 126 %s
ok 127 %s
ok 128 %s
ok 129 %s
ok 130 %s
ok 131 %s
ok 132 %s
ok 133 %s
ok 134 %s
ok 135 %s
ok 136 %s
ok 137 %s
ok 138 %s
ok 139 %s
ok 140 %s
ok 141 %s
ok 142 %s
ok 143 %s
ok 144 %s
ok 145 %s
ok 146 %s
ok 147 %s
ok 148 %s
ok 149 %s
ok 150 %s
ok 151 %s
ok 152
