========================================================================
              aspell-python - Python bindings for GNU aspell
========================================================================

.. image:: https://travis-ci.org/WojciechMula/aspell-python.svg?branch=master
    :target: https://travis-ci.org/WojciechMula/aspell-python

.. contents::


Introduction
============

`GNU Aspell`__ is a leading spelling engine, fast, with many 
dictionaries available. Take a look at `Python Cookbook`__ ---
Ryan Kelly have collected links to all python bindings for spellers.

**aspell-python** is a Python wrapper for GNU Aspell, there
are two variants:

* ``pyaspell.py`` --- Python library, that utilize ctypes__
  module; compatible with python3;
* ``aspell-python`` --- C extension, two versions are available,
  one for Python 2.x, and Python 3.x.

C exension exist in two versions: one compatible with Python 2.x
and other with Python 3.x. 

Version for Py2 has been tested with Python 2.1, Python 2.3.4
and Python 2.4.1. Probably it works fine with all Python versions
not older than 2.0. Version for Py3 has been tested with Python 3.2.

__ http://docs.python.org/library/ctypes.html
__ http://aspell.net
__ http://code.activestate.com/recipes/117221/


License
=======

Both libraries are licensed under BSD license


Author
======

Wojciech Muła, wojciech_mula@poczta.onet.pl

Thanks to:

* **Adam Karpierz** for conviencing me to change license from GPL to BSD
  and for compiling early versions of C extension under Windows

* **Gora Mohanty** for reporting a bug.


Installation
============

To build & install module for python2.x please use script setup.2.py, i.e.::

	$ python setup.2.py build
	$ python setup.2.py install

Module for python3.x is build with setup.3.py::

	$ python3 setup.3.py build
	$ python3 setup.3.py install

Note ``python3`` name. Many Linux distributions ship both Python 2 and 3,
and use the different name to distinguish versions.


Details
-------

You need to have libaspell headers installed, Debian package is called
``libaspell-dev``, other distributions should have similar package.

In order to install **aspell-python** for all users, you must be a root.
If you are, type following command::

	$ python setup.py install

It builds package and installs ``aspell.so`` in directory
``/usr/lib/{python}/site-packages``.

If you don't have root login, you can append ``--user`` to the install
command, to install it for the current user in
``~/.local/lib/{python}/site-packages``.


Windows issues
--------------

To correctly install aspell's dictionaries in Windows some additional
work is needed. `Eric Woudenberg`__ has prepared detailed step-by-step
instruction avaiable in file `windows.rst <windows.rst>`_.

__ http://www.woudy.org/


API
===

**Aspell-python** module is seen in python under name ``aspell``. So,
**aspell-python** module is imported in following way::

	import aspell

The module provides Speller_ class, two methods, and three types
of exceptions --- all described below.


Methods
-------


.. _ConfigKeysMeth:

ConfigKeys() => dictionary
~~~~~~~~~~~~~~~~~~~~~~~~~~

Method returns a dictionary, where keys are names of configuration
item, values are 3-tuples:

* key type (``string``, ``integer``, ``boolean``, ``list``)
* default value for the key
* short description - "internal" means that aspell doesn't provide
  any description of item and you shouldn't set/change it, unless
  you know what you do


Aspell's documentation covers in details all of keys and their meaning.
Below is a list of most useful and obvious options (it is a filtered
output of ``ConfigKeys``).

::

	('data-dir', 'string', '/usr/lib/aspell-0.60', 'location of language data files')
	('dict-dir', 'string', '/usr/lib/aspell-0.60', 'location of the main word list')
	('encoding', 'string', 'ISO-8859-2', 'encoding to expect data to be in')
	('home-dir', 'string', '/home/wojtek', 'location for personal files')
	('ignore', 'integer', 1, 'ignore words <= n chars')
	('ignore-accents', 'boolean', False, 'ignore accents when checking words -- CURRENTLY IGNORED')
	('ignore-case', 'boolean', False, 'ignore case when checking words')
	('ignore-repl', 'boolean', False, 'ignore commands to store replacement pairs')
	('keyboard', 'string', 'standard', 'keyboard definition to use for typo analysis')
	('lang', 'string', 'pl_PL', 'language code')
	('master', 'string', 'pl_PL', 'base name of the main dictionary to use')
	('personal-path', 'string', '/home/wojtek/.aspell.pl_PL.pws', 'internal')
	('repl-path', 'string', '/home/wojtek/.aspell.pl_PL.prepl', 'internal')
	('run-together', 'boolean', False, 'consider run-together words legal')
	('save-repl', 'boolean', True, 'save replacement pairs on save all')
	('warn', 'boolean', True, 'enable warnings')
	('backup', 'boolean', True, 'create a backup file by appending ".bak"')
	('reverse', 'boolean', False, 'reverse the order of the suggest list')
	('suggest', 'boolean', True, 'suggest possible replacements')


Classes
-------

_`Speller`\ ()
~~~~~~~~~~~~~~

Method creates an AspellSpeller_ object which is an interface to the GNU
Aspell.

``Speller`` called with no parameters creates speller using default
configuration. If you want to change or set some parameter you can pass
pair of strings: key and it's value. One can get available keys using
ConfigKeys_.

>>> aspell.Speller("key", "value")

If you want to set more than one pair of key&value, pass the list
of pairs to the Speller().

>>> aspell.Speller( ("k1","v1"), ("k2","v2"), ("k3","v3") )


Exceptions
----------

Module defines following errors:

* AspellConfigError_,
* AspellModuleError_ and
* AspellSpellerError_.

Additionally ``TypeError`` is raised when you pass wrong parameters to
method.

_`AspellConfigError`
~~~~~~~~~~~~~~~~~~~~

Error is reported by methods Speller_ and ConfigKeys_. The most common
error is passing unknown key.

>>> s = aspell.Speller('python', '2.3')
Traceback (most recent call last):
  File "<stdin>", line 1, in ?
aspell.AspellConfigError: The key "python" is unknown.
>>>


_`AspellModuleError`
~~~~~~~~~~~~~~~~~~~~

Error is reported when module can't allocate aspell structures.


_`AspellSpellerError`
~~~~~~~~~~~~~~~~~~~~~


Error is reported by ``libaspell``.

>>> # we set master dictionary file, the file doesn't exist
>>> s = Speller('master', '/home/dictionary.rws')
Traceback (most recent call last):
  File "<stdin>", line 1, in ?
aspell.AspellSpellerError: The file "/home/dictionary.rws" can not be opened for reading.
>>>

_`AspellSpeller` Object
-----------------------

The AspellSpeller object provides interface to the aspell. It has
several methods, described below.

* ConfigKeys_
* check_
* suggest_
* addReplacement_
* addtoPersonal_
* saveAllwords_
* addtoSession_
* clearSession_
* getPersonalwordlist_
* getSessionwordlist_
* getMainwordlist_

In examples the assumption is that following code has been executed
earlier:

>>> import aspell
>>> s = aspell.Speller('lang', 'en')
>>> s
<AspellSpeller object at 0x40209050>
>>>


_`ConfigKeys`\ () => dictionary
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**New in version 1.1, changed in 1.13.**

Method returns current configuration of speller.

Result has the same meaning as ``ConfigKeys()`` procedure.


_`setConfigKey`\ (key, value)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**New in version 1.14**

Method alters configuration value. Note that depending on key's type value
is expected to be: string, boolean or integer.

Although setting all keys is possible, changes to some of them have no
effect. For example changing **lang** doesn't change current language,
it's an aspell limitation (feature).


_`check`\ (word) => boolean
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Method checks spelling of given ``word``. If ``word`` is present in
the main or personal (see addtoPersonal_) or session dictionary
(see addtoSession_) returns True, otherwise False.

>>> s.check('word') # correct word
True
>>> s.check('wrod') # incorrect
False
>>>

**New in version 1.13.**

It's possible to use operator ``in`` or ``not in`` instead
of ``check()``.

>>> 'word' in s
True
>>> 'wrod' in s
False
>>>


_`suggest` (word) => list of suggestions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Method returns a list of suggested spellings for given word.  Even if
word is correct, i.e. method check_ returned 1, action is performed.

>>> s.suggest('wrod') # we made mistake, what aspell suggests?
['word', 'Rod', 'rod', 'Brod', 'prod', 'trod', 'Wood', 'wood', 'wried']
>>>

**Warning!** ``suggest()`` in aspell 0.50 is very, very slow. I
recommend caching it's results if program calls the function several
times with the same argument.


_`addReplacement`\ (incorrect, correct) => None
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Adds a replacement pair, it affects order of words in suggest_ result.

>>> # we choose 7th word from previous result
>>> s.addReplacement('wrod', 'trod')

>>> # and the selected word appears at the 1st position
>>> s.suggest('word')
['trod', 'word', 'Rod', 'rod', 'Brod', 'prod', 'Wood', 'wood', 'wried']

If config key ``save-repl`` is ``true`` method saveAllwords_ saves
the replacement pairs to file ``~/.aspell.{lang_code}.prepl``.


_`addtoPersonal`\ (word) => None
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Adds word to the personal dictionary, which is stored in file
``~./.aspell.{lang_code}.pws``. The added words are available for
AspellSpeller object, but they remain unsaved until method saveAllwords_
is called.

::

	# personal dictionary is empty now
	$ cat ~/.aspell.en.pws
	personal_ws-1.1 en 0

	$ python
	>>> import aspell
	>>> s = aspell.Speller('lang', 'en')
	# word 'aspell' doesn't exist
	>>> s.check('aspell')
	0

	# we add it to the personal dictionary
	>>> s.addtoPersonal('aspell')

	# and now aspell knows it
	>>> s.check('aspell')
	1

	# we save personal dictionary
	>>> s.saveAllwords()

	# new word appeared in the file
	$ cat ~/.aspell.en.pws
	personal_ws-1.1 en 1
	aspell

	# check it once again
	$ python
	>>> import aspell
	>>> s = aspell.Speller('lang', 'en')

	# aspell still knows it's own name
	>>> s.check('aspell')
	1

	>>> s.check('aaa')
	0
	>>> s.check('bbb')
	0
	# add incorrect words, they shouldn't be saved
	>>> s.addtoPersonal('aaa')
	>>> s.addtoPersonal('bbb')
	>>> s.check('aaa')
	1
	>>> s.check('bbb')
	1

	# we've exit without saving, words 'aaa' and 'bbb' doesn't exists
	$ cat ~/.aspell.en.pws
	personal_ws-1.1 en 1
	aspell
	$


_`addtoSession`\ (word) => None
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Adds word to the session dictionary. The session dictionary is
volatile, it is not saved to any file. It is destroyed with
AspellSpeller_ object or when method clearSession_ is called.


_`saveAllwords`\ () => None
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Save all words from personal dictionary.


_`clearSession`\ () => None
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Clears session dictionary.

>>> import aspell
>>> s = aspell.Speller('lang', 'en')
>>> s.check('linux')
0
>>> s.addtoSession('linux')
>>> s.check('linux')
1
>>> s.clearSession()
>>> s.check('linux')
0

_`getPersonalwordlist`\ () => [list of strings]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Returns list of words from personal dictionary.

_`getSessionwordlist`\ () => [list of strings]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Returns list of words from session dictionary.

>>> s.addtoSession('aaa')
>>> s.addtoSession('bbb')
>>> s.getSessionwordlist()
['aaa', 'bbb']
>>> s.clearSession()
>>> s.getSessionwordlist()
[]
>>>


_`getMainwordlist`\ () => [list of strings]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Returns list of words from the main dictionary.


Known problems
==============

All version of aspell I've tested have the same error - calling method
getMainwordlist_ produces ``SIGKILL``. It is aspell problem and if you
really need a full list of words, use external program
``word-list-compress``.


.. list-table::

	* - method
	  - aspell 0.50.5
	  - aspell 0.60.2
	  - aspell 0.60.3

	* - ConfigKeys_
	  - ok
	  - ok
	  - ok

	* - Speller_
	  - ok
	  - ok
	  - ok

	* - check_
	  - ok
	  - ok
	  - ok

	* - suggest_
	  - ok
	  - ok
	  - ok

	* - addReplacement_
	  - ok
	  - ok
	  - ok

	* - addtoPersonal_
	  - ok
	  - ok
	  - ok

	* - saveAllwords_
	  - ok
	  - ok
	  - ok

	* - addtoSession_
	  - ok
	  - ok
	  - ok

	* - clearSession_
	  - ok
	  - AspellSpellerError_
	  - ok

	* - getPersonalwordlist_
	  - ok
	  - **SIGKILL**
	  - ok

	* - getSessionwordlist_
	  - ok
	  - **SIGKILL**
	  - ok

	* - getMainwordlist_
	  - **SIGKILL**
	  - **SIGKILL**
	  - **SIGKILL**

Character encoding
==================

Aspell uses 8-bit encoding. The encoding depend on dictionary setting and 
is stored in key ``encoding``. One can obtain this key using speller's
ConfigKeys_.

If your application uses other encoding than aspell, the translation is
needed. Here is a sample session (polish dictionary is used).

>>> import aspell
>>> s=aspell.Speller('lang', 'pl')
>>> 
>>> s.ConfigKeys()['encoding']
[('encoding', 'string', 'iso8859-2')]
>>> enc =s.ConfigKeys()['encoding'][2]
>>> enc  # dictionary encoding
'iso8859-2'
>>> word # encoding of word is utf8
# 'gżegżółka' means in some polish dialects 'cuckoo'
'g\xc5\xbceg\xc5\xbc\xc3\xb3\xc5\x82ka'
>>> s.check(word)
0
>>> s.check( unicode(word, 'utf-8').encode(enc) )
1

