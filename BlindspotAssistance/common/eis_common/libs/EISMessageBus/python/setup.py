# Copyright (c) 2019 Intel Corporation.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
"""Python distutils installer for the Python EIS Message Bus library
"""

import os
from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize


def read(fname):
    """Read long description
    """
    return open(os.path.join(os.path.dirname(__file__), fname)).read()


# Main package setup
setup(
    name='eis-msgbus',
    version='${PACKAGE_VERSION}',
    author='Kevin Midkiff',
    author_email='kevin.midkiff@intel.com',
    description='EIS message bus Python wrapper',
    keywords='msgbus eis zeromq',
    url='',
    long_description=read('../README.md'),
    classifiers=[
        'Development Status :: 5 - Production/Stable',
        'Operating System :: POSIX',
        'Intended Audience :: Developers',
        'Topic :: System :: Networking',
    ],
    package_dir={'': '.'},
    packages=['eis'],
    ext_modules = cythonize([
            Extension(
                '*',
                ['./eis/*.pyx'],
                include_dirs=['${PROJECT_SOURCE_DIR}/include'],
                library_dirs=['${PROJECT_BINARY_DIR}/'],
                libraries=['eismsgbus', 'eisutils', 'eismsgenv'])
        ],
        build_dir='./build/cython',
        compiler_directives={'language_level': 3}
    )
)
