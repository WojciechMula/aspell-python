try:
	import ctypes
	import ctypes.util
except ImportError:
	raise ImportError("ctypes library is needed")


class AspellError(Exception): pass
class AspellConfigError(AspellError): pass
class AspellSpellerError(AspellError): pass


class AspellLinux(object):
	def __init__(self, configkeys=None, libname=None):
		if libname is None:
			libname = ctypes.util.find_library('aspell')
		self.__lib = ctypes.CDLL(libname)

		# Initialize speller

		# 1. create configuration
		config = self.__lib.new_aspell_config()
		if config == None:
			raise AspellError("Can't create aspell config object")

		# 2. parse configkeys arg.
		if configkeys is not None:
			assert type(configkeys) in [tuple, list], "Tuple or list expeced"
			if len(configkeys) == 2 and \
			   type(configkeys[0]) is str and \
			   type(configkeys[1]) is str:
				configkeys = [configkeys]

			for key, value in configkeys:
				assert type(key) is str, "Key must be string"
				assert type(value) is str, "Value must be string"
				if not self.__lib.aspell_config_replace(config, key, value):
					raise self._aspell_config_error(config)

		# 3. create speller
		possible_error = self.__lib.new_aspell_speller(config)
		self.__lib.delete_aspell_config(config)

		if self.__lib.aspell_error_number(possible_error) != 0:
			self.__lib.delete_aspell_can_have_error(possible_error)
			raise AspellError("Can't create speller object")

		self.__speller = self.__lib.to_aspell_speller(possible_error)


	def check(self, word):
		if type(word) is str:
			return bool(
				self.__lib.aspell_speller_check(
					self.__speller,
					word,
					len(word)
				))
		else:
			raise TypeError("String expeced")


	def suggest(self, word):
		if type(word) is str:
			return self._aspellwordlist(
				self.__lib.aspell_speller_suggest(
					self.__speller,
					word,
					len(word)
				))
		else:
			raise TypeError("String expeced")


	def personal_dict(self, word=None):
		if word is not None:
			# add new word
			assert type(word) is str, "String expeced"
			self.__lib.aspell_speller_add_to_personal(
				self.__speller,
				word,
				len(word)
			)
			self._aspell_check_error()
		else:
			# return list of words from personal dictionary
			return self._aspellwordlist(
				self.__lib.aspell_speller_personal_word_list(self.__speller)
			)
	

	def session_dict(self, word=None, clear=False):
		if clear:
			self.__lib.aspell_speller_clear_session(self.__speller)
			self._aspell_check_error()


		if word is not None:
			# add new word
			assert type(word) is str, "String expeced"
			self.__lib.aspell_speller_add_to_session(
				self.__speller,
				word,
				len(word)
			)
			self._aspell_check_error()
		else:
			# return list of words from personal dictionary
			return self._aspellwordlist(
				self.__lib.aspell_speller_session_word_list(self.__speller)
			)
	

	def add_replacement_pair(self, misspelled, correct):
		assert type(misspelled) is str, "String is required"
		assert type(correct) is str, "String is required"

		self.__lib.aspell_speller_store_replacement(
			self.__speller,
			misspelled,
			len(misspelled),
			correct,
			len(correct)
		)
		self._aspell_check_error()
	

	def save_all(self):
		self.__lib.spell_speller_save_all_word_lists(self.__speller)
		self._aspell_check_error()
	

	def configkeys(self):
		
		config = self.__lib.aspell_speller_config(self.__speller)
		if config is None:
			raise AspellConfigError("Can't get speller's config")

		keys_enum = self.__lib.aspell_config_possible_elements(config, 1)
		if keys_enum is None:
			raise AspellError("Can't get list of config keys")

		class KeyInfo(ctypes.Structure):
			_fields_ = [
				("name",	ctypes.c_char_p),
				("type",	ctypes.c_int),
				("default",	ctypes.c_char_p),
				("desc",	ctypes.c_char_p),
				("flags",	ctypes.c_int),
				("other_data", ctypes.c_int),
			]

		key_next = self.__lib.aspell_key_info_enumeration_next
		key_next.restype = ctypes.POINTER(KeyInfo)

		list = []
		while True:
			key_info = key_next(keys_enum)
			if not key_info:
				break
			else:
				key_info = key_info.contents

			if key_info.type == 10:
				# string
				list.append((
					key_info.name,
					key_info.default,
					key_info.desc,
				))

			elif key_info.type == 11:
				# integer
				list.append((
					key_info.name,
					int(key_info.default),
					key_info.desc,
				))
			elif key_info.type == 12:
				# boolean
				if key_info.default.lower() == 'true':
					list.append((
						key_info.name,
						True,
						key_info.desc,
					))
				else:
					list.append((
						key_info.name,
						False,
						key_info.desc,
					))
			elif key_info.type == 3:
				# list
				list.append((
					key_info.name,
					key_info.default.split(),
					key_info.desc,
					))

		self.__lib.delete_aspell_key_info_enumeration(keys_enum)
		return list


	def close(self):
		self.__lib.delete_aspell_speller(self.__speller)
	

	def _aspellwordlist(self, wordlist_id):
		elements = self.__lib.aspell_word_list_elements(wordlist_id)
		list = []
		while True:
			wordptr = self.__lib.aspell_string_enumeration_next(elements)
			if not wordptr:
				break
			else:
				word = ctypes.c_char_p(wordptr)
				list.append(word.value)

		self.__lib.delete_aspell_string_enumeration(elements)
		return list
	

	def _aspell_config_error(self, config):
		# make exception object & copy error msg 
		exc = AspellConfigError(
			ctypes.c_char_p(
				self.__lib.aspell_config_error_message(config)
			).value
		)
	
		# then destroy config objcet
		self.__lib.delete_aspell_config(config)

		# and then raise exception
		raise exc


	def _aspell_check_error(self):
		if self.__lib.aspell_speller_error(self.__speller) != 0:
			msg = self.__lib.aspell_speller_error_message(self.__speller)
			raise AspellSpellerError(msg)
		
	
if __name__ == '__main__':
	a = AspellLinux(("lang", "en"))
	a.close()

# vim: ts=4 sw=4
