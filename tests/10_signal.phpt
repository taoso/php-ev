--TEST--
Check for EvSignal functionality
--SKIPIF--
<?php
if (!extension_loaded("pcntl")) print "skip pcntl extension is not loaded";
if (!extension_loaded("posix")) print "skip posix extension is not loaded";
?>
--FILE--
<?php 
error_reporting(0);

echo "ok 1\n";

$sig1 = new EvSignal(SIGUSR1, function () { echo "ok 8\n"; });

echo "ok 2\n";

$loop = new EvLoop();

echo "ok 3\n";

$usr2_1 = $loop->signal(SIGUSR2, function ($w) { echo "ok 6\n"; $w->stop(); });
$usr2_2 = $loop->signal(SIGUSR1, function () { echo "not ok 6\n"; });

$sig2 = new EvSignal(SIGUSR2, function () { echo "not ok 8\n"; });

echo "ok 4\n";

posix_kill(posix_getpid(), SIGUSR1);
posix_kill(posix_getpid(), SIGUSR2);

echo "ok 5\n";

$loop->run();

echo "ok 7\n";

Ev::run(Ev::RUN_ONCE);

echo "ok 9\n";

// Re-bind
$usr2 = new EVSignal(SIGUSR2, function () { echo "ok 11\n"; });
$sig1->stop();
$usr1_2 = $loop->signal(SIGUSR1, function () { echo "ok 13\n"; });

echo "ok 10\n";

posix_kill(posix_getpid(), SIGUSR2);

Ev::run(Ev::RUN_NOWAIT);

echo "ok 12\n";

posix_kill(posix_getpid(), SIGUSR1);

$loop->run(Ev::RUN_NOWAIT);

echo "ok 14\n";
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
ok 12
ok 13
ok 14
