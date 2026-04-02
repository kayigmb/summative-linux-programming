from setuptools import Extension, setup

module = Extension("vibration", sources=["q3.c"])

setup(name="vibration", version="1.0", ext_modules=[module])
