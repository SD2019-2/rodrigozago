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

#ifndef PYORBIT_PRIVATE_H
#define PYORBIT_PRIVATE_H

#ifdef PYORBIT_H
#  error "don't include pyorbit.h and pyorbit-private.h together"
#endif

#define _INSIDE_PYORBIT_
#include "pyorbit.h"
#undef _INSIDE_PYORBIT_

/* public API */
extern PyTypeObject PyCORBA_Object_Type;
extern PyTypeObject PyCORBA_ORB_Type;
extern PyTypeObject PyCORBA_TypeCode_Type;
extern PyTypeObject PyCORBA_Any_Type;

PyObject *pycorba_object_new(CORBA_Object objref);
PyObject *pycorba_orb_new(CORBA_ORB orb);
PyObject *pycorba_typecode_new(CORBA_TypeCode tc);
PyObject *pycorba_any_new(CORBA_any *any);

/* private API */

typedef struct {
    PyObject_VAR_HEAD

    CORBA_fixed_d_s fixed;
} PyCORBA_fixed;
extern PyTypeObject PyCORBA_fixed_Type;

void      pyorbit_register_stub(CORBA_TypeCode tc, PyObject *stub);
CORBA_TypeCode pyorbit_lookup_typecode(const gchar *repo_id);
PyObject *pyorbit_get_stub(CORBA_TypeCode tc);
PyObject *pyorbit_get_stub_from_repo_id(const gchar *repo_id);
PyObject *pyorbit_get_stub_from_objref(CORBA_Object objref);

void      pyorbit_generate_typecode_stubs(CORBA_TypeCode tc);
void      pyorbit_generate_iinterface_stubs(ORBit_IInterface *iface);

extern PyTypeObject PyCORBA_Method_Type;
extern PyTypeObject PyCORBA_BoundMethod_Type;
void      pyorbit_add_imethods_to_stub(PyObject *stub,
				       ORBit_IMethods *imethods);

gboolean  pyorbit_marshal_any(CORBA_any *any, PyObject *value);
PyObject *pyorbit_demarshal_any(CORBA_any *any);

extern PyObject *pyorbit_exception;
extern PyObject *pyorbit_system_exception;
extern PyObject *pyorbit_user_exception;

gboolean  pyorbit_check_ex(CORBA_Environment *ev);
gboolean  pyorbit_check_python_ex(CORBA_Environment *ev);
void      pyorbit_register_exceptions(PyObject *corbamod);

extern PyTypeObject PyCORBA_Struct_Type;
extern PyTypeObject PyCORBA_Union_Type;
extern PyTypeObject PyCORBA_UnionMember_Type;
void      pyorbit_add_union_members_to_stub(PyObject *stub, CORBA_TypeCode tc);

extern PyTypeObject PyCORBA_Enum_Type;
PyObject *_pyorbit_generate_enum(CORBA_TypeCode tc, PyObject **values_p);
PyObject *pycorba_enum_from_long(CORBA_TypeCode tc, long value);


/* utils */
gchar    *_pyorbit_escape_name(const gchar *name);
PyObject *_pyorbit_get_container(const gchar *repo_id, gboolean is_poa);

/* skels */

typedef struct _PyORBitInterfaceInfo PyORBitInterfaceInfo;
typedef struct {
    PyObject_HEAD

    PortableServer_ServantBase servant;

    PyORBitInterfaceInfo *info;

    PyObject *delegate;
    PyObject *this;
} PyPortableServer_Servant;

/* simple macros to go back and forth between Python object and servant */
#define PYSERVANT_TO_SERVANT(pyservant) (&(pyservant)->servant)
#define SERVANT_TO_PYSERVANT(servant)   ((PyPortableServer_Servant *)((guchar *)(servant) - offsetof(PyPortableServer_Servant, servant)))

extern PyTypeObject PyPortableServer_Servant_Type;
void _pyorbit_register_skel(ORBit_IInterface *iinterface);

extern PyTypeObject PyPortableServer_POA_Type;
extern PyTypeObject PyPortableServer_POAManager_Type;
PyObject *pyorbit_poa_new(PortableServer_POA poa);
PyObject *pyorbit_poamanager_new(PortableServer_POAManager poamanager);


extern PortableServer_POA _pyorbit_poa;

#endif
