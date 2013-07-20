--TEST--
Check for watcher destruction depending on it's data property value
--FILE--
<?php
class indicator {
	public $i;
	public function __construct($i) {
		$this->i = $i;
	}
	public function __destruct() {
		echo $this->i;
	}
}

function test() {
	$loop = new EvLoop();
	$i1 = new indicator(1);
	$i2 = new indicator(2);

	// store data as ref to object
	$t1 = $loop->timer(1, 0, function () {});
	$t1->data = &$i1;

	// store data as object
	$t2 = $loop->timer(1, 0, function () {});
	$t2->data = $i2;

	echo "0";
	$t1->stop();
	$t2->stop();
	$loop->stop();

	$i1     = null;
	$i2     = null;
	$timer  = null;
	$timer2 = null;
	$loop   = null;
	echo "3";
}

test();
?>
--EXPECT--
0123
