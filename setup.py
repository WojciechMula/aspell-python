from distutils.core import setup, Extension

module1 = Extension('aspell', libraries = ['aspell'], library_dirs = ['/usr/local/lib/'], sources = ['aspell.c'])

setup (name = 'GNU Aspell wrapper',
	version = '1.0',
	ext_modules = [module1])
