# -*- coding: utf-8 -*-
from distutils.core import setup

setup (name = 'aspell-python-ctypes',
	version = '1.13',
    packages         = ['pyaspell'],
	description      = "ctypes-based wrapper around GNU Aspell",
	author           = "Wojciech Muła",
	author_email     = "wojciech_mula@poczta.onet.pl",
	maintainer       = "Wojciech Muła",
	maintainer_email = "wojciech_mula@poczta.onet.pl",
    url              = "http://github.com/WojciechMula/aspell-python",
    platforms        = ["Linux", "Windows"],
    license          = "BSD (3 clauses)",
    long_description = "aspell-python - module for Python 2 & 3",
    keywords         = [
        "spellcheck",
        "aspell",
    ],
    classifiers      = [
        "Development Status :: 5 - Production/Stable",
        "License :: OSI Approved :: BSD License",
        "Programming Language :: Python",
        "Topic :: Software Development :: Libraries",
        "Topic :: Text Editors :: Text Processing",
    ],
)
