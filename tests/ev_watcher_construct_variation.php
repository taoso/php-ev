<?php
function cb() {
	return FALSE;
}

$fd   = fopen("listen_to_me", "r");

$loop = new EvLoop();
$io_watcher = new EvIo($fd, EV_READ, $loop, "cb");
var_dump($io_watcher);
exit();

fclose($fd);
?>
