#include "CORBAmodule.h"
#include "types.h"

PyObject *Union_PyClass__init(PyObject *_, PyObject *args)
{
	PyObject *self = PyTuple_GetItem(args, 0);
	PyObject *repo_obj = PyObject_GetAttrString(self, "__repo_id");
	CORBA_TypeCode tc = find_typecode(PyString_AsString(repo_obj));\
	// FIXME: throw exception
	if (!tc)
		g_warning("Can't find typecode for %s", tc->repo_id);
	else {
		PyObject *d = NULL, *v = NULL;
		PyArg_ParseTuple(args, "OO|O", &self, &d, &v);
		if (!v) { // no discriminator given
			v = d;
			if (tc->default_index == -1)
				d = Py_None;
			else
				d = PyInt_FromLong(tc->default_index);
		}
		else { // verify this discriminator exists
			CORBA_long i = find_union_arm(tc, d);
			if (i == -1) {
				g_warning("Unknown discriminator value and no default case");
				v = d = Py_None;
			}
			else d = PyInt_FromLong(i);
		}
		// XXX: should we validate v's type here?  For now, we'll let the
		// marshaller do that
		PyObject_SetAttrString(self, "d", d);
		PyObject_SetAttrString(self, "v", v);
	}
	Py_INCREF(Py_None);
	return Py_None;
}


#define F_M(cv, v, TYPE) \
	case cv: { \
		TYPE val = v; \
		CORBA_unsigned_long i; \
		for (i = 0; i < tc->sub_parts; i++) { \
			TYPE label_val = *(TYPE *)tc->sublabels[i]._value; \
			if (label_val == val) \
				return i; \
		} \
		break; \
	}


CORBA_long find_union_arm(CORBA_TypeCode tc, PyObject *d)
{
	g_assert(tc);
	if (d == Py_None)
		return tc->default_index;

	switch (tc->discriminator->kind) {
		F_M(CORBA_tk_short, PyInt_AsLong(d), CORBA_short);
		F_M(CORBA_tk_long, PyInt_AsLong(d), CORBA_long);
		F_M(CORBA_tk_ushort, PyInt_AsLong(d), CORBA_unsigned_short);
		F_M(CORBA_tk_ulong, PyInt_AsLong(d), CORBA_unsigned_long);
		F_M(CORBA_tk_longlong, PyInt_AsLong(d), CORBA_long_long);
		F_M(CORBA_tk_ulonglong, PyInt_AsLong(d), CORBA_unsigned_long_long);
		F_M(CORBA_tk_enum, PyInt_AsLong(d), CORBA_unsigned_long);
		case CORBA_tk_boolean:
		{
			CORBA_boolean val = PyInt_AsLong(d);
			CORBA_unsigned_long i;
			for (i = 0; i < tc->sub_parts; i++) {
				CORBA_boolean lv = *(CORBA_boolean *)tc->sublabels[i]._value;
				if (!lv == !val)
					return i;
			}
			break;
		}
		default:
			g_warning("unsupported discriminator: %d", tc->discriminator->kind);
			break;
	}
	return tc->default_index;
}	
		
