#include <Python.h>
#include <windows.h>

#define READ_TIMEOUT 500

HANDLE hComm;
const char *port;
const int *baudrate;
DCB params = {0};
char lpBuf[2];

static PyObject *openPortError;
static PyObject *statePortError;
static PyObject *readPortError;

void openPort(const char *port) {
    hComm = CreateFileA(strcat("\\\\.\\", port),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);
}

static PyObject *openSerial(PyObject *self, PyObject *args) {
    if (!PyArg_ParseTuple(args, "si", &port, &baudrate)) {
        return NULL;
    }

    openPort(port);

    if (hComm == INVALID_HANDLE_VALUE) {
        PyErr_SetString(openPortError, "Impossible to open the port (not connected ?)");
        return Py_None;
    } else {
        params.DCBlength = sizeof(DCB);
        params.BaudRate = baudrate;
        params.ByteSize = 8;
        params.Parity = NOPARITY;
        params.StopBits = ONESTOPBIT;

        BOOL setState = SetCommState(hComm, &params);
        if (!setState) {
            PyErr_SetString(statePortError, "Impossible to set the port state (check the wiring)");
            return Py_None;
        }
    }

    return Py_None;
}

void HandleASuccessfulRead(char lpBuf[], DWORD dwRead) {
      byte b;

      for(int i = 0; i < (int)dwRead; i++) {
            b = lpBuf[i];
            printf("%c", b);
      }
}

static PyObject *getSettings(PyObject *self) {
    PyObject *paramsList;
    paramsList = Py_BuildValue("[sii]", port, params.BaudRate, params.ByteSize);
    return paramsList;
}

static PyObject *readSerial(PyObject *self) {
    DWORD dwRead;
    DWORD dwRes;
    BOOL fWaitingOnRead = FALSE;
    OVERLAPPED osReader = {0};

    osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (osReader.hEvent == NULL)
        printf("error3");

    if (!fWaitingOnRead) {
        if (!ReadFile(hComm, lpBuf, sizeof(lpBuf), &dwRead, &osReader)) {
            if (GetLastError() != ERROR_IO_PENDING) {
                PyErr_SetString(readPortError, "Impossible to read the port (check the wiring)");
                return Py_None;
            } else {
                printf("error2");
                fWaitingOnRead = TRUE;
            }  
        } else {    
            HandleASuccessfulRead(lpBuf, dwRead);
        }
    }

    if (fWaitingOnRead) {
        dwRes = WaitForSingleObject(osReader.hEvent, READ_TIMEOUT);
        switch(dwRes) {
            case WAIT_OBJECT_0:
                if (!GetOverlappedResult(hComm, &osReader, &dwRead, FALSE))
                    printf("Error");
                else
                    HandleASuccessfulRead(lpBuf, dwRead);
                fWaitingOnRead = FALSE;
                break;

            case WAIT_TIMEOUT:
                break;                       

            default:
                break;
   }
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
    PyObject *module;
    module = PyModule_Create(&serialiomodule);

    openPortError = PyErr_NewException("SerialIO.OpenPortError", NULL, NULL);
    Py_INCREF(openPortError);
    statePortError = PyErr_NewException("SerialIO.StatePortError", NULL, NULL);
    Py_INCREF(openPortError);
    readPortError = PyErr_NewException("SerialIO.ReadPortError", NULL, NULL);
    Py_INCREF(openPortError);

    PyModule_AddObject(module, "OpenPortError", "OpenPortError");
    PyModule_AddObject(module, "StatePortError", "StatePortError");
    PyModule_AddObject(module, "ReadPortError", "ReadPortError");

    return module;
}