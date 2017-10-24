--TEST--
Check for bug #33
--FILE--
<?php
$time_delta = 0.05;
$time_steps = 10;
$max_time_diff = 0.008;

$reschedule_cb = function ($watcher, $now) use ($time_delta, $time_steps) {
    static $counter = 0;

    if (++$counter > $time_steps) {
        $watcher->stop();
        return $now;
    }

    return $now + $time_delta;
};

$w = new EvPeriodic(null, null, $reschedule_cb, function () {
    static $counter = 0;

    echo ++$counter, PHP_EOL;
});

$time_start = microtime(true);
Ev::run();
$time_spent = microtime(true) - $time_start;

$time_estimated = $time_delta * $time_steps;

if ($time_spent >= $time_estimated && ($time_spent - $time_estimated) <= $max_time_diff) {
    echo "ok\n";
}
?>
--EXPECT--
1
2
3
4
5
6
7
8
9
10
ok
