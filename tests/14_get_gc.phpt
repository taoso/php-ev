--TEST--
Check for get_gc property handler
--FILE--
<?php
class x {
        public $t = null;
 
        public function __construct() {
                $this->t = new EvTimer(0, .5, function () { });
        }
}
echo "1";
new x();
gc_collect_cycles();
echo "2"; // check whether it segfaults here
?>
--EXPECT--
12
