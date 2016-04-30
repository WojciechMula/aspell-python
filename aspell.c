/*****************************************************************************

        Python wrapper for aspell, version 1.12

        Tested with:
        * aspell 0.60.2 & python 3.2

        Released under BSD license

        Wojciech Muła
        wojciech_mula@poczta.onet.pl

 History:
 - 2011-03-31:
   * added __contains__ method

 - 2011-03-xx:
   * ConfigKeys returns dictionary

 - 2011-02-21, 2011-03-06:
	 * compliance with py3k

 - 2006-04-xx:
   * license is BSD now

 -    18.08.2005:
              * fixed method ConfigKeys - now works with aspell 0.60
							  thanks to Gora Mohanty for note
						  * fixed stupid bug in Speller
 -    29.01.2005:
              * added method ConfigKeys()
 -    30.12.2004:
              * new() constructor replaced with Speller()
              * constructor accepts now much simpler syntax for passing
                multiple arguments
              * removed methods releated to configuratinon from AspellSpeller
                object
              * global method ConfigKeys()
 -     7.10.2004:
              * fixed saveAllwords method
                patch by Helmut Jarausch
 -    28.08.2004:
              * tested with python 2.3.4
              * now aspell.new accepts list of config keys
                (see typescript2.txt for examples)
 - 20-22.08.2004:
              * first version of module

$Id$
******************************************************************************/

#include <Python.h>
#include <aspell.h>

#define Speller(pyobject) (((aspell_AspellObject*)pyobject)->speller)
#define Encoding(pyobject) (((aspell_AspellObject*)pyobject)->encoding)

static char* DefaultEncoding = "ascii";

static PyTypeObject aspell_AspellType;

/* error reported by speller */
static PyObject* _AspellSpellerException;

/* error reported by speller's config */
static PyObject* _AspellConfigException;

/* error reported by module */
static PyObject* _AspellModuleException;

typedef struct {
	PyObject_HEAD
	char* encoding; /* internal encoding */
	AspellSpeller* speller;	/* the speller */
} aspell_AspellObject;


/* helper function: converts an aspell word list into python list */
static PyObject* AspellWordList2PythonList(PyObject* self, const AspellWordList* wordlist) {
	PyObject* list;
	PyObject* elem;

	AspellStringEnumeration* elements;
	const char* word;

	list = PyList_New(0);
	if (!list) {
		PyErr_SetString(PyExc_Exception, "can't create new list");
		return NULL;
	}

	elements = aspell_word_list_elements(wordlist);
	while ( (word=aspell_string_enumeration_next(elements)) != 0) {
		elem = PyUnicode_Decode(word, strlen(word), Encoding(self), NULL);

		if (elem == 0) {
			delete_aspell_string_enumeration(elements);
			Py_DECREF(list);
			return NULL;
		}

		if (PyList_Append(list, elem) == -1) {
			delete_aspell_string_enumeration(elements);
			Py_DECREF(elem);
			Py_DECREF(list);
			return NULL;
		}
	}

	delete_aspell_string_enumeration(elements);
	return list;
}

/* helper function: converts an aspell string list into python list */
static PyObject* AspellStringList2PythonList(const AspellStringList* wordlist) {
	PyObject* list;
	AspellStringEnumeration* elements;
	const char* word;

	list = PyList_New(0);
	if (!list) {
		PyErr_SetString(PyExc_Exception, "can't create new list");
		return NULL;
	}

	elements = aspell_string_list_elements(wordlist);
	while ( (word=aspell_string_enumeration_next(elements)) != 0)
		if (PyList_Append(list, Py_BuildValue("s", word)) == -1) {
			PyErr_SetString(PyExc_Exception, "It is almost impossible, but happend! Can't append element to the list.");
			delete_aspell_string_enumeration(elements);
			Py_DECREF(list);
			return NULL;
		}
	delete_aspell_string_enumeration(elements);
	return list;
}


static PyObject* get_single_arg_string(
	PyObject* self,	// [in]
	PyObject* obj,	// [in]
	char** word,		// [out]
	Py_ssize_t* size	// [out]
);

/* Create a new speller *******************************************************/
static PyObject* new_speller(PyTypeObject* self, PyObject* args, PyObject* kwargs) {
	aspell_AspellObject* newobj;

	AspellSpeller* speller = 0;
	AspellConfig*  config;
	AspellCanHaveError* possible_error;

	int i;
	int n; /* arg count */
	char *key, *value;
	char *encoding;

	config = new_aspell_config();
	if (config == NULL) {
		PyErr_SetString(_AspellModuleException, "can't create config");
		return NULL;
	}

	/* check constructor arguments */
	n = PyTuple_Size(args);
	switch (n) {
		case 0: /* no arguments passed */
			break;

		case 2: /* constructor is called with single pair: key & value */
			if (PyArg_ParseTuple(args, "ss", &key, &value)) {
				if (!aspell_config_replace(config, key, value)) {
					PyErr_SetString(_AspellConfigException, aspell_config_error_message(config));
					goto arg_error;
				}
				break;
			}
			PyErr_Clear();
		default: /* list of tuples key&value */
			for (i=0; i<n; i++) {
				if (!PyArg_ParseTuple(PyTuple_GetItem(args, i), "ss", &key, &value)) {
					PyErr_Format(PyExc_TypeError, "argument %d: tuple of two strings (key, value) expeced", i);
					goto arg_error;
				}
				if (!aspell_config_replace(config, key, value)) {
					PyErr_SetString(_AspellConfigException, aspell_config_error_message(config));
					goto arg_error;
				}
			}
			break;
	}

	/* try to create a new speller */
	possible_error = new_aspell_speller(config);

	if (aspell_error_number(possible_error) == 0)
		/* save a speller */
		speller = to_aspell_speller(possible_error);
	else {
		/* or raise an exception */
		PyErr_SetString(_AspellSpellerException, aspell_error_message(possible_error));
		delete_aspell_config(config);
		delete_aspell_can_have_error(possible_error);
		return NULL;
	}

	/* get encoding */
	encoding = NULL;
	value = (char*)aspell_config_retrieve(config, "encoding");
	if (value) {
		if (strcmp(value, "none") != 0) {
			encoding = (char*)malloc(strlen(value)+1);
			if (encoding)
				strcpy(encoding, value);
		}
	}

	if (encoding == NULL)
		encoding = DefaultEncoding;
	
	// free config
	delete_aspell_config(config);

	/* create a new py-object */
  newobj = (aspell_AspellObject*)PyObject_New(aspell_AspellObject, &aspell_AspellType);
	newobj->speller = speller;
	newobj->encoding = encoding;

	return (PyObject*)newobj;

/* argument error: before return NULL we need to
   delete speller's config we've created */
arg_error:
	delete_aspell_config(config);
	return NULL;
}

/* Delete speller *************************************************************/
static void speller_dealloc(PyObject* self) {
	if (Encoding(self) != DefaultEncoding)
		free(Encoding(self));

	delete_aspell_speller( Speller(self) );
	PyObject_Del(self);
}


static PyObject* configkeys_helper(PyObject* self) {
	AspellConfig* config;
	AspellKeyInfoEnumeration *keys_enumeration;
	AspellStringList* lst;
	AspellMutableContainer* amc;
	const AspellKeyInfo *key_info;

	PyObject *dict = 0, *obj = 0, *value = 0;
	const char*  string;
	unsigned int integer;
	unsigned int boolean;

	char *key_type = 0;

	if (self)
		config = aspell_speller_config(Speller(self));
	else
		config = new_aspell_config();

	if (config == NULL) {
		PyErr_SetString(_AspellModuleException, "can't create config");
		return NULL;
	}

	keys_enumeration = aspell_config_possible_elements(config, 1);
	if (!keys_enumeration) {
		if (!self) delete_aspell_config(config);
		PyErr_SetString(_AspellConfigException, "can't get list of config keys");
		return NULL;
	}

	dict = PyDict_New();
	if (dict == NULL) {
		if (!self) delete_aspell_config(config);
		return NULL;
	}
		
	while ((key_info = aspell_key_info_enumeration_next(keys_enumeration))) {

		/* key type -> string */
		switch (key_info->type) {
			case AspellKeyInfoString:
				key_type = "string";
				string   = aspell_config_retrieve(config, key_info->name);
				if (aspell_config_error(config) != NULL) goto config_get_error;
				obj      = PyUnicode_FromString( string );
				break;
			case AspellKeyInfoInt:
				key_type = "integer";
				integer  = aspell_config_retrieve_int(config, key_info->name);
				if (aspell_config_error(config) != NULL) goto config_get_error;
				obj      = PyLong_FromLong( integer );
				break;
			case AspellKeyInfoBool:
				key_type = "boolean";
				boolean  = aspell_config_retrieve_bool(config, key_info->name);
				if (aspell_config_error(config) != NULL) goto config_get_error;
				obj      = PyBool_FromLong( boolean );
				break;
			case AspellKeyInfoList:
				key_type = "list";
				lst = new_aspell_string_list();
				amc = aspell_string_list_to_mutable_container(lst);
				aspell_config_retrieve_list(config, key_info->name, amc);
				if (aspell_config_error(config) != NULL) goto config_get_error;

				obj = AspellStringList2PythonList(lst);
				delete_aspell_string_list(lst);
				break;
		}

		/* value */
		value = Py_BuildValue("(sOs)",
							key_type,
							obj,
							key_info->desc ? key_info->desc : "internal"
		);

		if (value) {
			if (PyDict_SetItemString(dict, key_info->name, value)) {
				goto python_error;
			}
			else
				Py_DECREF(value);
		}
		else
			goto python_error;
	}
	
	delete_aspell_key_info_enumeration(keys_enumeration);
	if (!self) delete_aspell_config(config);
	return dict;

config_get_error:
	PyErr_SetString(_AspellConfigException, aspell_config_error_message(config));
python_error:
	delete_aspell_key_info_enumeration(keys_enumeration);
	if (!self) delete_aspell_config(config);
	Py_DECREF(dict);
	return NULL;
}

/* ConfigKeys *****************************************************************/
static PyObject* configkeys(PyObject* _) {
	return configkeys_helper(NULL);
}

/* method:ConfigKeys **********************************************************/
static PyObject* m_configkeys(PyObject* self, PyObject* args) {
	return configkeys_helper(self);
}

/* method:setConfigKey ********************************************************/
static PyObject* m_set_config_key(PyObject* self, PyObject* args) {
	AspellConfig* config;
	const AspellKeyInfo* info;
	char* key;
	char* string;
	Py_ssize_t length;
	long  number;
	char  buffer[32];

	PyObject* arg1;
	PyObject* value;

	if (PyTuple_Size(args) != 2) {
		PyErr_Format(PyExc_TypeError, "expected two arguments");
		return NULL;
	}

	if (!PyArg_ParseTuple(args, "sO", &key, &arg1)) {
		PyErr_Format(PyExc_TypeError, "first argument must be a string");
		return NULL;
	}


	config = aspell_speller_config(Speller(self));
	info   = aspell_config_keyinfo(config, key);
	if (aspell_config_error(config) != 0) {
		PyErr_SetString(_AspellConfigException, aspell_config_error_message(config));
		return NULL;
	}

	switch (info->type) {
		case AspellKeyInfoList:
			// Can't figure out the splitting char for lists,
			// it seems to be the ':', but doesn't work.
		case AspellKeyInfoString:
			value = get_single_arg_string(self, arg1, &string, &length);
			if (value == NULL) {
				PyErr_Format(PyExc_TypeError, "second argument have to be string");
				return NULL;
			}

			aspell_config_replace(config, key, string);
			Py_DECREF(value);
			break;

		case AspellKeyInfoInt:
			number = PyLong_AsLong(arg1);
			if (number == -1 && PyErr_Occurred()) {
				return NULL;
			}

			snprintf(buffer, 32, "%ld", number);
			aspell_config_replace(config, key, buffer);
			break;

		case AspellKeyInfoBool:
			if (PyBool_Check(arg1)) {
				aspell_config_replace(config, key, (arg1 == Py_True) ? "true" : "false");
			} else {
				PyErr_Format(PyExc_TypeError, "second argument have to be boolean");
				return NULL;
			}
			break;

		default:
			PyErr_Format(_AspellModuleException, "unsupported aspell config item type");
			return NULL;
	}

	if (aspell_config_error(config) != 0) {
		PyErr_SetString(_AspellConfigException, aspell_config_error_message(config));
		return NULL;
	}

	Py_RETURN_NONE;
}


static PyObject* get_single_arg_string(
	PyObject* self,	// [in]
	PyObject* obj,	// [in]
	char** word,		// [out]
	Py_ssize_t* size	// [out]
) {
	PyObject* buf;

	/* unicode */
	if (PyUnicode_Check(obj))
		/* convert to buffer */
		buf = PyUnicode_AsEncodedString(obj, Encoding(self), "strict");
	else
	/* buffer */
	if (PyBytes_Check(obj)) {
		buf = obj;
		Py_INCREF(buf);	// PyTuple_GetItem returns borrowed reference
	}
	else {
		PyErr_SetString(PyExc_TypeError, "string of bytes required");
		return NULL;
	}

	/* unpack buffer */
	if (buf) {
		if (PyBytes_AsStringAndSize(buf, word, size) != -1) {
			return buf;
		}
		else {
			Py_DECREF(buf);
			return NULL;
		}
	}
	else
		return NULL;
}


static PyObject* get_arg_string(
	PyObject* self,	// [in]
	PyObject* args,	// [in]
	const int index,// [in] args[index]
	char** word,		// [out]
	Py_ssize_t* size	// [out]
) {
	PyObject* obj;

	obj = PyTuple_GetItem(args, index);
	if (obj)
		return get_single_arg_string(self, obj, word, size);
	else
		return NULL;
}


/* method:__contains__ ********************************************************/
static int
m_contains(PyObject* self, PyObject* args) {
	char*	word;
	Py_ssize_t length;
	PyObject* buf;

	buf = get_single_arg_string(self, args, &word, &length);
	if (buf != NULL)
		switch (aspell_speller_check(Speller(self), word, length)) {
			case 0:
				Py_DECREF(buf);
				return 0;

			case 1:
				Py_DECREF(buf);
				return 1;

			default:
				Py_DECREF(buf);
				PyErr_SetString(_AspellSpellerException, aspell_speller_error_message(Speller(self)));
				return -1;
		} // switch
	else
		return -1;
}


/* method:check ***************************************************************/
static PyObject* m_check(PyObject* self, PyObject* args) {
	PyObject* word;

	word = PyTuple_GetItem(args, 0);
	if (word)
		switch (m_contains(self, word)) {
			case 0:
				Py_RETURN_FALSE;
			
			case 1:
				Py_RETURN_TRUE;

			default:
				return NULL;
		}
	else
		return NULL;
}

/* method:suggest ************************************************************/
static PyObject* m_suggest(PyObject* self, PyObject* args) {
	char* word;
	Py_ssize_t length;
	PyObject* buf;
	PyObject* list;

	buf = get_arg_string(self, args, 0, &word, &length);
	if (buf) {
		list = AspellWordList2PythonList(
			self,
			aspell_speller_suggest(Speller(self),
			word,
			length)
		);
		Py_DECREF(buf);
		return list;
	}
	else
		return NULL;
}

/* method:getMainwordlist *****************************************************/
static PyObject* m_getMainwordlist(PyObject* self, PyObject* args) {
	return AspellWordList2PythonList(self, aspell_speller_main_word_list(Speller(self)));
}

/* method:getPersonalwordlist *************************************************/
static PyObject* m_getPersonalwordlist(PyObject* self, PyObject* args) {
	return AspellWordList2PythonList(self, aspell_speller_personal_word_list(Speller(self)));
}

/* method:getSessionwordlist **************************************************/
static PyObject* m_getSessionwordlist(PyObject* self, PyObject* args) {
	return AspellWordList2PythonList(self, aspell_speller_session_word_list(Speller(self)));
}

/* check for any aspell error after a lib call
   and either raises exception one or returns none  */
static PyObject* AspellCheckError(PyObject* self) {
	if (aspell_speller_error(Speller(self)) != 0) {
		PyErr_SetString(_AspellSpellerException, aspell_speller_error_message(Speller(self)));
		return NULL;
	}
	else
		Py_RETURN_NONE;
}

/* method:addtoPersonal *******************************************************/
static PyObject* m_addtoPersonal(PyObject* self, PyObject* args) {
	char *word;
	Py_ssize_t length;

	if (!PyArg_ParseTuple(args, "s#", &word, &length)) {
		PyErr_SetString(PyExc_TypeError, "a string is required");
		return NULL;
	}

	aspell_speller_add_to_personal(Speller(self), word, length);
	return AspellCheckError(self);
}

/* method:addtoSession ********************************************************/
static PyObject* m_addtoSession(PyObject* self, PyObject* args) {
	char *word;
	Py_ssize_t length;
	PyObject* buf;

	buf = get_arg_string(self, args, 0, &word, &length);
	if (buf) {
		aspell_speller_add_to_session(Speller(self), word, length);
		Py_DECREF(buf);
		return AspellCheckError(self);
	}
	else
		return NULL;

}

/* method:clearsession ********************************************************/
static PyObject* m_clearsession(PyObject* self, PyObject* args) {
	aspell_speller_clear_session(Speller(self));
	return AspellCheckError(self);
}

/* method:saveallwords ********************************************************/
static PyObject* m_saveallwords(PyObject* self, PyObject* args) {
	aspell_speller_save_all_word_lists(Speller(self));
	return AspellCheckError(self);
}

/* method:addReplacement ******************************************************/
static PyObject* m_addReplacement(PyObject* self, PyObject* args) {
	char *mis; Py_ssize_t ml;
	char *cor; Py_ssize_t cl;
	PyObject* Mbuf;
	PyObject* Cbuf;

	Mbuf = get_arg_string(self, args, 0, &mis, &ml);
	if (Mbuf == NULL) {
		PyErr_SetString(PyExc_TypeError, "first argument have to be a string or bytes");
		return NULL;
	}

	Cbuf = get_arg_string(self, args, 1, &cor, &cl);
	if (Cbuf == NULL) {
		Py_DECREF(Mbuf);
		PyErr_SetString(PyExc_TypeError, "second argument have to be a string or bytes");
		return NULL;
	}

	aspell_speller_store_replacement(Speller(self), mis, ml, cor, cl);
	Py_DECREF(Mbuf);
	Py_DECREF(Cbuf);
	return AspellCheckError(self);
}

/* AspellSpeller methods table */
static PyMethodDef aspell_object_methods[] = {
	{
		"ConfigKeys",
		(PyCFunction)m_configkeys,
		METH_VARARGS,
		"ConfigKeys() => dictionary of config keys\n"
        "Keys are string, values are 3-touple:\n"
		"\t1. key type={string|integer|boolean|list}\n"
		"\t2. current value\n"
		"\t3. description (if 'internal' no description available)"
	},
	{
		"setConfigKey",
		(PyCFunction)m_set_config_key,
		METH_VARARGS,
		"changeConfig(key, value)\n"
		"Sets a new config value for given key"
	},
	{
		"check",
		(PyCFunction)m_check,
		METH_VARARGS,
		"check(word) => bool\n"
 		"Checks spelling of word.\n"
		"Returns if word is correct."
	},
	{
		"suggest",
		(PyCFunction)m_suggest,
		METH_VARARGS,
		"suggest(word) => list of words\n"
 		"Returns a list of suggested spelling for given word.\n"
		"Even if word is correct (i.e. check(word) returned 1) aspell performs action."
	},
	{
		"getMainwordlist",
		(PyCFunction)m_getMainwordlist,
		METH_VARARGS,
		"getMainwordlist() => list of words\n"
		"Return a list of words stored in the main dictionary."

	},
	{
		"getPersonalwordlist",
		(PyCFunction)m_getPersonalwordlist,
		METH_VARARGS,
		"getPersonalwordlist() => list of words\n"
		"Return a list of words stored in the personal dictionary."
	},
	{
		"getSessionwordlist",
		(PyCFunction)m_getSessionwordlist,
		METH_VARARGS,
		"getSessionwordlist() => list of words\n"
		"Return a list of words stored in the session dictionary."
	},
	{
		"clearSession",
		(PyCFunction)m_clearsession,
		METH_VARARGS,
		"clearSession() => None\n"
		"Clear current session, i.e. erases all words added thru addtoSession method since last saveallwords() call."
	},
	{
		"saveAllwords",
		(PyCFunction)m_saveallwords,
		METH_VARARGS,
		"saveAllwords() => None\n"
 		"Save all words added thru addtoPersonal() and addtoSession() methods."
	},
	{
		"addtoPersonal",
		(PyCFunction)m_addtoPersonal,
		METH_VARARGS,
		"addtoPersonal(word) => None\n"
		"Add word to the personal dictionary"
	},
	{
		"addtoSession",
		(PyCFunction)m_addtoSession,
		METH_VARARGS,
		"addtoSession(word) => None\n"
		"Add word to the session dictionary"
	},
	{
		"addReplacement",
		(PyCFunction)m_addReplacement,
		METH_VARARGS,
		"addReplacement(misspeled word, correct word) => None\n"
		"Add a replacement pair, i.e. a misspeled and correct words.\n"
		"For example 'teh' and 'the'."
	},
	{NULL, NULL, 0, NULL}
};

static PyTypeObject aspell_AspellType = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	"AspellSpeller",												/* tp_name */
	sizeof(aspell_AspellObject),						/* tp_size */
	0,																			/* tp_itemsize? */
	(destructor)speller_dealloc,            /* tp_dealloc */
	0,                                      /* tp_print */
	0,                                         /* tp_getattr */
	0,                                          /* tp_setattr */
	0,                                          /* tp_reserved */
	0,														              /* tp_repr */
	0,                                          /* tp_as_number */
	0,                                          /* tp_as_sequence */
	0,                                          /* tp_as_mapping */
	0,                                          /* tp_hash */
	0,                                          /* tp_call */
	0,                                          /* tp_str */
	PyObject_GenericGetAttr,                    /* tp_getattro */
	0,                                          /* tp_setattro */
	0,                                          /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,                         /* tp_flags */
	0,                                          /* tp_doc */
	0,                                          /* tp_traverse */
	0,                                          /* tp_clear */
	0,                                          /* tp_richcompare */
	0,                                          /* tp_weaklistoffset */
	0,                                          /* tp_iter */
	0,                                          /* tp_iternext */
	aspell_object_methods,                      /* tp_methods */
	0,							                            /* tp_members */
	0,                                          /* tp_getset */
	0,                                          /* tp_base */
	0,                                          /* tp_dict */
	0,                                          /* tp_descr_get */
	0,                                          /* tp_descr_set */
	0,                                          /* tp_dictoffset */
	0,                                          /* tp_init */
	0,                                          /* tp_alloc */
	new_speller,																/* tp_new */
};


static PySequenceMethods speller_as_sequence;

static PyMethodDef aspell_module_methods[] = {
	{
		"ConfigKeys",
		(PyCFunction)configkeys,
		METH_VARARGS,
		"ConfigKeys() => dictionary of config keys\n"
        "Keys are string, values are 3-touple:\n"
		"\t1. key type={string|integer|boolean|list}\n"
		"\t2. current value\n"
		"\t3. description (if 'internal' no description available)"
	},
	{NULL, NULL, 0, NULL}
};

static PyModuleDef aspellmodule = {
	PyModuleDef_HEAD_INIT,
	"aspell",
	"aspell wrapper",
	-1,
	aspell_module_methods,
};

PyMODINIT_FUNC
PyInit_aspell(void) {
	PyObject *module;

	module = PyModule_Create(&aspellmodule);
	if (module == NULL)
		return NULL;

	speller_as_sequence.sq_contains = m_contains;
	aspell_AspellType.tp_as_sequence = &speller_as_sequence;

	if (PyType_Ready(&aspell_AspellType) < 0) {
		Py_DECREF(module);
		return NULL;
	}
	else
		PyModule_AddObject(module, "Speller", (PyObject*)&aspell_AspellType);

	_AspellSpellerException = PyErr_NewException("aspell.AspellSpellerError", NULL, NULL);
	_AspellModuleException  = PyErr_NewException("aspell.AspellModuleError", NULL, NULL);
	_AspellConfigException  = PyErr_NewException("aspell.AspellConfigError", NULL, NULL);

	PyModule_AddObject(module, "AspellSpellerError", _AspellSpellerException);
	PyModule_AddObject(module, "AspellModuleError", _AspellModuleException);
	PyModule_AddObject(module, "AspellConfigError", _AspellConfigException);

	return module;
}

/*
vim:ts=2 sw=2
*/
