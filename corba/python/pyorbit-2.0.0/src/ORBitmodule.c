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
#include <sysmodule.h>
#include <orbit/poa/poa-types.h>
#ifdef HAVE_ORBIT2_IMODULE
#  include <ORBitservices/orbit-imodule.h>
#endif

PortableServer_POA _pyorbit_poa;

static void
pyorbit_handle_types_and_interfaces(CORBA_sequence_ORBit_IInterface *ifaces,
				    CORBA_sequence_CORBA_TypeCode *types,
				    const gchar *file)
{
    gint i;

    for (i = 0; i < ifaces->_length; i++) {
	if (ifaces->_buffer[i].tc->kind == CORBA_tk_null)
	    g_warning("%s is possibly broken: tc->kind == tk_null",
		      file);

	pyorbit_generate_iinterface_stubs(&ifaces->_buffer[i]);
	_pyorbit_register_skel(&ifaces->_buffer[i]);
    }

    for (i = 0; i < types->_length; i++) {
	CORBA_TypeCode tc = types->_buffer[i];

	if (tc->kind == CORBA_tk_null ||
	    (tc->kind==CORBA_tk_alias && tc->subtypes[0]->kind==CORBA_tk_null))
	    g_warning("%s is possibly broken: tc->kind == tk_null",
		      file);
	pyorbit_generate_typecode_stubs(tc);
    }
}

static PyObject *
pyorbit_load_typelib(PyObject *self, PyObject *args)
{
    gchar *typelib;
    CORBA_sequence_ORBit_IInterface *ifaces;
    CORBA_sequence_CORBA_TypeCode *types;

    if (!PyArg_ParseTuple(args, "s", &typelib))
	return NULL;
    if (!ORBit_small_load_typelib(typelib)) {
	PyErr_SetString(PyExc_RuntimeError, "could not load typelib");
	return NULL;
    }
    ifaces = ORBit_small_get_iinterfaces(typelib);
    types = ORBit_small_get_types(typelib);

    pyorbit_handle_types_and_interfaces(ifaces, types, typelib);

    CORBA_free(ifaces);
    CORBA_free(types);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pyorbit_load_file(PyObject *self, PyObject *args)
{
#ifdef HAVE_ORBIT2_IMODULE
    gchar *path, *cpp_args = "";
    CORBA_sequence_ORBit_IInterface *ifaces;
    CORBA_sequence_CORBA_TypeCode *types;

    if (!PyArg_ParseTuple(args, "s|s", &path, &cpp_args))
	return NULL;
    ifaces = ORBit_iinterfaces_from_file(path, cpp_args, &types);
    if (!ifaces) {
	PyErr_Format(PyExc_RuntimeError, "could not load '%s'", path);
	return NULL;
    }

    pyorbit_handle_types_and_interfaces(ifaces, types, path);

#if 0
    CORBA_free(ifaces);
    CORBA_free(types);
#endif

    Py_INCREF(Py_None);
    return Py_None;
#else
    PyErr_SetString(PyExc_RuntimeError,
		    "pyorbit was not configured with support for the "
		    "imodule service");
    return NULL;
#endif
}

static PyMethodDef orbit_functions[] = {
    { "load_typelib", pyorbit_load_typelib, METH_VARARGS },
    { "load_file",    pyorbit_load_file,    METH_VARARGS },
    { NULL, NULL, 0 }
};

static PyObject *
pyorbit_corba_orb_init(PyObject *self, PyObject *args)
{
    PyObject *py_argv = NULL;
    gchar *orb_id = "orbit-local-orb";
    int argc, i;
    gchar **argv;
    CORBA_Environment ev;
    CORBA_ORB orb;
    PyObject *pyorb;

    if (!PyArg_ParseTuple(args, "|O!s:CORBA.ORB_init",
			  &PyList_Type, &py_argv, &orb_id))
	return NULL;
    if (py_argv && PyList_Size(py_argv) > 0) {
	argc = PyList_Size(py_argv);
	argv = g_new(gchar *, argc);
	for (i = 0; i < argc; i++) {
	    PyObject *item = PyList_GetItem(py_argv, i);
	    
	    if (!PyString_Check(item)) {
		PyErr_SetString(PyExc_TypeError,
				"argv must be a list of strings");
		g_free(argv);
		return NULL;
	    }
	}
    } else {
	argc = 1;
	argv = g_new(gchar *, argc);
	argv[0] = "python";
    }
    CORBA_exception_init(&ev);
    orb = CORBA_ORB_init(&argc, argv, orb_id, &ev);
    g_free(argv);
    _pyorbit_poa = (PortableServer_POA) CORBA_ORB_resolve_initial_references(orb, "RootPOA", &ev);
    if (pyorbit_check_ex(&ev))
	return NULL;
    PortableServer_POAManager_activate (PortableServer_POA__get_the_POAManager (_pyorbit_poa, &ev), &ev);
    if (pyorbit_check_ex(&ev))
	return NULL;

    pyorb = pycorba_orb_new(orb);
    CORBA_Object_duplicate((CORBA_Object)orb, NULL);
    return pyorb;
}

static PyMethodDef corba_functions[] = {
    { "ORB_init", pyorbit_corba_orb_init, METH_VARARGS },
    { NULL, NULL, 0 }
};

static void
add_tc_constants(PyObject *corbamod)
{
    PyModule_AddObject(corbamod, "TC_null",
		       pycorba_typecode_new(TC_null));
    PyModule_AddObject(corbamod, "TC_void",
		       pycorba_typecode_new(TC_void));
    PyModule_AddObject(corbamod, "TC_short",
		       pycorba_typecode_new(TC_CORBA_short));
    PyModule_AddObject(corbamod, "TC_long",
		       pycorba_typecode_new(TC_CORBA_long));
    PyModule_AddObject(corbamod, "TC_longlong",
		       pycorba_typecode_new(TC_CORBA_long_long));
    PyModule_AddObject(corbamod, "TC_ushort",
		       pycorba_typecode_new(TC_CORBA_unsigned_short));
    PyModule_AddObject(corbamod, "TC_ulong",
		       pycorba_typecode_new(TC_CORBA_unsigned_long));
    PyModule_AddObject(corbamod, "TC_ulonglong",
		       pycorba_typecode_new(TC_CORBA_unsigned_long_long));
    PyModule_AddObject(corbamod, "TC_float",
		       pycorba_typecode_new(TC_CORBA_float));
    PyModule_AddObject(corbamod, "TC_double",
		       pycorba_typecode_new(TC_CORBA_double));
    PyModule_AddObject(corbamod, "TC_longdouble",
		       pycorba_typecode_new(TC_CORBA_long_double));
    PyModule_AddObject(corbamod, "TC_boolean",
		       pycorba_typecode_new(TC_CORBA_boolean));
    PyModule_AddObject(corbamod, "TC_char",
		       pycorba_typecode_new(TC_CORBA_char));
    PyModule_AddObject(corbamod, "TC_wchar",
		       pycorba_typecode_new(TC_CORBA_wchar));
    PyModule_AddObject(corbamod, "TC_octet",
		       pycorba_typecode_new(TC_CORBA_octet));
    PyModule_AddObject(corbamod, "TC_any",
		       pycorba_typecode_new(TC_CORBA_any));
    PyModule_AddObject(corbamod, "TC_TypeCode",
		       pycorba_typecode_new(TC_CORBA_TypeCode));
    PyModule_AddObject(corbamod, "TC_Object",
		       pycorba_typecode_new(TC_CORBA_Object));
    PyModule_AddObject(corbamod, "TC_string",
		       pycorba_typecode_new(TC_CORBA_string));
    PyModule_AddObject(corbamod, "TC_wstring",
		       pycorba_typecode_new(TC_CORBA_wstring));
}

static void
register_corba_types(void)
{
    pyorbit_register_stub(TC_null, NULL);
    pyorbit_register_stub(TC_void, NULL);
    pyorbit_register_stub(TC_CORBA_short, NULL);
    pyorbit_register_stub(TC_CORBA_long, NULL);
    pyorbit_register_stub(TC_CORBA_long_long, NULL);
    pyorbit_register_stub(TC_CORBA_unsigned_short, NULL);
    pyorbit_register_stub(TC_CORBA_unsigned_long, NULL);
    pyorbit_register_stub(TC_CORBA_unsigned_long_long, NULL);
    pyorbit_register_stub(TC_CORBA_float, NULL);
    pyorbit_register_stub(TC_CORBA_double, NULL);
    pyorbit_register_stub(TC_CORBA_long_double, NULL);
    pyorbit_register_stub(TC_CORBA_boolean, NULL);
    pyorbit_register_stub(TC_CORBA_char, NULL);
    pyorbit_register_stub(TC_CORBA_wchar, NULL);
    pyorbit_register_stub(TC_CORBA_octet, NULL);
    pyorbit_register_stub(TC_CORBA_any, NULL);
    pyorbit_register_stub(TC_CORBA_TypeCode,(PyObject*)&PyCORBA_TypeCode_Type);
    pyorbit_register_stub(TC_CORBA_Object, (PyObject *)&PyCORBA_Object_Type);
    pyorbit_register_stub(TC_CORBA_string, NULL);
    pyorbit_register_stub(TC_CORBA_wstring, NULL);

    /* pyorbit_generate_typecode_stubs(TC_CORBA_ConstructionPolicy); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_Current); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_DomainManager); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_Policy); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_AliasDef); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_ArrayDef); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_AttributeDef); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_ConstantDef); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_Contained); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_Container); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_EnumDef); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_ExceptionDef); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_FixedDef); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_IDLType); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_InterfaceDef); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_IRObject); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_ModuleDef); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_NativeDef); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_OperationDef); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_PrimitiveDef); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_Repository); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_SequenceDef); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_StringDef); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_StructDef); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_TypedefDef); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_UnionDef); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_ValueDef); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_ValueBoxDef); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_ValueMemberDef); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_WstringDef); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_Identifier); */
    pyorbit_generate_typecode_stubs(TC_CORBA_completion_status);
    pyorbit_generate_typecode_stubs(TC_CORBA_exception_type);
    /* pyorbit_generate_typecode_stubs(TC_CORBA_PolicyType); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_PolicyList); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_PolicyErrorCode); */
    pyorbit_generate_typecode_stubs(TC_CORBA_PolicyError);
    /* pyorbit_generate_typecode_stubs(TC_CORBA_DomainManagersList); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_ScopedName); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_RepositoryId); */
    pyorbit_generate_typecode_stubs(TC_CORBA_DefinitionKind);
    /* pyorbit_generate_typecode_stubs(TC_CORBA_VersionSpec); */
    /* ** pyorbit_generate_typecode_stubs(TC_CORBA_Contained_Description); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_InterfaceDefSeq); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_ValueDefSeq); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_ContainedSeq); */
    pyorbit_generate_typecode_stubs(TC_CORBA_StructMember);
    /* pyorbit_generate_typecode_stubs(TC_CORBA_StructMemberSeq); */
    pyorbit_generate_typecode_stubs(TC_CORBA_Initializer);
    /* pyorbit_generate_typecode_stubs(TC_CORBA_InitializerSeq); */
    pyorbit_generate_typecode_stubs(TC_CORBA_UnionMember);
    /* pyorbit_generate_typecode_stubs(TC_CORBA_UnionMemberSeq); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_EnumMemberSeq); */
    /* ** pyorbit_generate_typecode_stubs(TC_CORBA_Container_Description); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_Container_DescriptionSeq); */
    pyorbit_generate_typecode_stubs(TC_CORBA_PrimitiveKind);
    pyorbit_generate_typecode_stubs(TC_CORBA_ModuleDescription);
    pyorbit_generate_typecode_stubs(TC_CORBA_ConstantDescription);
    pyorbit_generate_typecode_stubs(TC_CORBA_TypeDescription);
    pyorbit_generate_typecode_stubs(TC_CORBA_ExceptionDescription);
    pyorbit_generate_typecode_stubs(TC_CORBA_AttributeMode);
    pyorbit_generate_typecode_stubs(TC_CORBA_AttributeDescription);
    pyorbit_generate_typecode_stubs(TC_CORBA_OperationMode);
    pyorbit_generate_typecode_stubs(TC_CORBA_ParameterMode);
    pyorbit_generate_typecode_stubs(TC_CORBA_ParameterDescription);
    /* pyorbit_generate_typecode_stubs(TC_CORBA_ParDescriptionSeq); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_ContextIdentifier); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_ContextIdSeq); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_ExceptionDefSeq); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_ExcDescriptionSeq); */
    pyorbit_generate_typecode_stubs(TC_CORBA_OperationDescription);
    /* pyorbit_generate_typecode_stubs(TC_CORBA_RepositoryIdSeq); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_OpDescriptionSeq); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_AttrDescriptionSeq); */
    /* ** pyorbit_generate_typecode_stubs(TC_CORBA_InterfaceDef_FullInterfaceDescription); */
    pyorbit_generate_typecode_stubs(TC_CORBA_InterfaceDescription);
    /* pyorbit_generate_typecode_stubs(TC_CORBA_Visibility); */
    pyorbit_generate_typecode_stubs(TC_CORBA_ValueMember);
    /* pyorbit_generate_typecode_stubs(TC_CORBA_ValueMemberSeq); */
    /* ** pyorbit_generate_typecode_stubs(TC_CORBA_ValueDef_FullValueDescription); */
    pyorbit_generate_typecode_stubs(TC_CORBA_ValueDescription);
    pyorbit_generate_typecode_stubs(TC_CORBA_TCKind);
    /* pyorbit_generate_typecode_stubs(TC_CORBA_ValueModifier); */
    pyorbit_generate_typecode_stubs(TC_CORBA_TypeCode_Bounds);
    pyorbit_generate_typecode_stubs(TC_CORBA_TypeCode_BadKind);
    /* pyorbit_generate_typecode_stubs(TC_CORBA_AnySeq); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_BooleanSeq); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_CharSeq); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_WCharSeq); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_OctetSeq); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_ShortSeq); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_UShortSeq); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_LongSeq); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_ULongSeq); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_LongLongSeq); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_ULongLongSeq); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_FloatSeq); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_DoubleSeq); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_Flags); */
    pyorbit_generate_typecode_stubs(TC_CORBA_NamedValue);
    pyorbit_generate_typecode_stubs(TC_CORBA_SetOverrideType);
    /* pyorbit_generate_typecode_stubs(TC_CORBA_RequestSeq); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_ServiceType); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_ServiceOption); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_ServiceDetailType); */
    pyorbit_generate_typecode_stubs(TC_CORBA_ServiceDetail);
    pyorbit_generate_typecode_stubs(TC_CORBA_ServiceInformation);
    /* pyorbit_generate_typecode_stubs(TC_CORBA_ORB_ObjectId); */
    /* pyorbit_generate_typecode_stubs(TC_CORBA_ORB_ObjectIdList); */
    pyorbit_generate_typecode_stubs(TC_CORBA_ORB_InvalidName);
}

static PyMethodDef portableserver_functions[] = {
    { NULL, NULL, 0 }
};

static struct _PyORBit_APIStruct api = {
    &PyCORBA_Object_Type,
    &PyCORBA_ORB_Type,
    &PyCORBA_TypeCode_Type,
    &PyCORBA_Any_Type,
    &PyPortableServer_POA_Type,
    &PyPortableServer_POAManager_Type,

    pycorba_object_new,
    pycorba_orb_new,
    pycorba_typecode_new,
    pycorba_any_new,
    pyorbit_poa_new,
    pyorbit_poamanager_new,

    pyorbit_check_ex,
    pyorbit_marshal_any,
    pyorbit_demarshal_any
};

void initORBit(void);

DL_EXPORT(void)
initORBit(void)
{
    PyObject *mod, *modules_dict, *corbamod, *psmod;

#define INIT_TYPE(tp) G_STMT_START{ \
    if (!tp.ob_type)  tp.ob_type  = &PyType_Type; \
    if (!tp.tp_alloc) tp.tp_alloc = PyType_GenericAlloc; \
    if (!tp.tp_new)   tp.tp_new   = PyType_GenericNew; \
    if (PyType_Ready(&tp) < 0) \
        return; }G_STMT_END

    INIT_TYPE(PyCORBA_TypeCode_Type);
    INIT_TYPE(PyCORBA_Object_Type);
    INIT_TYPE(PyCORBA_Method_Type);
    INIT_TYPE(PyCORBA_BoundMethod_Type);
    INIT_TYPE(PyCORBA_ORB_Type);
    INIT_TYPE(PyCORBA_Any_Type);
    INIT_TYPE(PyCORBA_Struct_Type);
    INIT_TYPE(PyCORBA_Union_Type);
    INIT_TYPE(PyCORBA_UnionMember_Type);
    PyCORBA_Enum_Type.tp_base = &PyInt_Type;
    INIT_TYPE(PyCORBA_Enum_Type);
    INIT_TYPE(PyCORBA_fixed_Type);
    INIT_TYPE(PyPortableServer_Servant_Type);
    INIT_TYPE(PyPortableServer_POA_Type);
    INIT_TYPE(PyPortableServer_POAManager_Type);

#undef INIT_TYPE

    modules_dict = PySys_GetObject("modules");

    mod = Py_InitModule("ORBit", orbit_functions);

    PyModule_AddObject(mod, "orbit_version",
		       Py_BuildValue("(iii)",
				     orbit_major_version,
				     orbit_minor_version,
				     orbit_micro_version));

    PyModule_AddObject(mod, "__version__",
		       Py_BuildValue("(iii)",
				     PYORBIT_MAJOR_VERSION,
				     PYORBIT_MINOR_VERSION,
				     PYORBIT_MICRO_VERSION));

    PyModule_AddObject(mod, "_PyORBit_API",
		       PyCObject_FromVoidPtr(&api, NULL));

    corbamod = Py_InitModule("ORBit.CORBA", corba_functions);
    Py_INCREF(corbamod);
    PyModule_AddObject(mod, "CORBA", corbamod);
    PyDict_SetItemString(modules_dict, "CORBA", corbamod);

    PyModule_AddObject(corbamod, "TypeCode",
		       (PyObject *)&PyCORBA_TypeCode_Type);
    PyModule_AddObject(corbamod, "Object",
		       (PyObject *)&PyCORBA_Object_Type);
    PyModule_AddObject(corbamod, "ORB",
		       (PyObject *)&PyCORBA_ORB_Type);
    PyModule_AddObject(corbamod, "Any",
		       (PyObject *)&PyCORBA_Any_Type);
    PyModule_AddObject(corbamod, "fixed",
		       (PyObject *)&PyCORBA_fixed_Type);

    PyModule_AddObject(corbamod, "TRUE",  Py_True);
    PyModule_AddObject(corbamod, "FALSE", Py_False);

    pyorbit_register_exceptions(corbamod);

    register_corba_types();
    add_tc_constants(corbamod);

    psmod = Py_InitModule("ORBit.PortableServer", portableserver_functions);
    Py_INCREF(psmod);
    PyModule_AddObject(mod, "PortableServer", psmod);
    PyDict_SetItemString(modules_dict, "PortableServer", psmod);
    PyModule_AddObject(psmod, "POA",
		       (PyObject *)&PyPortableServer_POA_Type);
    PyModule_AddObject(psmod, "POAManager",
		       (PyObject *)&PyPortableServer_POAManager_Type);
    PyModule_AddObject(psmod, "Servant",
		       (PyObject *)&PyPortableServer_Servant_Type);
}
