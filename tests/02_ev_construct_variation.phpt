--TEST--
Check for constructor and factory methods' behaviour
--FILE--
<?php
function cb() {
	return FALSE;
}

$prev_data = "prev_data";
$third_data = "new_data";

// Passing data var to constructor
$loop = new EvLoop(0, "cb", $prev_data);
var_dump($loop->data);

// data property read/write
$loop->data = "new data";
var_dump($loop->data);

// rewrite data property with string literal
$loop->data = "new new data";
var_dump($loop->data);

// rewrite data property with a variable
$loop->data = $third_data;
var_dump($loop->data);

// Multiple attempts to create the default loop
$loop2 = EvLoop::default_loop();
$loop2 = EvLoop::default_loop();
$loop2 = EvLoop::default_loop();

// Overwriting previously created loop with the default loop
$loop = EvLoop::default_loop();

// Overwriting previously created default loop with new loop
$loop = new EvLoop(EVFLAG_AUTO);
// Should be NULL
var_dump($loop->data);
?>
--CLEAN--
--EXPECTF--
string(9) "prev_data"
string(8) "new data"
string(12) "new new data"
string(8) "new_data"

Warning: EvLoop::default_loop(): Returning previously created default loop in %s on line %d

Warning: EvLoop::default_loop(): Returning previously created default loop in %s on line %d

Warning: EvLoop::default_loop(): Returning previously created default loop in %s on line %d
NULL
