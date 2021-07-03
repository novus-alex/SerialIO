'''
This is a simple script to test the module
'''

# Import the module
import serialio

# Infos
__author__ = "alex"
__version__ = "1.0"
__date__ = "07/01/2021"

# Open the serial port
serialio.openSerial("COM3", 9600)

# Read the port
serialio.read()

# Get the serial parameters
params = serialio.getSettings()
print(params)