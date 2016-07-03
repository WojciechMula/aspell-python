# -*- coding: utf-8 -*-
from distutils.core import setup, Extension

def get_include_dirs():
    import sys
    if sys.platform.startswith('darwin'):
        return ['/opt/local/include'] # dir used by MacPorts
    else:
        return []

module = Extension('aspell',
	libraries = ['aspell'],
	library_dirs = ['/usr/local/lib/'],
    include_dirs = get_include_dirs(),
	sources = ['aspell.2.c']
)

setup (name = 'aspell-python-py2',
	version = '1.15',
	ext_modules = [module],

	description      = "Wrapper around GNU Aspell for Python 2",
	author           = "Wojciech Muła",
	author_email     = "wojciech_mula@poczta.onet.pl",
	maintainer       = "Wojciech Muła",
	maintainer_email = "wojciech_mula@poczta.onet.pl",
    url              = "http://github.com/WojciechMula/aspell-python",
    platforms        = ["Linux", "Windows"],
    license          = "BSD (3 clauses)",
    long_description = "aspell-python - C extension for Python 2",
    classifiers      = [
        "Development Status :: 5 - Production/Stable",
        "License :: OSI Approved :: BSD License",
        "Programming Language :: C",
        "Topic :: Software Development :: Libraries",
        "Topic :: Text Editors :: Text Processing",
    ],
)
