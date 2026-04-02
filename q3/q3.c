#include <Python.h>
#include <math.h>
#include <stdlib.h>

static int parse_input(PyObject *obj, double **data, Py_ssize_t *len) {
  if (!PyList_Check(obj) && !PyTuple_Check(obj)) {
    PyErr_SetString(PyExc_TypeError, "Expected list or tuple");
    return 0;
  }

  *len = PySequence_Size(obj);
  if (*len == 0) {
    *data = NULL;
    return 1;
  }

  *data = (double *)malloc(sizeof(double) * (*len));
  if (!*data) {
    PyErr_SetString(PyExc_MemoryError, "Failed to allocate memory");
    return 0;
  }

  for (Py_ssize_t i = 0; i < *len; i++) {
    PyObject *item = PySequence_GetItem(obj, i); // New reference
    if (!PyFloat_Check(item) && !PyLong_Check(item)) {
      PyErr_SetString(PyExc_TypeError, "All elements must be numbers");
      free(*data);
      Py_XDECREF(item);
      return 0;
    }
    (*data)[i] = PyFloat_AsDouble(item);
    Py_XDECREF(item);
  }
  return 1;
}

// 1. RMS
static PyObject *rms(PyObject *self, PyObject *args) {
  PyObject *input;
  if (!PyArg_ParseTuple(args, "O", &input))
    return NULL;

  double *data;
  Py_ssize_t n;
  if (!parse_input(input, &data, &n))
    return NULL;

  if (n == 0) {
    if (data)
      free(data);
    return PyFloat_FromDouble(0.0);
  }

  double sum_sq = 0;
  for (Py_ssize_t i = 0; i < n; i++)
    sum_sq += data[i] * data[i];

  double result = sqrt(sum_sq / n);
  free(data);
  return PyFloat_FromDouble(result);
}

// 2. Standard deviation
static PyObject *std_dev(PyObject *self, PyObject *args) {
  PyObject *input;
  if (!PyArg_ParseTuple(args, "O", &input))
    return NULL;

  double *data;
  Py_ssize_t n;
  if (!parse_input(input, &data, &n))
    return NULL;

  if (n == 0) {
    if (data)
      free(data);
    return PyFloat_FromDouble(0.0);
  }

  double sum = 0;
  for (Py_ssize_t i = 0; i < n; i++)
    sum += data[i];
  double mean = sum / n;

  double sum_sq = 0;
  for (Py_ssize_t i = 0; i < n; i++)
    sum_sq += (data[i] - mean) * (data[i] - mean);

  double result = sqrt(sum_sq / n);
  free(data);
  return PyFloat_FromDouble(result);
}

// 3. Peak-to-peak
static PyObject *peak_to_peak(PyObject *self, PyObject *args) {
  PyObject *input;
  if (!PyArg_ParseTuple(args, "O", &input))
    return NULL;

  double *data;
  Py_ssize_t n;
  if (!parse_input(input, &data, &n))
    return NULL;

  if (n == 0) {
    if (data)
      free(data);
    return PyFloat_FromDouble(0.0);
  }

  double min = data[0];
  double max = data[0];
  for (Py_ssize_t i = 1; i < n; i++) {
    if (data[i] < min)
      min = data[i];
    if (data[i] > max)
      max = data[i];
  }

  free(data);
  return PyFloat_FromDouble(max - min);
}

// 4. Count above threshold
static PyObject *above_threshold(PyObject *self, PyObject *args) {
  PyObject *input;
  double threshold;
  if (!PyArg_ParseTuple(args, "Od", &input, &threshold))
    return NULL;

  double *data;
  Py_ssize_t n;
  if (!parse_input(input, &data, &n))
    return NULL;

  int count = 0;
  for (Py_ssize_t i = 0; i < n; i++)
    if (data[i] > threshold)
      count++;

  free(data);
  return PyLong_FromLong(count);
}

// 5. Summary
static PyObject *summary(PyObject *self, PyObject *args) {
  PyObject *input;
  if (!PyArg_ParseTuple(args, "O", &input))
    return NULL;

  double *data;
  Py_ssize_t n;
  if (!parse_input(input, &data, &n))
    return NULL;

  PyObject *dict = PyDict_New();
  if (!dict) {
    if (data)
      free(data);
    return NULL;
  }

  double sum = 0, min = 0, max = 0;
  if (n > 0) {
    min = max = data[0];
    for (Py_ssize_t i = 0; i < n; i++) {
      sum += data[i];
      if (data[i] < min)
        min = data[i];
      if (data[i] > max)
        max = data[i];
    }
  }

  PyDict_SetItemString(dict, "count", PyLong_FromSsize_t(n));
  PyDict_SetItemString(dict, "sum", PyFloat_FromDouble(sum));
  PyDict_SetItemString(dict, "min", PyFloat_FromDouble(n > 0 ? min : 0.0));
  PyDict_SetItemString(dict, "max", PyFloat_FromDouble(n > 0 ? max : 0.0));
  PyDict_SetItemString(dict, "rms", rms(self, args));
  PyDict_SetItemString(dict, "std_dev", std_dev(self, args));

  free(data);
  return dict;
}

static PyMethodDef VibrationMethods[] = {
    {"peak_to_peak", peak_to_peak, METH_VARARGS, "Compute peak-to-peak value"},
    {"rms", rms, METH_VARARGS, "Compute RMS value"},
    {"std_dev", std_dev, METH_VARARGS, "Compute standard deviation"},
    {"above_threshold", above_threshold, METH_VARARGS,
     "Count numbers above threshold"},
    {"summary", summary, METH_VARARGS, "Return summary dictionary"},
    {NULL, NULL, 0, NULL} // Sentinel
};

static struct PyModuleDef vibrationmodule = {
    PyModuleDef_HEAD_INIT, "vibration", "C extension for vibration analysis",
    -1, VibrationMethods};

PyMODINIT_FUNC PyInit_vibration(void) {
  return PyModule_Create(&vibrationmodule);
}
