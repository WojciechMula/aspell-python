========================================================================
                      Windows troubleshooting
========================================================================

:Author: **Eric Woudenberg** <eaw(at)woudy(dot)org>
:Date:   2015-08-29
:Updated: 2016-03-17

.. contents::

The problem
------------------------------------------------------------------------

Prerequisites
~~~~~~~~~~~~~

Cygwin Aspell 0.60 or some other Windows Aspell 0.60 installed.
Python 2.6 or 2.7.

Steps to reproduce problem
~~~~~~~~~~~~~~~~~~~~~~~~~~

1) Run pre-packaged Windows aspell-python ".whl" installer (from: 
   http://www.lfd.uci.edu/~gohlke/pythonlibs/#aspell-python) with e.g. "pip 
   install aspell_python‑1.13‑cp27‑none‑win32.whl"

2) Try to instantiate Speller, e.g.::

    $ python -c "import aspell; aspell.Speller()"
    Traceback (most recent call last):
      File "<string>", line 1, in <module>
    aspell.AspellSpellerError: No word lists can be found for the language "en_US".  

3) Get error "No word lists can be found".

The problem (that took me so long to figure out) is that all the aspell 
dictionary installers don't seem to know about where aspell-python 
expects to find the dictionaries and simply assume you're installing the 
dictionary for the command line aspell.

The solution
------------------------------------------------------------------------

1) Get modern 0.60 dictionaries from e.g. 
   ftp://ftp.gnu.org/gnu/aspell/dict/en/aspell6-en-2015.04.24-0.tar.bz2

2) Unpack and run the configure script. Make a note of the location it 
   finds for the **Dictionary** and **Data** dirs::

    $ ./configure
    Finding Dictionary file location ... /usr/lib/aspell-0.60
    Finding Data file location ... /usr/lib/aspell-0.60  

3) Run the dictionary's "make" and "make install" scripts::

    $ make
    /usr/bin/prezip-bin -d < en-common.cwl | /usr/bin/aspell --lang=en create master ./en-common.rws
    /usr/bin/prezip-bin -d < en-variant_0.cwl | /usr/bin/aspell --lang=en create master ./en-variant_0.rws
    /usr/bin/prezip-bin -d < en-variant_1.cwl | /usr/bin/aspell --lang=en create master ./en-variant_1.rws
    /usr/bin/prezip-bin -d < en-variant_2.cwl | /usr/bin/aspell --lang=en create master ./en-variant_2.rws
    /usr/bin/prezip-bin -d < en-w_accents-only.cwl | /usr/bin/aspell --lang=en create master ./en-w_accents-only.rws
    /usr/bin/prezip-bin -d < en-wo_accents-only.cwl | /usr/bin/aspell --lang=en create master ./en-wo_accents-only.rws
    /usr/bin/prezip-bin -d < en_CA-variant_0.cwl | /usr/bin/aspell --lang=en create master ./en_CA-variant_0.rws
    /usr/bin/prezip-bin -d < en_CA-variant_1.cwl | /usr/bin/aspell --lang=en create master ./en_CA-variant_1.rws
    /usr/bin/prezip-bin -d < en_CA-w_accents-only.cwl | /usr/bin/aspell --lang=en create master ./en_CA-w_accents-only.rws
    /usr/bin/prezip-bin -d < en_CA-wo_accents-only.cwl | /usr/bin/aspell --lang=en create master ./en_CA-wo_accents-only.rws
    /usr/bin/prezip-bin -d < en_GB-ise-w_accents-only.cwl | /usr/bin/aspell --lang=en create master ./en_GB-ise-w_accents-only.rws
    /usr/bin/prezip-bin -d < en_GB-ise-wo_accents-only.cwl | /usr/bin/aspell --lang=en create master ./en_GB-ise-wo_accents-only.rws
    /usr/bin/prezip-bin -d < en_GB-ize-w_accents-only.cwl | /usr/bin/aspell --lang=en create master ./en_GB-ize-w_accents-only.rws
    /usr/bin/prezip-bin -d < en_GB-ize-wo_accents-only.cwl | /usr/bin/aspell --lang=en create master ./en_GB-ize-wo_accents-only.rws
    /usr/bin/prezip-bin -d < en_GB-variant_0.cwl | /usr/bin/aspell --lang=en create master ./en_GB-variant_0.rws
    /usr/bin/prezip-bin -d < en_GB-variant_1.cwl | /usr/bin/aspell --lang=en create master ./en_GB-variant_1.rws
    /usr/bin/prezip-bin -d < en_US-w_accents-only.cwl | /usr/bin/aspell --lang=en create master ./en_US-w_accents-only.rws
    /usr/bin/prezip-bin -d < en_US-wo_accents-only.cwl | /usr/bin/aspell --lang=en create master ./en_US-wo_accents-only.rws 
    
    $ make install
    mkdir -p /usr/lib/aspell-0.60/
    cp en-common.rws en-variant_0.rws en-variant_1.rws en-variant_2.rws 
    en-w_accents-only.rws en-wo_accents-only.rws en_CA-variant_0.rws 
    en_CA-variant_1.rws en_CA-w_accents-only.rws en_CA-wo_acc
    ents-only.rws en_GB-ise-w_accents-only.rws 
    en_GB-ise-wo_accents-only.rws en_GB-ize-w_accents-only.rws 
    en_GB-ize-wo_accents-only.rws en_GB-variant_0.rws en_GB-variant_1.rws 
    en_US-w_accents-on
    ly.rws en_US-wo_accents-only.rws american.alias 
    american-variant_0.alias american-variant_1.alias 
    american-w_accents.alias american-wo_accents.alias british.alias 
    british-ise.alias british-i
    se-w_accents.alias british-ise-wo_accents.alias british-ize.alias 
    british-ize-w_accents.alias british-ize-wo_accents.alias 
    british-variant_0.alias british-variant_1.alias british-w_accents.a
    lias british-wo_accents.alias canadian.alias canadian-variant_0.alias 
    canadian-variant_1.alias canadian-w_accents.alias 
    canadian-wo_accents.alias en.multi en-variant_0.multi en-variant_1.mul
    ti en-variant_2.multi en-w_accents.multi en-wo_accents.multi 
    en_CA.multi en_CA-variant_0.multi en_CA-variant_1.multi 
    en_CA-w_accents.multi en_CA-wo_accents.multi en_GB.multi en_GB-ise.multi
    en_GB-ise-w_accents.multi en_GB-ise-wo_accents.multi en_GB-ize.multi 
    en_GB-ize-w_accents.multi en_GB-ize-wo_accents.multi 
    en_GB-variant_0.multi en_GB-variant_1.multi en_GB-w_accents.multi en
    _GB-wo_accents.multi en_US.multi en_US-variant_0.multi 
    en_US-variant_1.multi en_US-w_accents.multi en_US-wo_accents.multi 
    english.alias english-variant_0.alias english-variant_1.alias englis
    h-variant_2.alias english-w_accents.alias english-wo_accents.alias 
    /usr/lib/aspell-0.60/
    cd /usr/lib/aspell-0.60/ && chmod 644 en-common.rws en-variant_0.rws 
    en-variant_1.rws en-variant_2.rws en-w_accents-only.rws 
    en-wo_accents-only.rws en_CA-variant_0.rws en_CA-variant_1.rws en
    _CA-w_accents-only.rws en_CA-wo_accents-only.rws 
    en_GB-ise-w_accents-only.rws en_GB-ise-wo_accents-only.rws 
    en_GB-ize-w_accents-only.rws en_GB-ize-wo_accents-only.rws 
    en_GB-variant_0.rws en_
    GB-variant_1.rws en_US-w_accents-only.rws en_US-wo_accents-only.rws 
    american.alias american-variant_0.alias american-variant_1.alias 
    american-w_accents.alias american-wo_accents.alias britis
    h.alias british-ise.alias british-ise-w_accents.alias 
    british-ise-wo_accents.alias british-ize.alias 
    british-ize-w_accents.alias british-ize-wo_accents.alias 
    british-variant_0.alias british-
    variant_1.alias british-w_accents.alias british-wo_accents.alias 
    canadian.alias canadian-variant_0.alias canadian-variant_1.alias 
    canadian-w_accents.alias canadian-wo_accents.alias en.multi
    en-variant_0.multi en-variant_1.multi en-variant_2.multi 
    en-w_accents.multi en-wo_accents.multi en_CA.multi 
    en_CA-variant_0.multi en_CA-variant_1.multi en_CA-w_accents.multi 
    en_CA-wo_accents
    .multi en_GB.multi en_GB-ise.multi en_GB-ise-w_accents.multi 
    en_GB-ise-wo_accents.multi en_GB-ize.multi en_GB-ize-w_accents.multi 
    en_GB-ize-wo_accents.multi en_GB-variant_0.multi en_GB-varia
    nt_1.multi en_GB-w_accents.multi en_GB-wo_accents.multi en_US.multi 
    en_US-variant_0.multi en_US-variant_1.multi en_US-w_accents.multi 
    en_US-wo_accents.multi english.alias english-variant_0.a
    lias english-variant_1.alias english-variant_2.alias 
    english-w_accents.alias english-wo_accents.alias
    mkdir -p /usr/lib/aspell-0.60/
    cp en.dat en_phonet.dat en_affix.dat /usr/lib/aspell-0.60/
    cd /usr/lib/aspell-0.60/ && chmod 644 en.dat en_phonet.dat en_affix.dat  

4) Get the location where aspell-python is expecting the dictionaries to 
   be installed::

    $ python -c "import aspell, sys; [sys.stdout.write(i[0] + ' ' + 
    str(i[2]) + '\n') for i in aspell.ConfigKeys()]" | grep -e -dir  
    
    actual-dict-dir C:\Documents and Settings\All Users\Application Data\Aspell\Dictionaries/
    conf-dir C:\Documents and Settings\All Users\Application Data\Aspell\
    data-dir C:\Documents and Settings\All Users\Application Data\Aspell\Data
    dict-dir C:\Documents and Settings\All Users\Application > Data\Aspell\Dictionaries
    home-dir C:\Documents and Settings\All Users\Application Data\Aspell\
    local-data-dir C:\Documents and Settings\All Users\Application Data\Aspell\Dictionaries/
    personal-dir C:\Documents and Settings\All Users\Application Data\Aspell\Personal  

5) Make the target directory and copy all the freshly installed 
   directories there::

    $ mkdir 'C:\Documents and Settings\All Users\Application Data\Aspell'
    $ cp -r /usr/lib/aspell-0.60 'C:\Documents and Settings\All Users\Application Data\Aspell'/Data
    $ cp -r /usr/lib/aspell-0.60 'C:\Documents and Settings\All Users\Application Data\Aspell'/Dictionaries  

6) Now the aspell-python package will correctly instantiate a Speller::

    $ python -c "import aspell; aspell.Speller()"  

