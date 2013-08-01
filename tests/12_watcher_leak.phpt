<?php

error_reporting(E_ALL);
ini_set('memory_limit', '5M');

$i = 0;
$startedAt = microtime(TRUE);
$callback = function() use (&$i, $startedAt) {
    if (!(++$i % 2500)) {
        echo vsprintf("%d\t%d\t%d\t%.4f\t%d\n", $last = [
            "job" => $i,
            "mem" => memory_get_usage(),
            "real" => memory_get_usage(true),
            "runtime" => $runtime = (microtime(true) - $startedAt),
            "jps" => ceil($i / $runtime)
        ]);
    }
};

$n = 0;
while (++$n < 10000) {
    //$timer = EvTimer::createStopped(-1, 0, $callback);
    $timer = new EvTimer(-1, 0, $callback);
    //Ev::run(Ev::RUN_NOWAIT | Ev::RUN_ONCE);

}
// What is the expected behaviour?
// timers are created and destructed continuosly in the loop.
// So very few of them will be actually invoked(if any).
// Isn't it?
Ev::run();
echo "n: $n\n";
