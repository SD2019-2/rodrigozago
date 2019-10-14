/* -*- mode: C; c-basic-offset: 4 -*-
 * pyorbit - a Python language mapping for the ORBit2 CORBA ORB
 * Copyright (C) 2002-2003  James Henstridge <james@daa.com.au>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include "pyorbit-private.h"

typedef struct {
    PyObject_HEAD
    ORBit_IMethod *imethod;
    PyObject *meth_class;
} PyCORBA_Method;

typedef struct {
    PyObject_HEAD
    PyCORBA_Method *meth;
    PyObject *meth_self;
} PyCORBA_BoundMethod;

static void
pycorba_method_dealloc(PyCORBA_Method *self)
{
    Py_DECREF(self->meth_class);
    PyObject_DEL(self);
}

static PyObject *
pycorba_method_repr(PyCORBA_Method *self)
{
    return PyString_FromFormat("<unbound CORBA method %s.%s>",
			       ((PyTypeObject *)self->meth_class)->tp_name,
			       self->imethod->name);
}

/* XXXX handle keyword arguments? */
static PyObject *
pycorba_method_call(PyCORBA_Method *self, PyObject *args, PyObject *kwargs)
{
    ORBit_IMethod *imethod = self->imethod;
    CORBA_Object objref;
    CORBA_Environment ev;
    CORBA_TypeCode ret_tc;
    gpointer ret = NULL, retptr, *argv = NULL, *argvptr = NULL;
    PyObject *obj, *item, *pyret = NULL;
    gint n_args, n_rets, argpos, num_args, i, retpos;
    gboolean has_ret;

    /* unaliased argument types ... */
    ret_tc = self->imethod->ret;
    while (ret_tc && ret_tc->kind == CORBA_tk_alias)
	ret_tc = ret_tc->subtypes[0];
    has_ret = ret_tc != CORBA_OBJECT_NIL && ret_tc->kind != CORBA_tk_void;
    /* calculate number of in or inout arguments */
    n_args = 0;
    n_rets = 0;
    for (i = 0; i < imethod->arguments._length; i++) {
	if ((self->imethod->arguments._buffer[i].flags &
	     (ORBit_I_ARG_IN | ORBit_I_ARG_INOUT)) != 0)
	    n_args++;
	if ((self->imethod->arguments._buffer[i].flags &
	     (ORBit_I_ARG_OUT | ORBit_I_ARG_INOUT)) != 0)
	    n_rets++;
    }
    if (PyTuple_Size(args) != n_args + 1) {
	PyErr_SetString(PyExc_TypeError, "wrong number of arguments");
	return NULL;
    }

    obj = PyTuple_GetItem(args, 0);
    if (!PyObject_TypeCheck(obj, (PyTypeObject *)self->meth_class)) {
	PyErr_SetString(PyExc_TypeError, "wrong object type as first arg");
	return NULL;
    }
    objref = ((PyCORBA_Object *)obj)->objref;

    num_args = imethod->arguments._length;
    
    /* set up return value argument, depending on type */
    if (has_ret)
	switch (ret_tc->kind) {
	case CORBA_tk_any:
	case CORBA_tk_sequence:
	case CORBA_tk_array:
	    retptr = NULL;
	    ret = &retptr;
	    break;
	case CORBA_tk_struct:
	case CORBA_tk_union:
	    /* handle non-fixed size structs, arrays and unions like any/seq */
	    if ((imethod->flags & ORBit_I_COMMON_FIXED_SIZE) == 0) {
		retptr = NULL;
		ret = &retptr;
		break;
	    }
	    /* else fall through */
	default:
	    ret = ORBit_small_alloc(imethod->ret);
	}
    argv = g_new0(gpointer, num_args);
    argvptr = g_new0(gpointer, num_args);
    for (i = 0; i < num_args; i++) {
	gint flags = self->imethod->arguments._buffer[i].flags;
	CORBA_TypeCode tc = self->imethod->arguments._buffer[i].tc;

	if ((flags & (ORBit_I_ARG_IN | ORBit_I_ARG_INOUT)) != 0) {
	    argv[i] = ORBit_small_alloc(tc);
	} else { /* OUT */
	    argv[i] = &argvptr[i];
	    /* is it a "variable length" type? */
	    if (tc->kind == CORBA_tk_any || tc->kind == CORBA_tk_sequence ||
		((tc->kind == CORBA_tk_struct || tc->kind == CORBA_tk_union ||
		  tc->kind == CORBA_tk_array) &&
		 (flags & ORBit_I_COMMON_FIXED_SIZE) == 0))
		argvptr[i] = NULL;
	    else
		argvptr[i] = ORBit_small_alloc(tc);
	}
    }

    argpos = 1;
    for (i = 0; i < num_args; i++) {
	gint flags = self->imethod->arguments._buffer[i].flags;

	if ((flags & (ORBit_I_ARG_IN | ORBit_I_ARG_INOUT)) != 0) {
	    CORBA_any any = { NULL, NULL, FALSE };

	    any._type = imethod->arguments._buffer[i].tc;
	    any._value = argv[i];
	    item = PyTuple_GetItem(args, argpos++);
	    if (!pyorbit_marshal_any(&any, item)) {
		PyErr_Format(PyExc_TypeError, "could not marshal arg '%s'",
			     imethod->arguments._buffer[i].name ?
			     imethod->arguments._buffer[i].name : "<unknown>");
		goto cleanup;
	    }
	}
    }
    CORBA_exception_init(&ev);
    ORBit_small_invoke_stub(objref, imethod, ret, argv,
			    CORBA_OBJECT_NIL, &ev);

    if (pyorbit_check_ex(&ev))
	goto cleanup;
    CORBA_exception_free(&ev);

    pyret = PyTuple_New(n_rets + (has_ret ? 1 : 0));
    retpos = 0;
    if (has_ret) {
	CORBA_any any = { NULL, NULL, FALSE };

	any._type = imethod->ret;
	switch (ret_tc->kind) {
	case CORBA_tk_any:
	case CORBA_tk_sequence:
	case CORBA_tk_array:
	    any._value = *(gpointer *)ret;
	    break;
	case CORBA_tk_struct:
	case CORBA_tk_union:
	    /* handle non-fixed size structs, arrays and unions like any/seq */
	    if ((imethod->flags & ORBit_I_COMMON_FIXED_SIZE) == 0) {
		any._value = *(gpointer *)ret;
		break;
	    }
	    /* else fall through */
	default:
	    any._value = ret;
	}
	item = pyorbit_demarshal_any(&any);
	if (!item) {
	    Py_DECREF(pyret);
	    pyret = NULL;
	    PyErr_SetString(PyExc_TypeError,
			    "could not demarshal return value");
	    goto cleanup;
	}
	PyTuple_SetItem(pyret, retpos++, item);
    }
    for (i = 0; i < num_args; i++) {
	gint flags = self->imethod->arguments._buffer[i].flags;

	if ((flags & ORBit_I_ARG_OUT) != 0) {
	    CORBA_any any = { NULL, NULL, FALSE };

	    any._type = imethod->arguments._buffer[i].tc;
	    any._value = argvptr[i];
	    item = pyorbit_demarshal_any(&any);
	    if (!item) {
		Py_DECREF(pyret);
		pyret = NULL;
		PyErr_Format(PyExc_TypeError,
			     "could not demarshal return value '%s'",
			     imethod->arguments._buffer[i].name ?
			     imethod->arguments._buffer[i].name : "<unknown>");
		goto cleanup;
	    }
	    PyTuple_SetItem(pyret, retpos++, item);
	} else if ((flags & (ORBit_I_ARG_INOUT)) != 0) {
	    CORBA_any any = { NULL, NULL, FALSE };

	    any._type = imethod->arguments._buffer[i].tc;
	    any._value = argv[i];
	    item = pyorbit_demarshal_any(&any);
	    if (!item) {
		Py_DECREF(pyret);
		pyret = NULL;
		PyErr_Format(PyExc_TypeError,
			     "could not demarshal return value '%s'",
			     imethod->arguments._buffer[i].name ?
			     imethod->arguments._buffer[i].name : "<unknown>");
		goto cleanup;
	    }
	    PyTuple_SetItem(pyret, retpos++, item);
	}
    }
    /* special case certain n_args cases */
    switch (PyTuple_Size(pyret)) {
    case 0:
	Py_DECREF(pyret);
	Py_INCREF(Py_None);
	pyret = Py_None;
	break;
    case 1:
	item = PyTuple_GetItem(pyret, 0);
	Py_INCREF(item);
	Py_DECREF(pyret);
	pyret = item;
	break;
    default:
	break;
    }

 cleanup:
    if (ret) {
	switch (ret_tc->kind) {
	case CORBA_tk_any:
	case CORBA_tk_sequence:
	case CORBA_tk_array:
	    CORBA_free(retptr);
	    break;
	case CORBA_tk_struct:
	case CORBA_tk_union:
	    /* handle non-fixed size structs, arrays and unions like any/seq */
	    if ((imethod->flags & ORBit_I_COMMON_FIXED_SIZE) == 0) {
		CORBA_free(retptr);
		break;
	    }
	    /* else fall through */
	default:
	    CORBA_free(ret);
	}
    }
    if (argv) {
	for (i = 0; i < num_args; i++) {
	    gint flags = self->imethod->arguments._buffer[i].flags;

	    if ((flags & ORBit_I_ARG_OUT) != 0)
		CORBA_free(argvptr[i]);
	    else
		CORBA_free(argv[i]);
	}
	g_free(argv);
	g_free(argvptr);
    }
    return pyret;
}

static PyObject *
pycorba_method_get_doc(PyCORBA_Method *self, void *closure)
{
    GString *string;
    gint i;
    gboolean has_arg;
    PyObject *ret;

    string = g_string_new(NULL);
    g_string_append(string, self->imethod->name);
    g_string_append_c(string, '(');
    has_arg = FALSE;
    for (i = 0; i < self->imethod->arguments._length; i++) {
	if ((self->imethod->arguments._buffer[i].flags &
	     (ORBit_I_ARG_IN | ORBit_I_ARG_INOUT)) != 0) {
	    const gchar *argname = self->imethod->arguments._buffer[i].name;
	    g_string_append(string, argname ? argname : "arg");
	    g_string_append(string, ", ");
	    has_arg = TRUE;
	}
    }
    if (has_arg) g_string_truncate(string, string->len - 2); /* ", " */
    g_string_append(string, ") -> ");
    has_arg = FALSE;
    if (self->imethod->ret != CORBA_OBJECT_NIL) {
	g_string_append_c(string, '\'');
	g_string_append(string, self->imethod->ret->repo_id);
	g_string_append(string, "', ");
	has_arg = TRUE;
    }
    for (i = 0; i < self->imethod->arguments._length; i++) {
	if ((self->imethod->arguments._buffer[i].flags &
	     (ORBit_I_ARG_OUT | ORBit_I_ARG_INOUT)) != 0) {
	    g_string_append(string, self->imethod->arguments._buffer[i].name);
	    g_string_append(string, ", ");
	    has_arg = TRUE;
	}
    }
    if (has_arg)
	g_string_truncate(string, string->len - 2); /* ", " */
    else
	g_string_truncate(string, string->len - 4); /* " -> " */

    ret = PyString_FromString(string->str);
    g_string_free(string, TRUE);
    return ret;
}

static PyObject *
pycorba_method_get_name(PyCORBA_Method *self, void *closure)
{
    return PyString_FromString(self->imethod->name);
}

static PyObject *
pycorba_method_get_class(PyCORBA_Method *self, void *closure)
{
    Py_INCREF(self->meth_class);
    return self->meth_class;
}

static PyGetSetDef pycorba_method_getsets[] = {
    { "__doc__",  (getter)pycorba_method_get_doc,   (setter)0 },
    { "__name__", (getter)pycorba_method_get_name,  (setter)0 },
    { "im_name",  (getter)pycorba_method_get_name,  (setter)0 },
    { "im_class", (getter)pycorba_method_get_class, (setter)0 },
    { NULL,       (getter)0,                  (setter)0 }
};

static PyObject *
pycorba_method_descr_get(PyCORBA_Method *self, PyObject *obj, PyObject *type)
{
    PyCORBA_BoundMethod *bmeth;

    if (obj == NULL || obj == Py_None) {
	Py_INCREF(self);
	return (PyObject *)self;
    }

    bmeth = PyObject_NEW(PyCORBA_BoundMethod, &PyCORBA_BoundMethod_Type);
    if (!bmeth)
	return NULL;
    Py_INCREF(self);
    bmeth->meth = self;
    Py_INCREF(obj);
    bmeth->meth_self = obj;
    
    return (PyObject *)bmeth;
}

PyTypeObject PyCORBA_Method_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                  /* ob_size */
    "ORBit.Method",                     /* tp_name */
    sizeof(PyCORBA_Method),             /* tp_basicsize */
    0,                                  /* tp_itemsize */
    /* methods */
    (destructor)pycorba_method_dealloc, /* tp_dealloc */
    (printfunc)0,                       /* tp_print */
    (getattrfunc)0,                     /* tp_getattr */
    (setattrfunc)0,                     /* tp_setattr */
    (cmpfunc)0,                         /* tp_compare */
    (reprfunc)pycorba_method_repr,      /* tp_repr */
    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
    (hashfunc)0,                        /* tp_hash */
    (ternaryfunc)pycorba_method_call,   /* tp_call */
    (reprfunc)0,                        /* tp_str */
    (getattrofunc)0,                    /* tp_getattro */
    (setattrofunc)0,                    /* tp_setattro */
    0,                                  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                 /* tp_flags */
    NULL, /* Documentation string */
    (traverseproc)0,                    /* tp_traverse */
    (inquiry)0,                         /* tp_clear */
    (richcmpfunc)0,                     /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    (getiterfunc)0,                     /* tp_iter */
    (iternextfunc)0,                    /* tp_iternext */
    0,                                  /* tp_methods */
    0,                                  /* tp_members */
    pycorba_method_getsets,             /* tp_getset */
    (PyTypeObject *)0,                  /* tp_base */
    (PyObject *)0,                      /* tp_dict */
    (descrgetfunc)pycorba_method_descr_get,   /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    (initproc)0,                        /* tp_init */
    0,                                  /* tp_alloc */
    0,                                  /* tp_new */
    0,                                  /* tp_free */
    (inquiry)0,                         /* tp_is_gc */
    (PyObject *)0,                      /* tp_bases */
};

static void
pycorba_bound_method_dealloc(PyCORBA_BoundMethod *self)
{
    Py_DECREF(self->meth);
    Py_DECREF(self->meth_self);
    PyObject_DEL(self);
}

static PyObject *
pycorba_bound_method_repr(PyCORBA_BoundMethod *self)
{
    return PyString_FromFormat("<bound CORBA method %s.%s>",
			       self->meth->meth_class->ob_type->tp_name,
			       self->meth->imethod->name);
}

static PyObject *
pycorba_bound_method_call(PyCORBA_BoundMethod *self, PyObject *args, PyObject *kwargs)
{
    PyObject *selfarg, *ret;

    selfarg = PyTuple_New(1);
    Py_INCREF(self->meth_self);
    PyTuple_SetItem(selfarg, 0, self->meth_self);
    args = PySequence_Concat(selfarg, args);
    Py_DECREF(selfarg);

    ret = pycorba_method_call(self->meth, args, kwargs);
    Py_DECREF(args);

    return ret;
}

static PyObject *
pycorba_bound_method_get_doc(PyCORBA_BoundMethod *self, void *closure)
{
    return pycorba_method_get_doc(self->meth, closure);
}

static PyObject *
pycorba_bound_method_get_name(PyCORBA_BoundMethod *self, void *closure)
{
    return PyString_FromString(self->meth->imethod->name);
}

static PyObject *
pycorba_bound_method_get_class(PyCORBA_BoundMethod *self, void *closure)
{
    Py_INCREF(self->meth->meth_class);
    return self->meth->meth_class;
}

static PyObject *
pycorba_bound_method_get_self(PyCORBA_BoundMethod *self, void *closure)
{
    Py_INCREF(self->meth_self);
    return self->meth_self;
}

static PyGetSetDef pycorba_bound_method_getsets[] = {
    { "__doc__",  (getter)pycorba_bound_method_get_doc,   (setter)0 },
    { "__name__", (getter)pycorba_bound_method_get_name,  (setter)0 },
    { "im_name",  (getter)pycorba_bound_method_get_name,  (setter)0 },
    { "im_class", (getter)pycorba_bound_method_get_class, (setter)0 },
    { "im_self",  (getter)pycorba_bound_method_get_self,  (setter)0 },
    { NULL,       (getter)0,                       (setter)0 }
};

PyTypeObject PyCORBA_BoundMethod_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                  /* ob_size */
    "ORBit.BoundMethod",                /* tp_name */
    sizeof(PyCORBA_BoundMethod),        /* tp_basicsize */
    0,                                  /* tp_itemsize */
    /* methods */
    (destructor)pycorba_bound_method_dealloc, /* tp_dealloc */
    (printfunc)0,                       /* tp_print */
    (getattrfunc)0,                     /* tp_getattr */
    (setattrfunc)0,                     /* tp_setattr */
    (cmpfunc)0,                         /* tp_compare */
    (reprfunc)pycorba_bound_method_repr, /* tp_repr */
    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
    (hashfunc)0,                        /* tp_hash */
    (ternaryfunc)pycorba_bound_method_call, /* tp_call */
    (reprfunc)0,                        /* tp_str */
    (getattrofunc)0,                    /* tp_getattro */
    (setattrofunc)0,                    /* tp_setattro */
    0,                                  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                 /* tp_flags */
    NULL, /* Documentation string */
    (traverseproc)0,                    /* tp_traverse */
    (inquiry)0,                         /* tp_clear */
    (richcmpfunc)0,                     /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    (getiterfunc)0,                     /* tp_iter */
    (iternextfunc)0,                    /* tp_iternext */
    0,                                  /* tp_methods */
    0,                                  /* tp_members */
    pycorba_bound_method_getsets,       /* tp_getset */
    (PyTypeObject *)0,                  /* tp_base */
    (PyObject *)0,                      /* tp_dict */
    (descrgetfunc)0,                    /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    (initproc)0,                        /* tp_init */
    0,                                  /* tp_alloc */
    0,                                  /* tp_new */
    0,                                  /* tp_free */
    (inquiry)0,                         /* tp_is_gc */
    (PyObject *)0,                      /* tp_bases */
};


void
pyorbit_add_imethods_to_stub(PyObject *stub, ORBit_IMethods *imethods)
{
    PyObject *tp_dict;
    int i;

    g_return_if_fail(PyType_Check(stub) && PyType_IsSubtype((PyTypeObject *)stub, &PyCORBA_Object_Type));

    tp_dict = ((PyTypeObject *)stub)->tp_dict;
    for (i = 0; i < imethods->_length; i++) {
	PyCORBA_Method *meth;
	gchar *pyname;

	meth = PyObject_NEW(PyCORBA_Method, &PyCORBA_Method_Type);
	if (!meth)
	    return;
	Py_INCREF(stub);
	meth->meth_class = stub;
	meth->imethod = &imethods->_buffer[i];
	pyname = _pyorbit_escape_name(meth->imethod->name);
	PyDict_SetItemString(tp_dict, pyname, (PyObject *)meth);
	g_free(pyname);
	Py_DECREF(meth);
    }

    /* set up property descriptors for interface attributes */
    for (i = 0; i < imethods->_length; i++) {
	ORBit_IMethod *imethod = &imethods->_buffer[i];

	if (!strncmp(imethod->name, "_get_", 4)) {
	    PyObject *fget, *fset, *property;
	    gchar *name;

	    fget = PyDict_GetItemString(tp_dict, imethod->name);
	    name = g_strdup(imethod->name);
	    name[1] = 's';
	    fset = PyDict_GetItemString(tp_dict, name);
	    g_free(name);
	    if (!fset)
		PyErr_Clear();

	    name = g_strconcat(&imethod->name[5], ": ", imethod->ret->repo_id,
			       (fset ? "" : " (readonly)"), NULL);
	    property = PyObject_CallFunction((PyObject *)&PyProperty_Type,
					"OOOs", fget, fset ? fset : Py_None,
					Py_None, name);
	    g_free(name);

	    name = _pyorbit_escape_name(&imethod->name[5]);
	    PyDict_SetItemString(tp_dict, name, property);
	    g_free(name);

	    Py_DECREF(property);
	    Py_DECREF(fget);
	    Py_XDECREF(fset);
	}
    }
}
