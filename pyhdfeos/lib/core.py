import sys


def decode_comma_delimited_ffi_string(strbuf):
    if sys.hexversion < 0x03000000:
        lst = strbuf.split(',')
    else:
        lst = strbuf.decode('ascii').split(',')
    return lst
