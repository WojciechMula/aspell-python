from distutils.core import setup, Extension
import distutils.sysconfig as ds

module1 = Extension('aspell', libraries = ['aspell'], library_dirs = ['/usr/local/lib/'], sources = ['aspell.c'])

setup (name = 'GNU Aspell wrapper',
	version = '1.12',
	ext_modules = [module1])
