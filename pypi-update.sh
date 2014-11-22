#!/bin/bash

set -e

options="--format=bztar --metadata-check upload"
pydist="python setup.py sdist $options"

function py3() {
    echo preparing package for Python 3 extension
    rm -f setup.py
    ln setup.3.py setup.py

    $pydist
}

function py2() {
    echo preparing package for Python 2 extension
    rm -f setup.py
    ln setup.2.py setup.py

    $pydist
}

function ctypes() {
    echo preparing package for ctypes module
    rm -f setup.py
    ln setup.ctypes.py setup.py

    $pydist
}

py3
py2
ctypes
