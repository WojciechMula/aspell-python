import unittest
import os
import sys

# import tested module
arg = '--ctypes-module'
if arg in sys.argv:
	import pyaspell as aspell 
else:
	import aspell


class TestBase(unittest.TestCase):
	def setUp(self):
		# select english dictionary and set name of personal word list
		self.speller = aspell.Speller(
			('lang', 'en'),
			('personal', '__unittest__.rws'),
		)

		# get current speller configuration
		config = self.speller.ConfigKeys()

		self.config = dict((name, value) for name, (type, value, desc) in config.items())
		
		# polish words (cat, tree, spring) not existing in english dict
		self.polish_words = ['kot', 'drzewo', 'wiosna']


class TestCheckMethod(TestBase):
	"test check method"

	def test_ok(self):
		words = ['word', 'flower', 'tree', 'rock', 'cat', 'winter']
		for word in words:
			self.assertTrue(self.speller.check(word))

	def test_false(self):
		words = ['misteke', 'zo', 'tre', 'bicyle']
		for word in words:
			self.assertFalse(self.speller.check(word))


	def test_in(self):
		words = ['word', 'flower', 'tree', 'rock', 'cat', 'winter']
		for word in words:
			self.assertTrue(word in self.speller)

	def test_notin(self):
		words = ['misteke', 'zo', 'tre', 'bicyle']
		for word in words:
			self.assertFalse(word in self.speller)
			self.assertTrue(word not in self.speller)


class TestSuggestMethod(TestBase):
	def test(self):
		pairs = {
			'wrod'	: 'word',
			'tre'	: 'tree',
			'xoo'	: 'zoo',
		}

		for incorrect, correct in pairs.items():
			sug = self.speller.suggest(incorrect)
			self.assertTrue(correct in sug)


class TestAddReplacementMethod(TestBase):
	def test(self):
		"addReplacement affects on order of words returing by suggest"

		wrong		= 'wrod'
		correct_def = 'word'	# first suggestion in default order
		correct		= 'trod'	# first suggestion after altering order

		sug = self.speller.suggest(wrong)
		self.assertEqual(sug[0], correct_def)

		self.speller.addReplacement(wrong, correct)

		sug = self.speller.suggest(wrong)
		self.assertEqual(sug[0], correct)


class TestaddtoSession(TestBase):
	def test(self):
		
		# aspell dosn't know any of these words
		for word in self.polish_words:
			self.assertFalse(self.speller.check(word))

		# now, we add them to session dictionary
		for word in self.polish_words:
			self.speller.addtoSession(word)

		# and check once again
		for word in self.polish_words:
			self.assertTrue(self.speller.check(word))


class TestSessionwordlist(TestBase):
	def all_correct(self):
		for word in self.polish_words:
			self.assertTrue(self.speller.check(word))

	def all_incorrect(self):
		for word in self.polish_words:
			self.assertFalse(self.speller.check(word))

	def test1(self):
		"by default session dict is empty"
		swl = self.speller.getSessionwordlist()
		self.assertEqual(swl, [])

	def test2(self):
		"fill session dict with some words, then clear"

		# fill
		for word in self.polish_words:
			self.speller.addtoSession(word)
		
		# test - all correct
		swl = self.speller.getSessionwordlist()
		self.assertEqual(set(swl), set(self.polish_words))

		# clear
		self.speller.clearSession()
		swl = self.speller.getSessionwordlist()

		# empty - none correct
		self.assertEqual(set(swl), set())

	def test3(self):
		self.all_incorrect()
		
		for word in self.polish_words:
			self.speller.addtoSession(word)

		self.all_correct()

		self.speller.clearSession()
		
		self.all_incorrect()


class TestPersonalwordlist(TestBase):
	
	def setUp(self):
		TestBase.setUp(self)
		self._clear_personal()

	def _read_personal(self):
		path = self.config['personal-path']
		with open(path, 'rt') as f:
			L = f.readlines()
			return [line.rstrip() for line in L[1:]]

	def _clear_personal(self):
		"clear personal dictionary - remove a file"
		path = self.config['personal-path']
		try:
			os.remove(path)
		except OSError:
			pass

	def test_add(self):
		"addtoPersonal"
		
		for word in self.polish_words:
			self.assertFalse(self.speller.check(word))

		for word in self.polish_words:
			self.speller.addtoPersonal(word)

		for word in self.polish_words:
			self.assertTrue(self.speller.check(word))

	def test_get(self):
		"getPersonalwordlist"

		pwl = self.speller.getPersonalwordlist()
		self.assertEqual(set(pwl), set())

		for word in self.polish_words:
			self.speller.addtoPersonal(word)

		pwl = self.speller.getPersonalwordlist()
		self.assertEqual(set(pwl), set(self.polish_words))

	def test_saveall(self):
		"saveAllwords"

		for word in self.polish_words:
			self.speller.addtoPersonal(word)

		pwl = self.speller.getPersonalwordlist()
		self.assertEqual(set(pwl), set(self.polish_words))

		self.speller.saveAllwords()
		saved_wl = self._read_personal()
		self.assertEqual(set(pwl), set(self.polish_words))

		self._clear_personal()


class TestSetConfigKey(unittest.TestCase):
	def setUp(self):
		self.speller = aspell.Speller(('lang', 'en'))


	def get_config(self):
		config = self.speller.ConfigKeys();
		return dict((name, value) for name, (type, value, desc) in config.items())


	def test_string_value(self):
		
		self.speller.setConfigKey('sug-mode', "normal");
		sug1 = self.speller.suggest('rutter')
		self.speller.setConfigKey('sug-mode', "bad-spellers");
		sug2 = self.speller.suggest('rutter')

		self.assertTrue(sug1 != sug2)


	def test_bool_value(self):
		key  = 'clean-affixes'

		self.speller.setConfigKey(key, True)
		self.assertEqual(self.get_config()[key], True)
		self.speller.setConfigKey(key, False)
		self.assertEqual(self.get_config()[key], False)


	def test_int_value(self):
		key   = 'run-together-min'
		value = 123
		self.speller.setConfigKey(key, value)
		self.assertEqual(self.get_config()[key], value)

	@unittest.skip("not implemented")
	def test_list_value(self):
		key   = 'filter'
		value = ['foo', 'bar', 'baz']
		self.speller.setConfigKey(key, value)
		self.assertEqual(self.get_config()[key], value)

if __name__ == '__main__':
	try:
		del sys.argv[sys.argv.index(arg)]
	except:
		pass

	unittest.main()

# vim: ts=4 sw=4 nowrap noexpandtab
