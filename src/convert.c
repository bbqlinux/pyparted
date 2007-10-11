/*
 * convert.c
 * Convert _ped Python types to libparted typedefs.  Only typedef structs
 * need to be converted.  When a typedef in libparted is a primitive type,
 * we can just use it directly.
 *
 * Copyright (C) 2007  Red Hat, Inc.
 * All rights reserved.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions of
 * the GNU General Public License v.2, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY expressed or implied, including the implied warranties of
 * MERCHANTABILITY or FITNESS FOR A * PARTICULAR PURPOSE.  See the GNU General
 * Public License for more details.  You should have received a copy of the
 * GNU General Public License along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.  Any Red Hat trademarks that are incorporated in the
 * source code or documentation are not subject to the GNU General Public
 * License and may only be used or replicated with the express permission of
 * Red Hat, Inc.
 *
 * Red Hat Author(s): David Cantrell
 */

#include <Python.h>

#include "convert.h"
#include "pyconstraint.h"
#include "pydevice.h"
#include "pygeom.h"
#include "pynatmath.h"
#include "pyunit.h"

/* Note... all of these functions malloc for memory to create the returning
 * type.  It is up to the caller to free the memory after using it.
 */

/* _ped_Alignment -> PedAlignment functions */
PedAlignment *_ped_Alignment2PedAlignment(PyObject *s) {
    PedAlignment *ret;
    PedSector offset, grain_size;
    _ped_Alignment *alignment = (_ped_Alignment *) s;

    if (alignment == NULL) {
        PyErr_SetString(PyExc_TypeError, "Empty _ped.Alignment()");
        return NULL;
    }

    offset = _ped_Sector2PedSector(alignment->offset);
    if (offset == -1) {
        return NULL;
    }

    grain_size = _ped_Sector2PedSector(alignment->grain_size);
    if (grain_size == -1) {
        return NULL;
    }

    ret = ped_alignment_new(offset, grain_size);
    if (ret == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory");
        return NULL;
    }

    return ret;
}

/* _ped_Constraint -> PedConstraint functions */
PedConstraint *_ped_Constraint2PedConstraint(PyObject *s) {
    PedConstraint *ret;
    PedAlignment *start_align, *end_align;
    PedGeometry *start_range, *end_range;
    PedSector min_size, max_size;
    _ped_Constraint *constraint = (_ped_Constraint *) s;

    if (constraint == NULL) {
        PyErr_SetString(PyExc_TypeError, "Empty _ped.Constraint()");
        return NULL;
    }

    start_align = _ped_Alignment2PedAlignment(constraint->start_align);
    if (start_align == NULL) {
        return NULL;
    }

    end_align = _ped_Alignment2PedAlignment(constraint->end_align);
    if (end_align == NULL) {
        return NULL;
    }

    start_range = _ped_Geometry2PedGeometry(constraint->start_range);
    if (start_range == NULL) {
        return NULL;
    }

    end_range = _ped_Geometry2PedGeometry(constraint->end_range);
    if (end_range == NULL) {
        return NULL;
    }

    min_size = _ped_Sector2PedSector(constraint->min_size);
    if (min_size == -1) {
        return NULL;
    }

    max_size = _ped_Sector2PedSector(constraint->max_size);
    if (max_size == -1) {
        return NULL;
    }

    ret = ped_constraint_new(start_align, end_align, start_range, end_range,
                             min_size, max_size);
    if (ret == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory");
        return NULL;
    }

    ped_alignment_destroy(start_align);
    ped_alignment_destroy(end_align);
    ped_geometry_destroy(start_range);
    ped_geometry_destroy(end_range);

    return ret;
}

_ped_Constraint *PedConstraint2_ped_Constraint(PedConstraint *constraint) {
    /* FIXME */
    return NULL;
}

/* _ped_Device -> PedDevice functions */
PedDevice *_ped_Device2PedDevice(PyObject *s) {
    PedDevice *ret;
    _ped_Device *dev = (_ped_Device *) s;

    if (dev == NULL) {
        PyErr_SetString(PyExc_TypeError, "Empty _ped.Device()");
        return NULL;
    }

    if (dev->path) {
        ret = ped_device_get(dev->path);
        if (ret == NULL) {
            PyErr_SetString(PyExc_MemoryError, "Out of memory");
            return NULL;
        }
    } else {
        return NULL;
    }

    return ret;
}

/* _ped_Geometry -> PedGeometry functions */
PedGeometry *_ped_Geometry2PedGeometry(PyObject *s) {
    PedGeometry *ret;
    PedDevice *dev;
    PedSector start, length;
    _ped_Geometry *geometry = (_ped_Geometry *) s;

    if (geometry == NULL) {
        PyErr_SetString(PyExc_TypeError, "Empty _ped.Geometry()");
        return NULL;
    }

    dev = _ped_Device2PedDevice(geometry->dev);
    if (dev == NULL) {
        return NULL;
    }

    start = _ped_Sector2PedSector(geometry->start);
    if (start == -1) {
        return NULL;
    }

    length = _ped_Sector2PedSector(geometry->length);
    if (length == -1) {
        return NULL;
    }

    ret = ped_geometry_new(dev, start, length);
    if (ret == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory");
        return NULL;
    }

    /* do not ped_destroy_device(dev) here because ret contains a ptr to it */

    return ret;
}

/* _ped_CHSGeometry -> PedCHSGeometry functions */
PedCHSGeometry *_ped_CHSGeometry2PedCHSGeometry(PyObject *s) {
    PedCHSGeometry *ret;
    _ped_CHSGeometry *srcgeom = (_ped_CHSGeometry *) s;

    if (srcgeom == NULL) {
        PyErr_SetString(PyExc_TypeError, "Empty _ped.CHSGeometry()");
        return;
    }

    ret = malloc(sizeof(PedCHSGeometry));
    if (ret == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory");
        return NULL;
    }

    ret->cylinders = srcgeom->cylinders;
    ret->heads = srcgeom->heads;
    ret->sectors = srcgeom->sectors;

    return ret;
}

/* _ped_Sector -> PedSector functions */
PedSector _ped_Sector2PedSector(PyObject *s) {
    PedSector ret;
    _ped_Sector *sector = (_ped_Sector *) s;

    if (sector == NULL) {
        PyErr_SetString(PyExc_TypeError, "Empty _ped.Sector()");
        return -1;
    }

    ret = sector->val;
    return ret;
}

/* _ped_Unit -> PedUnit functions */
PedUnit _ped_Unit2PedUnit(PyObject *s) {
    PedUnit ret;
    _ped_Unit *unit = (_ped_Unit *) s;

    if (unit == NULL) {
        PyErr_SetString(PyExc_TypeError, "Empty _ped_Unit()");
        return -1;
    }

    ret = unit->val;
    return ret;
}