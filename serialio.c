/*
 * Python support for serial I/O
 *
 * This module was originally created by novus-alex,
 * https://github.com/novus-alex
 * https://github.com/novus-alex/SerialIO
 *
 * Copyright (c) 2021 by Alexandre Hachet
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <Python.h>
#include <windows.h>

#define READ_TIMEOUT 500
DCB params = {0};

static PyObject *openPortError;
static PyObject *statePortError;
static PyObject *readPortError;

HANDLE openPort(const char *port) {
    HANDLE hComm;

    hComm = CreateFileA(strcat("\\\\.\\", port),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    return hComm;
}

static PyObject *Serial(PyObject *self, PyObject *args) {
    /*
     * Main func, need multiples args: [PORT, BAUDRATE]
     * Openning the ports with the windows.h api
     * While opened, the ports are declared through HANDLE
     *
     */

    const char *port;
    const int *baudrate;

    if (!PyArg_ParseTuple(args, "si", &port, &baudrate)) {
        return NULL;
    }

    HANDLE com_port = openPort(port);

    if (com_port == INVALID_HANDLE_VALUE) {
        PyErr_SetString(openPortError, "Impossible to open the port (not connected ?)");
        return NULL;
    } else {
        params.DCBlength = sizeof(DCB);
        params.BaudRate = baudrate;
        params.ByteSize = 8;
        params.Parity = NOPARITY;
        params.StopBits = ONESTOPBIT;

        COMMTIMEOUTS timeouts = { 0 };
        timeouts.ReadIntervalTimeout = 50;
        timeouts.ReadTotalTimeoutConstant = 50;
        timeouts.ReadTotalTimeoutMultiplier = 10;
        timeouts.WriteTotalTimeoutConstant = 50;
        timeouts.WriteTotalTimeoutMultiplier = 10;

        if (SetCommTimeouts(com_port, &timeouts) == FALSE)
            return NULL;

        BOOL setState = SetCommState(com_port, &params);
        if (!setState) {
            PyErr_SetString(statePortError, "Impossible to set the port state (check the wiring)");
            return NULL;
        }
    }

    return PyLong_FromVoidPtr(com_port);
}

static PyObject *readPort(PyObject *self, PyObject *args) {
    /*
     * The read func is built on WaitingEvent func so it's waiting for an incoming byte(s)
     * Buffer is 64 char size
     * The func return a string var with the containing data read
     *
     */

    DWORD dwEventMask;
    DWORD NoBytesRead;
    char ReadData;
    unsigned char loop = 0;
    char SerialBuffer[64] = { 0 };
    BOOL Status;
    PyObject *python_handle;
    HANDLE com_port;

    if (!PyArg_ParseTuple(args, "O", &python_handle)) {
        return NULL;
    }
    com_port = PyLong_AsVoidPtr(python_handle);

    Status = SetCommMask(com_port, EV_RXCHAR);
    Status = WaitCommEvent(com_port, &dwEventMask, NULL);
    if (Status == FALSE) {
        return Py_None;
    }

    do {
        Status = ReadFile(com_port, &ReadData, sizeof(ReadData), &NoBytesRead, NULL);
        SerialBuffer[loop] = ReadData;
        ++loop;
    }

    while (NoBytesRead > 0);
    --loop;

    return Py_BuildValue("s", SerialBuffer);
}

static PyObject *closePort(PyObject *self, PyObject *args) {
    /*
     * Simple close port func, only working with opened ports
     *
     */

    PyObject *python_handle;
    HANDLE com_port;

    if (!PyArg_ParseTuple(args, "O", &python_handle)) {
        return NULL;
    }
    com_port = PyLong_AsVoidPtr(python_handle);

    CloseHandle(com_port);
    return Py_None;
}

static PyMethodDef SerialIOMethods[] = {
    {"Serial", Serial, METH_VARARGS, "Open a serial port in python"},
    {"read", readPort, METH_VARARGS, "Read the serial port"},
    {"close", closePort, METH_VARARGS, "Close the serial port"},
    {NULL, NULL, 0, NULL}
};


static struct PyModuleDef serialiomodule = {
    PyModuleDef_HEAD_INIT,
    "serialio",
    "Python interface for serial communications",
    -1,
    SerialIOMethods
};

PyMODINIT_FUNC PyInit_serialio(void) {
    PyObject *module;
    module = PyModule_Create(&serialiomodule);

    openPortError = PyErr_NewException("SerialIO.OpenPortError", NULL, NULL);
    Py_INCREF(openPortError);
    statePortError = PyErr_NewException("SerialIO.StatePortError", NULL, NULL);
    Py_INCREF(openPortError);
    readPortError = PyErr_NewException("SerialIO.ReadPortError", NULL, NULL);
    Py_INCREF(openPortError);

    PyModule_AddObject(module, "OpenPortError", openPortError);
    PyModule_AddObject(module, "StatePortError", statePortError);
    PyModule_AddObject(module, "ReadPortError", readPortError);

    return module;
}