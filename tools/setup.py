from distutils.core import setup
from Cython.Build import cythonize

setup(
    name = "voxie tool modules",
    ext_modules = cythonize('mesher.pyx'),
)