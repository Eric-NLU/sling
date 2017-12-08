// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SLING_PYAPI_PYFRAME_H_
#define SLING_PYAPI_PYFRAME_H_

#include "sling/pyapi/pybase.h"
#include "sling/pyapi/pystore.h"
#include "sling/frame/store.h"

namespace sling {

// Python wrapper for frame.
struct PyFrame : public PyBase, public Root {
  // Initialize frame wrapper.
  void Init(PyStore *pystore, Handle handle);

  // Deallocate frame wrapper.
  void Dealloc();

  // Return the number of slots in the frame.
  Py_ssize_t Size();

  // Look up role value for frame.
  PyObject *Lookup(PyObject *key);

  // Assign value to slot.
  int Assign(PyObject *key, PyObject *v);

  // Check if frame has role.
  int Contains(PyObject *key);

  // Get role value for frame wrapper.
  PyObject *GetAttr(PyObject *key);

  // Set role value for frame.
  int SetAttr(PyObject *key, PyObject *v);

  // Append slot to frame.
  PyObject *Append(PyObject *args);

  // Extend frame with list of slots.
  PyObject *Extend(PyObject *arg);

  // Return iterator for all slots in frame.
  PyObject *Slots();

  // Return iterator for finding all slots with a specific name.
  PyObject *Find(PyObject *args, PyObject *kw);

  // Return handle as hash value for frame.
  long Hash();

  // Check id frame is the same as another object.
  PyObject *Compare(PyObject *other, int op);

  // Return store for frame.
  PyObject *GetStore();

  // Return frame as string.
  PyObject *Str();

  // Return frame in ascii or binary encoding.
  PyObject *Data(PyObject *args, PyObject *kw);

  // Check if frame can be modified.
  bool Writable();

  // Return handle for frame.
  Handle handle() const { return handle_; }

  // Dereference frame reference.
  FrameDatum *frame() { return pystore->store->Deref(handle())->AsFrame(); }

  // Store for frame.
  PyStore *pystore;

  // Registration.
  static PyTypeObject type;
  static PyMappingMethods mapping;
  static PySequenceMethods sequence;
  static PyMethodDef methods[];
  static void Define(PyObject *module);
};

// Python wrapper for frame slot iterator.
struct PySlots : public PyBase {
  // Initialize frame slot iterator.
  void Init(PyFrame *pyframe, Handle role);

  // Deallocate frame slot iterator.
  void Dealloc();

  // Return next (matching) slot.
  PyObject *Next();

  // Return self.
  PyObject *Self();

  // Frame that is being iterated.
  PyFrame *pyframe;

  // Current slot index.
  int current;

  // Slot role name or nil if all slots should be iterated.
  Handle role;

  // Registration.
  static PyTypeObject type;
  static void Define(PyObject *module);
};

}  // namespace sling

#endif  // SLING_PYAPI_PYFRAME_H_
