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

#ifndef PYORBIT_H
#define PYORBIT_H

#include <Python.h>
#include <orbit/orbit.h>

typedef struct {
    PyObject_HEAD
    CORBA_Object objref;
} PyCORBA_Object;

typedef struct {
    PyObject_HEAD
    CORBA_ORB orb;
} PyCORBA_ORB;

typedef struct {
    PyObject_HEAD
    CORBA_TypeCode tc;
} PyCORBA_TypeCode;

typedef struct {
    PyObject_HEAD
    CORBA_any any;
} PyCORBA_Any;

struct _PyORBit_APIStruct {
    PyTypeObject *corba_object_type;
    PyTypeObject *corba_orb_type;
    PyTypeObject *corba_typecode_type;
    PyTypeObject *corba_any_type;
    PyTypeObject *portable_server_poa_type;
    PyTypeObject *portable_server_poamanager_type;

    PyObject *(* corba_object_new)(CORBA_Object objref);
    PyObject *(* corba_orb_new)(CORBA_ORB orb);
    PyObject *(* corba_typecode_new)(CORBA_TypeCode tc);
    PyObject *(* corba_any_new)(CORBA_any *any);
    PyObject *(* poa_new)(PortableServer_POA poa);
    PyObject *(* poamanager_new)(PortableServer_POAManager poa);

    gboolean  (* check_ex)(CORBA_Environment *ev);
    gboolean  (* marshal_any)(CORBA_any *any, PyObject *value);
    PyObject *(* demarshal_any)(CORBA_any *any);

};

#ifndef _INSIDE_PYORBIT_

#if defined(NO_IMPORT) || defined(NO_IMPORT_PYORBIT)
extern struct _PyORBit_APIStruct *_PyORBit_API;
#else
struct _PyORBit_APIStruct *_PyORBit_API;

/* macro used to initialise the module */
#define init_pyorbit() { \
    PyObject *pyorbit = PyImport_ImportModule("ORBit"); \
    if (pyorbit != NULL) { \
        PyObject *module_dict = PyModule_GetDict(pyorbit); \
        PyObject *cobject = PyDict_GetItemString(module_dict, "_PyORBit_API");\
        if (PyCObject_Check(cobject)) \
            _PyORBit_API = (struct _PyORBit_APIStruct *) \
                PyCObject_AsVoidPtr(cobject); \
        else { \
            Py_FatalError("could not find _PyORBit_API object"); \
            return; \
        } \
    } else { \
        Py_FatalError("could not import ORBit module"); \
        return; \
    } \
}

#endif

/* types */
#define PyCORBA_Object_Type       *(_PyORBit_API->corba_object_type)
#define PyCORBA_ORB_Type          *(_PyORBit_API->corba_orb_type)
#define PyCORBA_TypeCode_Type     *(_PyORBit_API->corba_typecode_type)
#define PyCORBA_Any_Type          *(_PyORBit_API->corba_any_type)
#define PyPortableServer_POA_Type *(_PyORBit_API->portable_server_poa_type)
#define PyPortableServer_POAManager_Type *(_PyORBit_API->portable_server_poamanager_type)

/* constructors for above types ... */
#define pycorba_object_new        (* _PyORBit_API->corba_object_new)
#define pycorba_orb_new           (* _PyORBit_API->corba_orb_new)
#define pycorba_typecode_new      (* _PyORBit_API->corba_typecode_new)
#define pycorba_any_new           (* _PyORBit_API->corba_any_new)
#define pyorbit_poa_new           (* _PyORBit_API->poa_new)
#define pyorbit_poamanager_new    (* _PyORBit_API->poamanager_new)

/* utility functions */
#define pyorbit_check_ex          (* _PyORBit_API->check_ex)
#define pyorbit_marshal_any       (* _PyORBit_API->marshal_any)
#define pyorbit_demarshal_any     (* _PyORBit_API->demarshal_any)

#endif

#endif
