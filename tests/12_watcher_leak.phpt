--TEST--
Check for EvWatcher object destructors(leaks)
--FILE--
<?php
ini_set('memory_limit', '1M');

$limit = 100000;

$callback = function() {
	static $i = 0;
	echo "cb ", ++$i, PHP_EOL;
};

// Create and destroy EvTimer objects in loop.
// EvTimer's constructor starts the watcher automatically.
// Thus, eventually only one timer should fire.
$n = 0;
while (++$n < $limit) {
    $timer = new EvTimer(-1, 0, $callback);
    //Ev::run(Ev::RUN_NOWAIT | Ev::RUN_ONCE);
}
Ev::run();
echo $n, PHP_EOL;

// Create stopped timers and destroy them in loop.
// No timer should fire even after Ev::run() call.
// We're checking whether stopped watchers exhaust the memory.
$n = 0;
while (++$n < $limit) {
    $timer = EvTimer::createStopped(-1, 0, $callback);
    //Ev::run(Ev::RUN_NOWAIT | Ev::RUN_ONCE);
}
Ev::run();
echo $n;
?>
--EXPECT--
cb 1
100000
100000
