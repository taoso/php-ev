--TEST--
Check for EvLoop class properties
--FILE--
<?php 
function my_print_props($l) {
	echo "backend:";          var_dump($l->backend);
	echo "data:";             var_dump($l->data);
	echo "is_default_loop:";  var_dump($l->is_default_loop);
	echo "iteration:";        var_dump($l->iteration);
	echo "pending:";          var_dump($l->pending);
	echo "io_interval:";      var_dump($l->io_interval);
	echo "timeout_interval:"; var_dump($l->timeout_interval);
	echo "depth:";            var_dump($l->depth);
}

function cb($w, $r) {
	return FALSE;
}

$l                   = EvLoop::defaultLoop();
$l->data             = "ld";
$l->io_interval      = 1.1;
$l->timeout_interval = 1.2;
my_print_props($l);


$flags               = Ev::FLAG_NOENV | Ev::FLAG_NOINOTIFY; /* 17825792 */
$l                   = new EvLoop($flags, "cb", "data", 1.1, 1.2);
$l->data             = "ld2";
$l->io_interval      = 2.1;
$l->timeout_interval = 2.2;

var_dump($flags);
my_print_props($l);
?>
--EXPECTF--
backend:int(%d)
data:string(2) "ld"
is_default_loop:bool(true)
iteration:int(0)
pending:int(0)
io_interval:float(1.1)
timeout_interval:float(1.2)
depth:int(0)
int(17825792)
backend:int(%d)
data:string(3) "ld2"
is_default_loop:bool(false)
iteration:int(0)
pending:int(0)
io_interval:float(2.1)
timeout_interval:float(2.2)
depth:int(0)
