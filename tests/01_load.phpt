--TEST--
Check for ev presence
--SKIPIF--
<?php if (!extension_loaded("ev")) print "skip"; ?>
--FILE--
<?php 
echo "ev extension is available";
?>
--EXPECT--
ev extension is available
