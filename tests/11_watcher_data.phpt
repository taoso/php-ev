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

	$t1 = $loop->timer(1, 0, function () {});
	$t1->data = &$i1;

	$t2 = $loop->timer(1, 0, function () {});
	$t2->data = $i2;

	echo "0";
	$t1->stop();
	$t2->stop();
	$loop->stop();
	echo ':';

	$i1   = null; unset($i1);
	$i2   = null; unset($i2);
	$t1   = null; unset($t1);
	$t2   = null; unset($t2);
	$loop = null; unset($loop);
	echo "3";
}

test();
?>
--EXPECT--
0:123
