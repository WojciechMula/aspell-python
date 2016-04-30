/*****************************************************************************

        Python wrapper for aspell, version 1.0

        Tested with:
        * aspell 0.50.5 & python 2.1
        * aspell 0.50.5 & python 2.3.4, 3.4.1
        * aspell 0.60.2 & python 2.3.4

        Released under BSD license

        Wojciech Mu³a
        wojciech_mula@poczta.onet.pl

 History:
 # 20-22.08.2004:
              * first version of module
 #    28.08.2004:
              * tested with python 2.3.4
              * now aspell.new accepts list of config keys
                (see typescript2.txt for examples)
 #     7.10.2004:
              * fixed saveAllwords method
                patch by Helmut Jarausch

 #    30.12.2004:
              * new() constructor replaced with Speller()
              * constructor accepts now much simpler syntax for passing
                multiple arguments
              * removed methods releated to configuratinon from AspellSpeller
                object
              * global method ConfigKeys()
 #    29.01.2005:
              * added method ConfigKeys()
 #    18.08.2005:
              * fixed method ConfigKeys - now works with aspell 0.60
							  thanks to Gora Mohanty for note
						  * fixed stupid bug in Speller
 #       04.2006:
              * license is BSD now


$Id: aspell.c,v 1.2 2006-09-27 16:45:16 wojtek Exp $
******************************************************************************/

#include <Python.h>
#include <aspell.h>

/* helper function: converts an aspell word list into python list */
static PyObject* AspellWordList2PythonList(const AspellWordList* wordlist) {
	PyObject* list;
	AspellStringEnumeration* elements;
	const char* word;

	list = PyList_New(0);
	if (!list) {
		PyErr_SetString(PyExc_Exception, "can't create new list");
		return NULL;
	}

	elements = aspell_word_list_elements(wordlist);
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


staticforward PyTypeObject aspell_AspellType;
/* error reported by speller */
static PyObject* _AspellSpellerException;

/* error reported by speller's config */
static PyObject* _AspellConfigException;

/* error reported by module */
static PyObject* _AspellModuleException;

typedef struct {
	PyObject_HEAD
	AspellSpeller* speller;	/* the speller */
} aspell_AspellObject;

#define Speller(pyobject) (((aspell_AspellObject*)pyobject)->speller)

#define return_None   \
	Py_INCREF(Py_None); \
	return Py_None;

/* Create a new speller *******************************************************/
static PyObject* new_speller(PyObject* self, PyObject* args) {
	aspell_AspellObject* newobj;

	AspellSpeller* speller = 0;
	AspellConfig*  config;
	AspellCanHaveError* possible_error;

	int i;
	int n; /* arg count */
	char *key, *value;

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
			Py_DECREF(args);
			break;
	}

	/* try to create a new speller */
	possible_error = new_aspell_speller(config);
	delete_aspell_config(config);

	if (aspell_error_number(possible_error) == 0)
		/* save a speller */
		speller = to_aspell_speller(possible_error);
	else {
		/* or raise an exception */
		PyErr_SetString(_AspellSpellerException, aspell_error_message(possible_error));
		delete_aspell_can_have_error(possible_error);
		return NULL;
	}

	/* create a new py-object */
  newobj = (aspell_AspellObject*)PyObject_New(aspell_AspellObject, &aspell_AspellType);
	newobj->speller = speller;

	return (PyObject*)newobj;

/* argument error: before return NULL we need to
   delete speller's config we've created */
arg_error:
	delete_aspell_config(config);
	return NULL;
}

/* Delete speller *************************************************************/
static void speller_dealloc(PyObject* self) {
	delete_aspell_speller( Speller(self) );
	PyObject_Del(self);
}

/* ConfigKeys *****************************************************************/
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
	long  number;
	char  buffer[32];

	PyObject* arg1;

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
			string = PyString_AsString(arg1);
			if (string == NULL) {
				PyErr_Format(PyExc_TypeError, "second argument have to be string");
				return NULL;
			}

			aspell_config_replace(config, key, string);
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

/* method:check ***************************************************************/
static int m_contains(PyObject* self, PyObject* args) {
	char* word;
	Py_ssize_t length;

	if (PyTuple_Check(args)) {
		if (!PyArg_ParseTuple(args, "s#", &word, &length)) {
			PyErr_SetString(PyExc_TypeError, "a string is required");
			return -1;
		}
	} else {
		if (PyString_AsStringAndSize(args, &word, &length)) {
			return -1;
		}
	}

	if (!length) {
		return 1;
	}

	switch (aspell_speller_check(Speller(self), word, length)) {
		case 0:
			return 0;
		case 1:
			return 1;
		default:
			PyErr_SetString(_AspellSpellerException, aspell_speller_error_message(Speller(self)));
			return -1;
	}
}

/* method:check ***************************************************************/
static PyObject* m_check(PyObject* self, PyObject* args) {

	const int ret = m_contains(self, args);
	if (ret < 0) {
		return NULL;
	}

	return Py_BuildValue("i", ret);
}

/* method:suggest ************************************************************/
static PyObject* m_suggest(PyObject* self, PyObject* args) {
	char* word;
	int   length;

	if (!PyArg_ParseTuple(args, "s#", &word, &length)) {
		PyErr_SetString(PyExc_TypeError, "string expeced");
		return NULL;
	}

	return AspellWordList2PythonList( aspell_speller_suggest(Speller(self), word, length));
}

/* method:getMainwordlist *****************************************************/
static PyObject* m_getMainwordlist(PyObject* self, PyObject* args) {
	return AspellWordList2PythonList( aspell_speller_main_word_list(Speller(self)));
}

/* method:getPersonalwordlist *************************************************/
static PyObject* m_getPersonalwordlist(PyObject* self, PyObject* args) {
	return AspellWordList2PythonList( aspell_speller_personal_word_list(Speller(self)));
}

/* method:getSessionwordlist **************************************************/
static PyObject* m_getSessionwordlist(PyObject* self, PyObject* args) {
	return AspellWordList2PythonList( aspell_speller_session_word_list(Speller(self)));
}

/* method:addtoPersonal *******************************************************/
static PyObject* m_addtoPersonal(PyObject* self, PyObject* args) {
	char *word;
	int   length;

	if (!PyArg_ParseTuple(args, "s#", &word, &length)) {
		PyErr_SetString(PyExc_TypeError, "a string is required");
		return NULL;
	}

	aspell_speller_add_to_personal(Speller(self), word, length);
	if (aspell_speller_error(Speller(self)) != 0) {
		PyErr_SetString(_AspellSpellerException, aspell_speller_error_message(Speller(self)));
		return NULL;
	}
	return_None;
}

/* method:addtoSession ********************************************************/
static PyObject* m_addtoSession(PyObject* self, PyObject* args) {
	char *word;
	int   length;

	if (!PyArg_ParseTuple(args, "s#", &word, &length)) {
		PyErr_SetString(PyExc_TypeError, "Invalid argument");
		return NULL;
	}

	aspell_speller_add_to_session(Speller(self), word, length);
	if (aspell_speller_error(Speller(self)) != 0) {
		PyErr_SetString(_AspellSpellerException, aspell_speller_error_message(Speller(self)));
		return NULL;
	}
	return_None;
}

/* method:clearsession ********************************************************/
static PyObject* m_clearsession(PyObject* self, PyObject* args) {
	aspell_speller_clear_session(Speller(self));
	if (aspell_speller_error(Speller(self)) != 0) {
		PyErr_SetString(_AspellSpellerException, aspell_speller_error_message(Speller(self)));
		return NULL;
	}

	return_None;
}

/* method:saveallwords ********************************************************/
static PyObject* m_saveallwords(PyObject* self, PyObject* args) {
	aspell_speller_save_all_word_lists(Speller(self));
	if (aspell_speller_error(Speller(self)) != 0) {
		PyErr_SetString(_AspellSpellerException, aspell_speller_error_message(Speller(self)));
		return NULL;
	}
	return_None;
}
/* method:addReplacement ******************************************************/
static PyObject* m_addReplacement(PyObject* self, PyObject* args) {
	char *mis; int ml;
	char *cor; int cl;

	if (!PyArg_ParseTuple(args, "s#s#", &mis, &ml, &cor, &cl)) {
		PyErr_SetString(PyExc_TypeError, "two strings are required (misspelled, correct word)");
		return NULL;
	}
	aspell_speller_store_replacement(Speller(self), mis, ml, cor, cl);
	if (aspell_speller_error(Speller(self)) != 0) {
		PyErr_SetString(_AspellSpellerException, aspell_speller_error_message(Speller(self)));
		return NULL;
	}
	return_None;
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
		"check(word) => 0 or 1\n"
 		"Checks spelling of word.\n"
		"Returns 1 if word is correct, 0 otherwise."
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

static PyObject* speller_getattr(PyObject *obj, char *name) {
	return Py_FindMethod(aspell_object_methods, obj, name);
}


static PySequenceMethods sequence_methods = {
	0,                          /* sq_length */
	0,                          /* sq_concat */
	0,                          /* sq_repeat */
	0,                          /* sq_item */
	0,                          /* sq_slice */
	0,                          /* sq_ass_item */
	0,                          /* sq_ass_slice */
	m_contains,                 /* sq_contains */
	0,                          /* sq_inplace_concat */
	0,                          /* sq_inplace_repeat */
};


static PyMethodDef aspell_methods[] = {
	{
		"Speller",
		new_speller,
		METH_VARARGS,
		"Create a new AspellSpeller object"
	},
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



static PyTypeObject aspell_AspellType = {
    PyObject_HEAD_INIT(&PyType_Type)
    0,                                          /* ob_size */
    "AspellSpeller",                            /* tp_name */
    sizeof(aspell_AspellObject),                /* tp_basicsize */
    0,                                          /* tp_itemsize */
    speller_dealloc,                            /* tp_dealloc */
    0,                                          /* tp_print */
    speller_getattr,                            /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_compare */
    0,                                          /* tp_repr */
    0,                                          /* tp_as_number */
    &sequence_methods,                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    0,                                          /* tp_call */
    0,                                          /* tp_str */
    0,                                          /* tp_getattro */
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
    aspell_methods,                             /* tp_methods */
    0,                                          /* tp_members */
    0,                                          /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    0,                                          /* tp_init */
    0,                                          /* tp_alloc */
    0,                                          /* tp_new */
    0,                                          /* tp_free */
};

DL_EXPORT(void)
initaspell(void) {
	PyObject *module;
	PyObject *dict;

	aspell_AspellType.ob_type = &PyType_Type;
	module = Py_InitModule("aspell", aspell_methods);
	dict   = PyModule_GetDict(module);

	_AspellSpellerException = PyErr_NewException("aspell.AspellSpellerError", NULL, NULL);
	_AspellModuleException  = PyErr_NewException("aspell.AspellModuleError", NULL, NULL);
	_AspellConfigException  = PyErr_NewException("aspell.AspellConfigError", NULL, NULL);

	PyDict_SetItemString(dict, "AspellSpellerError", _AspellSpellerException);
	PyDict_SetItemString(dict, "AspellModuleError", _AspellModuleException);
	PyDict_SetItemString(dict, "AspellConfigError", _AspellConfigException);
}

/*
vim:ts=2 sw=2
*/
