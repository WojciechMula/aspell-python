.SUFFIXES:
.PHONY: work test test_py2 test_py3 clean

export PYTHONPATH := .:$(PYTHONPATH):$(PATH)

test: test_py2 test_py3

test_py3:
	python3 setup.3.py build_ext --inplace
	python3 test/unittests.py

test_py2:
	python2 setup.2.py build_ext --inplace
	python2 test/unittests.py

clean:
	rm -f *.so
