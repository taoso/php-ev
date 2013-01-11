EvAsync - `ev_async`
===================

For connectors like libeio + libev
<http://pod.tst.eu/http://cvs.schmorp.de/libeio/eio.pod#INITIALISATION_INTEGRATION>


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


vim: ft=markdown
