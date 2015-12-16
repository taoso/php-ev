--TEST--
Check for bug #18
--FILE--
<?php
$f = tmpfile();
$io = new EvIo($f, Ev::READ, function(\EvIo $io) {
	echo '2', PHP_EOL;
	var_dump($io->fd);
	var_dump($io->fd);
	var_dump($io->fd);
	echo '3', PHP_EOL;
	Ev::stop();
});
echo '1', PHP_EOL;
var_dump($io->fd);
var_dump($io->fd);
var_dump($io->fd);
\Ev::run();
var_dump($io->fd);
var_dump($io->fd);
var_dump($io->fd);
echo '4', PHP_EOL;
?>
--EXPECTF--
1
resource(%d) of type (stream)
resource(%d) of type (stream)
resource(%d) of type (stream)
2
resource(%d) of type (stream)
resource(%d) of type (stream)
resource(%d) of type (stream)
3
resource(%d) of type (stream)
resource(%d) of type (stream)
resource(%d) of type (stream)
4
