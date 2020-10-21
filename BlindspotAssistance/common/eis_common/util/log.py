# Copyright (c) 2018 Intel Corporation.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import os
import logging
import logging.handlers
import sys

LOG_LEVELS = {
    'DEBUG': logging.DEBUG,
    'INFO': logging.INFO,
    'ERROR': logging.ERROR,
    'WARN': logging.WARN
}


def configure_logging(log_level, module_name, dev_mode):
    """Configure logging to log to stdout.

    The log string will be formatted as follows:
        '%(asctime)s : %(levelname)s : %(name)s : [%(filename)s] :
            '%(funcName)s : in line : [%(lineno)d] : %(message)s'

    This function should only ever be called once in a the Python runtime.

    :param log_level: Logging level to use, must be one of the following:
           DEBUG, INFO, ERROR or WARN
    :type log_level: str
    :param module_name: Module running logging
    :type module_name: str
    :raises Exception: If the given log level is unknown, or if max_bytes
                       and file_count are both 0, or if the log file directory
                       does not exist.
    :return: Logging object
    :rtype: Object
    """
    if log_level not in LOG_LEVELS:
        raise Exception('Unknown log level: {}'.format(log_level))

    if dev_mode == True:
        fmt_str = ('%(asctime)s : %(levelname)s  : {} : %(name)s : [%(filename)s] :' .format("Insecure Mode")+
               '%(funcName)s : in line : [%(lineno)d] : %(message)s')
    else:
        fmt_str = ('%(asctime)s : %(levelname)s : %(name)s : [%(filename)s] :' +
               '%(funcName)s : in line : [%(lineno)d] : %(message)s')

    log_lvl = LOG_LEVELS[log_level]
    logging.basicConfig(format=fmt_str, level=log_lvl)
    logger = logging.getLogger(module_name)
    logger.setLevel(log_lvl)

    # Do basic configuration of logging (just for stdout config)
    logging.basicConfig(format=fmt_str, level=log_lvl)
    
    logger = logging.getLogger()
    logger.setLevel(log_lvl)
    handler = logging.StreamHandler(sys.stdout)
    formatter = logging.Formatter(fmt_str)
    handler.setFormatter(formatter)

    # Removing the default handler added by getLogger to avoid duplicate logs
    if(logger.hasHandlers()):
        logger.handlers.clear()
    logger.addHandler(handler)

    return logger
    
