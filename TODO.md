Add EvLoop::setInvokePendingCallback(`ev_set_invoke_pending_cb`)?
=================================================================

Who needs it in PHP :/?

Clone  handlers
===============

Add `__clone` handlers for all object types? Is it really needed?

Let's assume we are cloning a loop having a set of active watchers. Some of them
may finish before `__clone` handler did it's job. Because of such kind of issues
(and many others) we probably don't need `__clone` handler for EvLoop.

However, sometimes clone handler may be useful to copy entire watcher with it's
settings and bind to another event loop.

Will wait for user requests, or bug reports.

libevent buffer functionality?
=============================

	<?php
	ini_set('display_errors', 'On');
	error_reporting(-1);
	$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
	socket_set_nonblock($socket);
	socket_connect($socket, 'phpdaemon.net', 843);
	$eventBase = event_base_new();
	$buffer = event_buffer_new(
        	$socket,
        	function ($stream, $arg) use (&$buffer) {
                	echo "Read\n";
                	var_dump(event_buffer_read($buffer, 1024));
        	},
        	function ($stream, $arg) use (&$buffer)  {
                	echo "Write\n";
                	static $first = true;
                	if ($first) {
                        	event_buffer_write($buffer, "<policy-file-request/>\x00");
                        	$first = false;
                	}
        	},
        	function ($stream, $arg)  use (&$buffer)  {
                	echo "failure\n";
        	}
	);
 	 
	event_buffer_base_set($buffer, $eventBase);
	event_buffer_enable($buffer, EV_READ | EV_WRITE | EV_TIMEOUT | EV_PERSIST);
	event_base_loop($eventBase);

SAMPLE OUTPUT
------

	[root@gf-home-server phpdaemon]# php -q test.php

	Warning: socket_connect(): unable to connect [115]: Operation now in progress in /home/web/phpdaemon/test.php on line 6
	Write
	Write
	Read
	string(268) "<?xml version="1.0"?>
	<!DOCTYPE cross-domain-policy SYSTEM "http://www.macromedia.com/xml/dtds/cross-domain-policy.dtd">
	<cross-domain-policy>
 	 <allow-access-from domain="*" to-ports="*"/>
 	 <site-control permitted-cross-domain-policies="all"/>
	</cross-domain-policy>
	"
	failure
	[root@gf-home-server phpdaemon]# php -q test.php

	Warning: socket_connect(): unable to connect [115]: Operation now in progress in /home/web/phpdaemon/test.php on line 6
	Write
	Write
	Read
	string(268) "<?xml version="1.0"?>
	<!DOCTYPE cross-domain-policy SYSTEM "http://www.macromedia.com/xml/dtds/cross-domain-policy.dtd">
	<cross-domain-policy>
 	 <allow-access-from domain="*" to-ports="*"/>
 	 <site-control permitted-cross-domain-policies="all"/>
	</cross-domain-policy>
	"
	failure

But should it be in ev extension?! Sockets' buffering should be somewhere in
sockets extension, isn't it?  libevent functionality should be in libevent
itself. libevent extension currently doesn't support libevent 2. Consider
forking the project, or even writing something new.


vim: ft=markdown
