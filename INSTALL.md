INSTALLATION OF EV PECL EXTENSION
==================================

Currently GNU/Linux platforms supported only.


AUTOMATIC INSTALLATION
----------------------

To download and install ev automatically you need the following commands

	# pecl install ev

for stable release(if available), and the following for beta release:

	# pecl install ev-beta

If you have the package archive, unpack it and run: 

	# pecl install package2.xml

Note, these commands(started with `#`) most likely need root priveleges.

libev is embedded in ev. So you don't need to install the library separately.


MANUAL INSTALLATION
-------------------

Checkout the project or download it as archive. In the package directory run: 

	$ phpize 
	$ ./configure --with-ev
	$ make 

Additionally, you may take advantage of the following flags:

	--enable-ev-debug         Enable ev internal debugging
	--enable-ev-libevent-api  Enable libevent compatibility API support

Optionally test the extension:

	$ make test

Do install with root priveleges:

	# make install

In php.ini, or some other configuration like
</usr/local/etc/php/conf.d/ev.ini> write:

	extension=ev.so


FINALLY
------

Restart the SAPI server(Apache, PHP-FPM etc.), if any.

vim: ft=markdown tw=80
