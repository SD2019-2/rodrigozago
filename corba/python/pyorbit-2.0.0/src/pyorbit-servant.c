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

/* macro to simplify conditionals based on ORBit version */
#include <orbit/orbit-config.h>
#define ORBIT_VERSION_CHECK(x,y,z) \
  (ORBIT_MAJOR_VERSION > x || \
   (ORBIT_MAJOR_VERSION == x && (ORBIT_MINOR_VERSION > y || \
     (ORBIT_MINOR_VERSION == y && ORBIT_MICRO_VERSION >= z))))

#if !ORBIT_VERSION_CHECK(2,7,0)
#  define ORBIT2_INTERNAL_API
#endif
#include "pyorbit-private.h"
#include <structmember.h>
#include <orbit/poa/poa-types.h>
#include <orbit/orbit-config.h>


struct _PyORBitInterfaceInfo {
    ORBit_IInterface *iinterface;
    PortableServer_ClassInfo class_info;
    CORBA_unsigned_long classid;

    PyObject *poa_class, *stub_class;
    
    GHashTable *meth_hash;
    PortableServer_ServantBase__vepv *vepv;
};

PyTypeObject PyPortableServer_Servant_Type;

static ORBitSmallSkeleton impl_finder_func(PortableServer_ServantBase *servant,
					   const gchar *opname,
					   gpointer *m_data,
					   gpointer *impl);

#define MAX_CLASSID 512
static ORBit_VepvIdx *
get_fake_vepvmap(void)
{
    static ORBit_VepvIdx *fake_vepvmap = NULL;
    int i;

    if (!fake_vepvmap) {
	fake_vepvmap = g_new0 (ORBit_VepvIdx, MAX_CLASSID);
	for (i = 1; i < MAX_CLASSID; i++)
	    fake_vepvmap[i] = 1;
    }

    return fake_vepvmap;
}

void
_pyorbit_register_skel(ORBit_IInterface *iinterface)
{
    static GHashTable *interface_info_hash = NULL; 
    PyORBitInterfaceInfo *info;
    PyObject *instance_dict, *pyinfo, *container;
    gulong length, i, j, maxepvlen;

    if (!interface_info_hash)
	interface_info_hash = g_hash_table_new(g_str_hash, g_str_equal);

    if (g_hash_table_lookup(interface_info_hash, iinterface->tc->repo_id))
	return;

    info = g_new0(PyORBitInterfaceInfo, 1);
    info->iinterface = iinterface;
#if ORBIT_VERSION_CHECK(2,7,0)
    info->class_info.impl_finder = &impl_finder_func;
#else
    info->class_info.small_relay_call = &impl_finder_func;
#endif
    info->class_info.class_name = g_strdup(iinterface->tc->repo_id);
    info->class_info.class_id = &info->classid;
    info->class_info.idata = iinterface;
    info->class_info.vepvmap = get_fake_vepvmap();

    info->meth_hash = g_hash_table_new(g_str_hash, g_str_equal);

    g_assert(iinterface->base_interfaces._length >= 1);
    length = iinterface->base_interfaces._length - 1;

    info->vepv = (PortableServer_ServantBase__vepv *) g_new0(gpointer, 2);
    info->vepv[0] = g_new0(PortableServer_ServantBase__epv, 1);

    maxepvlen = iinterface->methods._length;
    for (i = 0; i < length; i++) {
	PyORBitInterfaceInfo *base_info;
	const CORBA_char *repo_id;

	repo_id = iinterface->base_interfaces._buffer[i];
	base_info = g_hash_table_lookup(interface_info_hash, repo_id);
	if (!base_info) {
	    g_warning("have not registered base interface '%s' needed by '%s'",
		      repo_id, iinterface->tc->repo_id);
	    continue;
	}

	maxepvlen = MAX(maxepvlen, base_info->iinterface->methods._length);
	for (j = 0; j < base_info->iinterface->methods._length; j++) {
	    ORBit_IMethod *imethod = &base_info->iinterface->methods._buffer[j];
	    g_hash_table_insert (info->meth_hash, imethod->name, imethod);
	}
    }
    /* empty epv large enough to cover all base epvs */
    info->vepv[1] = (PortableServer_ServantBase__vepv)g_new0(gpointer,
							     maxepvlen+1);


    instance_dict = PyDict_New();
    pyinfo = PyCObject_FromVoidPtr(info, NULL);
    PyDict_SetItemString(instance_dict, "__interface_info__", pyinfo);
    Py_DECREF(pyinfo);

    info->poa_class = PyObject_CallFunction((PyObject *)&PyType_Type, "s(O)O",
					    iinterface->tc->name,
					    (PyObject *)&PyPortableServer_Servant_Type,
					    instance_dict);
    Py_DECREF(instance_dict);

    for (i = 0; i < iinterface->methods._length; i++) {
	ORBit_IMethod *imethod = &iinterface->methods._buffer[i];

	g_hash_table_insert(info->meth_hash, imethod->name, imethod);
    }
    g_hash_table_insert(interface_info_hash, iinterface->tc->repo_id, info);

    /* add servant to module */
    container = _pyorbit_get_container(iinterface->tc->repo_id, TRUE);
    if (container) {
	gchar *pyname;

	pyname = _pyorbit_escape_name(iinterface->tc->name);
	if (PyType_Check(container)) {
	    PyObject *container_dict = ((PyTypeObject *)container)->tp_dict;

	    PyDict_SetItemString(container_dict, pyname, info->poa_class);
	} else {
	    PyObject_SetAttrString(container, pyname, info->poa_class);
	}
	g_free(pyname);
	Py_DECREF(container);
    }
}

static void
pyorbit_servant_generic_skel_func(PortableServer_ServantBase *servant,
				  gpointer retval, gpointer *argv,
				  gpointer ctx, CORBA_Environment *ev,
				  gpointer impl)
{
    PyPortableServer_Servant *pyservant;
    ORBit_IMethod *imethod;
    gchar *pyname;
    PyObject *method = NULL;
    CORBA_TypeCode ret_tc = NULL, *arg_tc = NULL;
    PyObject *args = NULL, *ret = NULL;
    gint n_args, n_rets, i, argpos, retpos;
    gboolean has_ret;

    pyservant = SERVANT_TO_PYSERVANT(servant);
    imethod = (ORBit_IMethod *)impl;

    /* look up the method on the servant */
    pyname = _pyorbit_escape_name(imethod->name);
    if (pyservant->delegate != Py_None)
	method = PyObject_GetAttrString(pyservant->delegate, pyname);
    else
	method = PyObject_GetAttrString((PyObject *)pyservant, pyname);
    g_free(pyname);
    if (!method) {
	PyErr_Clear();
	CORBA_exception_set_system(ev, ex_CORBA_NO_IMPLEMENT,
				   CORBA_COMPLETED_NO);
	goto cleanup;
    }

    /* count argument types */
    ret_tc = imethod->ret;
    while (ret_tc && ret_tc->kind == CORBA_tk_alias)
	ret_tc = ret_tc->subtypes[0];
    has_ret = ret_tc != CORBA_OBJECT_NIL && ret_tc->kind != CORBA_tk_void;
    arg_tc = g_new(CORBA_TypeCode, imethod->arguments._length);
    n_args = n_rets = 0;
    for (i = 0; i < imethod->arguments._length; i++) {
	if ((imethod->arguments._buffer[i].flags &
	     (ORBit_I_ARG_IN | ORBit_I_ARG_INOUT)) != 0)
	    n_args++;
	if ((imethod->arguments._buffer[i].flags &
	     (ORBit_I_ARG_OUT | ORBit_I_ARG_INOUT)) != 0)
	    n_rets++;
	arg_tc[i] = imethod->arguments._buffer[i].tc;
	while (arg_tc[i]->kind == CORBA_tk_alias)
	    arg_tc[i] = arg_tc[i]->subtypes[0];
    }

    /* demarshal arguments */
    args = PyTuple_New(n_args);
    argpos = 0;
    for (i = 0; i < imethod->arguments._length; i++) {
	gint flags = imethod->arguments._buffer[i].flags;

	if ((flags & (ORBit_I_ARG_IN | ORBit_I_ARG_INOUT)) != 0) {
	    CORBA_any any = { NULL, NULL, FALSE };
	    PyObject *item;

	    any._type = imethod->arguments._buffer[i].tc;
	    any._value = argv[i];
	    item = pyorbit_demarshal_any(&any);
	    if (!item) {
		CORBA_exception_set_system(ev, ex_CORBA_DATA_CONVERSION,
					   CORBA_COMPLETED_NO);
		goto cleanup;
	    }
	    PyTuple_SetItem(args, argpos++, item);
	}
    }

    /* invoke */
    ret = PyObject_CallObject(method, args);

    /* check exception */
    if (pyorbit_check_python_ex(ev))
	goto cleanup;

    /* if the result isn't a sequence, wrap it in a length 1 tuple */
    if (n_rets + (has_ret ? 1 : 0) == 0) {
	Py_DECREF(ret);
	ret = PyTuple_New(0);
    } else if (n_rets + (has_ret ? 1 : 0) == 1) {
	/* wrap a tuple round the return value ... */
	ret = Py_BuildValue("(N)", ret);
    } else if (n_rets + (has_ret ? 1 : 0) != PySequence_Length(ret)) {
	g_message("%s: return sequence length is wrong (expected %d, got %d)",
		  imethod->name, n_rets + (has_ret ? 1 : 0),
		  PySequence_Length(ret));
	CORBA_exception_set_system(ev, ex_CORBA_DATA_CONVERSION,
				   CORBA_COMPLETED_MAYBE);
	goto cleanup;
    }
    retpos = 0;
    if (has_ret) {
	CORBA_any any = { NULL, NULL, FALSE };
	PyObject *item;

	item = PySequence_GetItem(ret, retpos++);
	if (!item) {
	    PyErr_Clear();
	    g_message("%s: couldn't get return val", imethod->name);
	    CORBA_exception_set_system(ev, ex_CORBA_DATA_CONVERSION,
				       CORBA_COMPLETED_MAYBE);
	    goto cleanup;
	}
	any._type = imethod->ret;
	switch (ret_tc->kind) {
	case CORBA_tk_any:
	case CORBA_tk_sequence:
	case CORBA_tk_array:
	    *(gpointer *)retval = ORBit_small_alloc(ret_tc);
	    any._value = *(gpointer *)retval;
	    break;
	case CORBA_tk_struct:
	case CORBA_tk_union:
	    if ((imethod->flags & ORBit_I_COMMON_FIXED_SIZE) == 0) {
		*(gpointer *)retval = ORBit_small_alloc(ret_tc);
		any._value = *(gpointer *)retval;
		break;
	    }
	    /* else fall through */
	default:
	    any._value = retval;
	}
	if (!pyorbit_marshal_any(&any, item)) {
	    Py_DECREF(item);
	    g_message("%s: could not marshal return", imethod->name);
	    CORBA_exception_set_system(ev, ex_CORBA_DATA_CONVERSION,
				       CORBA_COMPLETED_MAYBE);
	    goto cleanup;
	}
    }

    /* handle inout and out args */
    for (i = 0; i < imethod->arguments._length; i++) {
	gint flags = imethod->arguments._buffer[i].flags;
	CORBA_TypeCode tc = arg_tc[i];
	PyObject *item;
	CORBA_any any = { NULL, NULL, FALSE };


	if ((flags & (ORBit_I_ARG_INOUT | ORBit_I_ARG_OUT)) == 0)
	    continue;

	/* read value from result sequence */
	item = PySequence_GetItem(ret, retpos++);
	if (!item) {
	    PyErr_Clear();
	    g_message("%s: could not get arg from tuple", imethod->name);
	    CORBA_exception_set_system(ev, ex_CORBA_DATA_CONVERSION,
				       CORBA_COMPLETED_MAYBE);
	    goto cleanup;
	}

	/* set up any */
	any._type = imethod->arguments._buffer[i].tc;
	if ((flags & ORBit_I_ARG_INOUT) != 0) {
	    /* clean up any internal allocations in the inout ... */
	    ORBit_small_freekids(tc, argv[i], NULL);
	    any._value = argv[i];
	} else if ((flags & ORBit_I_ARG_OUT) != 0) {
	    /* is it a variable length type?  If so, allocate storage. */
	    if (tc->kind == CORBA_tk_any || tc->kind == CORBA_tk_sequence ||
                ((tc->kind == CORBA_tk_struct || tc->kind == CORBA_tk_union ||
                  tc->kind == CORBA_tk_array) &&
                 (flags & ORBit_I_COMMON_FIXED_SIZE) == 0))
		*(gpointer *)argv[i] = ORBit_small_alloc(tc);

	    any._value = *(gpointer *)argv[i];
	}

	if (!pyorbit_marshal_any(&any, item)) {
	    Py_DECREF(item);
	    g_message("%s: could not marshal arg", imethod->name);
	    CORBA_exception_set_system(ev, ex_CORBA_DATA_CONVERSION,
				       CORBA_COMPLETED_MAYBE);
	    goto cleanup;
	}
    }

 cleanup:
    g_free(arg_tc);
    Py_XDECREF(method);
    Py_XDECREF(args);
    Py_XDECREF(ret);
}

static ORBitSmallSkeleton
impl_finder_func(PortableServer_ServantBase *servant,
		 const gchar *opname, gpointer *m_data, gpointer *impl)
{
    PyPortableServer_Servant *pyservant;
    gpointer value;
    ORBit_IMethod *imethod;

    pyservant = SERVANT_TO_PYSERVANT(servant);
    if (!g_hash_table_lookup_extended(pyservant->info->meth_hash, opname,
				      NULL, &value)) {
	return NULL;
    }
    imethod = (ORBit_IMethod *)value;

    *m_data = imethod; /* imethod */
    *impl = imethod;

    return pyorbit_servant_generic_skel_func;
}

static void
pyorbit_servant_dealloc(PyPortableServer_Servant *self)
{
    PortableServer_ServantBase *servant;

    Py_XDECREF(self->this);
    Py_XDECREF(self->delegate);
    self->this = NULL;
    servant = PYSERVANT_TO_SERVANT(self);
    PortableServer_ServantBase__fini(servant, NULL);

    if (self->ob_type->tp_free)
	self->ob_type->tp_free((PyObject *)self);
    else
	PyObject_DEL(self);
}

#define pyorbit_servant_repr 0


static PyObject *
pyorbit_servant_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    PyObject *pyinfo;
    PyORBitInterfaceInfo *info;
    PyPortableServer_Servant *self;
    PortableServer_ServantBase *servant;
    CORBA_Environment ev;

    /* get the InterfaceInfo struct ... */
    pyinfo = PyObject_GetAttrString((PyObject *)type, "__interface_info__");
    if (!pyinfo)
	return NULL;
    if (!PyCObject_Check(pyinfo)) {
	Py_DECREF(pyinfo);
	PyErr_SetString(PyExc_TypeError,
			"__interface_info__ attribute not a cobject");
	return NULL;
    }
    info = PyCObject_AsVoidPtr(pyinfo);
    Py_DECREF(pyinfo);

    self = (PyPortableServer_Servant *)type->tp_alloc(type, 0);
    self->info = info;
    self->delegate = Py_None;
    Py_INCREF(self->delegate);

    servant = PYSERVANT_TO_SERVANT(self);
    servant->vepv = info->vepv;
    ORBIT_SERVANT_SET_CLASSINFO(servant, &info->class_info);

    CORBA_exception_init(&ev);
    PortableServer_ServantBase__init(servant, &ev);
    if (pyorbit_check_ex(&ev)) {
	Py_DECREF(self);
	return NULL;
    }

    return (PyObject *)self;
}

static int
pyorbit_servant_init(PyPortableServer_Servant *self,
		     PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "delegate", NULL };
    PyObject *delegate = Py_None;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|O:Servant.__init__",
				     kwlist, &delegate))
	return -1;

    Py_XDECREF(self->delegate);
    self->delegate = delegate;
    Py_INCREF(self->delegate);

    return 0;
}

static PyObject *
pyorbit_servant__default_POA(PyPortableServer_Servant *self)
{
    if (!_pyorbit_poa) {
	Py_INCREF(Py_None);
	return Py_None;
    }
    CORBA_Object_duplicate((CORBA_Object)_pyorbit_poa, NULL);
    return pyorbit_poa_new(_pyorbit_poa);
}

static PyObject *
pyorbit_servant__this(PyPortableServer_Servant *self)
{
    CORBA_Environment ev;
    PyObject *pypoa;
    PortableServer_POA poa;
    PortableServer_ServantBase *servant;
    PortableServer_ObjectId *objid;
    CORBA_Object objref;

    if (self->this) {
	Py_INCREF(self->this);
	return self->this;
    }

    /* this is meant to call self._default_POA */
    pypoa = PyObject_CallMethod((PyObject *)self, "_default_POA", NULL);
    if (!pypoa)
	return NULL;
    if (!PyObject_TypeCheck(pypoa, &PyPortableServer_POA_Type)) {
	Py_DECREF(pypoa);
	PyErr_SetString(PyExc_TypeError, "could not lookup default POA");
	return NULL;
    }
    poa = (PortableServer_POA)((PyCORBA_Object *)pypoa)->objref;

    servant = PYSERVANT_TO_SERVANT(self);

    CORBA_exception_init(&ev);
    objid = PortableServer_POA_activate_object(poa, servant, &ev);
    CORBA_free(objid);
    if (pyorbit_check_ex(&ev)) {
	Py_DECREF(pypoa);
    	return NULL;
    }
    
    CORBA_exception_init(&ev);
    objref = PortableServer_POA_servant_to_reference(poa, servant, &ev);
    Py_DECREF(pypoa);
    if (pyorbit_check_ex(&ev))
    	return NULL;

    self->this = pycorba_object_new(objref);
    CORBA_Object_release(objref, NULL);
    Py_INCREF(self->this);
    return self->this;
}

static PyMethodDef pyorbit_servant_methods[] = {
    { "_default_POA", (PyCFunction)pyorbit_servant__default_POA, METH_NOARGS },
    { "_this", (PyCFunction)pyorbit_servant__this, METH_NOARGS },
    { NULL, NULL, 0 }
};

static PyMemberDef pyorbit_servant_members[] = {
    { "_delegate", T_OBJECT, offsetof(PyPortableServer_Servant, delegate),
      0, "delegate object" },
    { NULL, 0, 0, 0 }
};

PyTypeObject PyPortableServer_Servant_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                  /* ob_size */
    "PortableServer.Servant",           /* tp_name */
    sizeof(PyPortableServer_Servant),   /* tp_basicsize */
    0,                                  /* tp_itemsize */
    /* methods */
    (destructor)pyorbit_servant_dealloc, /* tp_dealloc */
    (printfunc)0,                       /* tp_print */
    (getattrfunc)0,                     /* tp_getattr */
    (setattrfunc)0,                     /* tp_setattr */
    (cmpfunc)0,                         /* tp_compare */
    (reprfunc)pyorbit_servant_repr,     /* tp_repr */
    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
    (hashfunc)0,                        /* tp_hash */
    (ternaryfunc)0,                     /* tp_call */
    (reprfunc)0,                        /* tp_str */
    (getattrofunc)0,                    /* tp_getattro */
    (setattrofunc)0,                    /* tp_setattro */
    0,                                  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /* tp_flags */
    NULL, /* Documentation string */
    (traverseproc)0,                    /* tp_traverse */
    (inquiry)0,                         /* tp_clear */
    (richcmpfunc)0,                     /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    (getiterfunc)0,                     /* tp_iter */
    (iternextfunc)0,                    /* tp_iternext */
    pyorbit_servant_methods,            /* tp_methods */
    pyorbit_servant_members,            /* tp_members */
    0,                                  /* tp_getset */
    (PyTypeObject *)0,                  /* tp_base */
    (PyObject *)0,                      /* tp_dict */
    (descrgetfunc)0,                    /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    (initproc)pyorbit_servant_init,     /* tp_init */
    0,                                  /* tp_alloc */
    (newfunc)pyorbit_servant_new,       /* tp_new */
    0,                                  /* tp_free */
    (inquiry)0,                         /* tp_is_gc */
    (PyObject *)0,                      /* tp_bases */
};
