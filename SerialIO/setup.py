'''
Please use this file to build the module. To complete it, open a cmd window inside the dev folder
and then run the command: setup.py install
This will automatically build and install the module

First publication on 01/07/2021
'''

from setuptools import setup, Extension

test_module = Extension('serialio',
	sources=['serialio.c'])

setup(
	name="serialio",
	version="1.0",
	description="Serial module for Python",
	author="alex",
	ext_modules=[test_module])