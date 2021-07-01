#include <Python.h>
#include <windows.h>

HANDLE hComm;
const char *port;
const int *baudrate;
DCB params = {0};

static PyObject *openSerial(PyObject *self, PyObject *args) {
    if (!PyArg_ParseTuple(args, "si", &port, &baudrate)) {
        return NULL;
    }

    hComm = CreateFileA(strcat("\\\\.\\", port),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (hComm == INVALID_HANDLE_VALUE) {
        printf("Error while openning the port\n");
    } else {
        params.DCBlength = sizeof(DCB);
        params.BaudRate = baudrate;
        params.ByteSize = 8;
        params.Parity = NOPARITY;
        params.StopBits = ONESTOPBIT;

        BOOL setState = SetCommState(hComm, &params);
        if (!setState) {
            printf("Error while setting the serial parameters");
        }
    }

    return Py_None;
}

static PyObject *getSettings(PyObject *self) {
    PyObject *paramsList;
    paramsList = Py_BuildValue("[sii]", port, params.BaudRate, params.ByteSize);
    return paramsList;
}

static PyObject *readSerial(PyObject *self) {
    DWORD dwRead;
    BOOL fWaitingOnRead = FALSE;
    OVERLAPPED osReader = {0};
    char a = "‏‏‎ ‎";
    int i = 0;
    char lpBuf[256];

    BOOL serialReturn = ReadFile(hComm, lpBuf, sizeof(lpBuf), &dwRead, &osReader);

    if (!serialReturn) {
        printf("Error while reading the port\n");
    } else {
        printf("%s", lpBuf);

        while (strcmp(&lpBuf[i], "\n") != 0) {
            i = i + 1;
        }

        for (int c = 0; lpBuf[c] != lpBuf[i]; c++) {
            strcpy(&lpBuf[c], &lpBuf[c+i]);
        }

        printf("%s", lpBuf);
    }

    return Py_None;
}

static PyMethodDef SerialIOMethods[] = {
    {"openSerial", openSerial, METH_VARARGS, "Open a serial port in python"},
    {"read", readSerial, METH_NOARGS, "Read the serial port"},
    {"getSettings", getSettings, METH_NOARGS, "Get the parameters of the port"},
    {NULL, NULL, 0, NULL}
};


static struct PyModuleDef serialiomodule = {
    PyModuleDef_HEAD_INIT,
    "serialio",
    "Python interface for the fputs C library function",
    -1,
    SerialIOMethods
};

PyMODINIT_FUNC PyInit_serialio(void) {
    return PyModule_Create(&serialiomodule);
}