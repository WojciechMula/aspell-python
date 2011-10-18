# -*- coding: utf-8 -*-
from distutils.core import setup, Extension

module = Extension('aspell',
	libraries = ['aspell'],
	library_dirs = ['/usr/local/lib/'],
	sources = ['aspell.2.c']
)

setup (name = 'aspell-python',
	version = '1.12',
	ext_modules = [module],

	description      = "Wrapper around GNU Aspell for python2.x",
	author           = "Wojciech Muła",
	author_email     = "wojciech_mula@poczta.onet.pl",
	maintainer       = "Wojciech Muła",
	maintainer_email = "wojciech_mula@poczta.onet.pl",
	url              = "http://0x80.pl/proj/aspell-python",
	classifiers      = [],
    platforms        = "Linux, Windows",
	license          = "BSD (3 clauses)",

	long_description = """
Aspell-python consist C extension modules both for Py2 and
Py3k (Py3 is maintained), and also pure python module using
ctypes to communicate with aspell library.
""",
)
