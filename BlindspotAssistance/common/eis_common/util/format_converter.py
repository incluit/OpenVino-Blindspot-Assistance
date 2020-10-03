
"""
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

import re


def lf_to_json_converter(data):
    '''Converts line protocol data to json format
    Argument:
        data: line protocol data.
    '''
    final_data = "Measurement="
    jbuf = data.split(" ")
    tagsValue = jbuf[0].split(",")
    # To handle json if tags are given in the line protocol
    tagsValue[0] = "\"" + tagsValue[0] + "\""
    tags_value_list = [tagsValue[0]]

    # To identify the datapoints with the TAGS.
    matchString = re.match(r'([a-zA-Z0-9_]+)([,])([a-zA-Z0-9_]*)', jbuf[0])

    # To Replace the values with the quoted values.
    if matchString:
        for i in range(1, len(tagsValue)):
            tag_key_value = tagsValue[i].split("=")
            tag_value = "\"" + tag_key_value[1] + "\""
            quoted_key_value_tag = tag_key_value[0] + "=" + tag_value
            tags_value_list.append(quoted_key_value_tag)
        jbuf[0] = ",".join(tags_value_list)
        final_data += jbuf[0] + ","
    else:
        final_data += "\"" + jbuf[0] + "\"" + ","

    for i in range(2, len(jbuf)):
        jbuf[1] += " " + jbuf[i]

    influxTS = ",influx_ts=" + jbuf[len(jbuf)-1]
    jbuf[1] = jbuf[1].replace(jbuf[len(jbuf)-1], influxTS)
    final_data = final_data + jbuf[1]

    key_value_buf = final_data.split("=")
    quoted_key = "\"" + key_value_buf[0] + "\""
    final_data = final_data.replace(key_value_buf[0], quoted_key)

    # Trimming white space
    final_data = final_data.replace(" ", "")

    # Replacing the Keys field with the quoted Keys.
    for j in range(1, len(key_value_buf)-1):
        key_buf = key_value_buf[j].split(",")
        key = "," + key_buf[len(key_buf)-1] + "="
        new_key = ",\"" + key_buf[len(key_buf)-1] + "\"="
        final_data = final_data.replace(key, new_key)

    final_data = final_data.replace("=", ":")

    # Removal of "i" added by influx,from the integer value
    variable = re.findall(r'[0-9]+i', final_data)
    for intValue in variable:
        stripped_i = intValue.strip("i")
        final_data = final_data.replace(intValue, stripped_i)
    final_data = "{" + final_data + "}"
    return final_data
