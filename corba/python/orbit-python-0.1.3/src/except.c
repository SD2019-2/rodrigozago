#include "CORBAmodule.h"

PyObject *ErrorObject, *SystemExcept, *UserExcept;
PyObject *except_base_class;
GHashTable *exceptions;


PyObject *UserExcept_PyClass__init(PyObject *_, PyObject *args, PyObject *keys)
{
	PyObject *self = PyTuple_GetItem(args, 0);
	if (self && keys) {
		int pos = 0;
		PyObject *key, *value;
		while (PyDict_Next(keys, &pos, &key, &value))
			PyObject_SetAttr(self, key, value);
	}
	Py_INCREF(Py_None);
	return Py_None;
}

void init_exceptions()
{
   ErrorObject = PyErr_NewException("CORBA.error", NULL, NULL);
   PyDict_SetItemString(ModuleDict, "error", ErrorObject);
   // Setup the system exception
   SystemExcept = PyErr_NewException("CORBA.SystemException", NULL, NULL);
   PyDict_SetItemString(ModuleDict, "SystemException", SystemExcept);
   // Setup the user exception
   UserExcept = PyErr_NewException("CORBA.UserException", NULL, NULL);

   PyDict_SetItemString(ModuleDict, "UserException", UserExcept);
}


void raise_user_exception(char *repo_id, PyObject *o)
{	
	PyObject *error;
	if (!o) {
		Py_INCREF(Py_None);
		o = Py_None;
	}
	error = (PyObject *)g_hash_table_lookup(exceptions, repo_id);
	if (error) {
		PyErr_SetObject(error, o);
	}
	else {
		raise_system_exception("IDL:omg.org/CORBA/UNKNOWN:1.0", 0,
		                       CORBA_COMPLETED_MAYBE, NULL);
	}
}

void raise_system_exception(char *repo_id, CORBA_unsigned_long minor,
                            CORBA_completion_status status, char *info, ...)
{
//	PyErr_SetString(SystemExcept, repo_id);
	char buffer[500];
	if (!info)
		info = repo_id;
	else {
		va_list args;
		va_start(args, info);
		vsprintf(buffer, info, args);
		info = buffer;
	}
	PyErr_Format(SystemExcept, "%s: %s", repo_id, info);
}
	
CORBA_boolean check_corba_ex(CORBA_Environment *ev)
{
	if (ev->_major != CORBA_NO_EXCEPTION) {
		g_message("CORBA exception raised: %s", CORBA_exception_id(ev));
		raise_system_exception(CORBA_exception_id(ev), 0, CORBA_COMPLETED_MAYBE,
		                       NULL);
		return CORBA_FALSE;
	}
	return CORBA_TRUE;
}
