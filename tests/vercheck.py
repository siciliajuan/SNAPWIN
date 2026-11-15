#!/usr/bin/env python3
"""
VERCHECK.PY - Checks the VERSIONINFO resource of Win16 binaries.

Walks the resource the way Windows' VER.DLL does (by the declared block
lengths) and prints every string value. Fails if:
  - a value is not NUL-terminated within its declared length, or an
    expected field is missing (Open Watcom's wrc bug: see src/version.rc);
  - a field does not match src/version.h (some texts are copied into
    version.rc to work around the bug, so they can drift).

Usage: python3 tests/vercheck.py dist/*.exe dist/*.dll dist/*.cpl

Copyright (c) 2026 Juan Sicilia
SPDX-License-Identifier: MIT
"""
import os, re, struct, sys

EXPECTED = ['CompanyName', 'FileDescription', 'FileVersion', 'InternalName',
            'LegalCopyright', 'OriginalFilename', 'ProductName',
            'ProductVersion', 'Comments']

def version_h():
    """String macros of src/version.h"""
    path = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'src', 'version.h')
    return dict(re.findall(r'#define\s+(SNAPWIN_\w+)\s+"([^"]*)"', open(path).read()))

def ne_resource(data, rtype=16, rid=1):
    """Return the bytes of resource (rtype, rid) from an NE executable."""
    ne = struct.unpack_from('<I', data, 0x3C)[0]
    assert data[ne:ne+2] == b'NE', 'not an NE file'
    rt = ne + struct.unpack_from('<H', data, ne + 0x24)[0]
    shift = struct.unpack_from('<H', data, rt)[0]
    p = rt + 2
    while True:
        tid, count = struct.unpack_from('<HH', data, p)
        if tid == 0:
            break
        p += 8
        for _ in range(count):
            off, length, flags, nid = struct.unpack_from('<HHHH', data, p)
            p += 12
            if tid == (0x8000 | rtype) and nid == (0x8000 | rid):
                return data[off << shift:(off << shift) + (length << shift)]
    raise KeyError('no VERSIONINFO resource')

def align4(n):
    return (n + 3) & ~3

def walk(block, start, end, depth, out):
    """Win16 version block: WORD wLength, WORD wValueLength, szKey, value, children."""
    p = start
    end = min(end, len(block))
    while p + 4 <= end:
        wlen, vlen = struct.unpack_from('<HH', block, p)
        if wlen == 0:
            break
        k_end = block.index(b'\0', p + 4)
        key = block[p + 4:k_end].decode('latin-1')
        v_start = align4(k_end + 1)
        value = block[v_start:v_start + vlen]
        out.append((depth, key, value))
        c_start = align4(v_start + vlen)
        if key in ('VS_VERSION_INFO', 'StringFileInfo') or (depth == 2 and len(key) == 8):
            walk(block, c_start, p + wlen, depth + 1, out)
        p = align4(p + wlen)

def main(paths):
    v = version_h()
    bad = 0
    for path in paths:
        res = ne_resource(open(path, 'rb').read())
        items = []
        walk(res, 0, len(res), 0, items)
        print(path)
        fields = {}
        for depth, key, value in items:
            if depth == 3:                        # string values
                ok = value.endswith(b'\0')
                text = value.split(b'\0')[0].decode('cp1252')
                fields[key] = text
                print('  %-17s %-4s %r' % (key, 'ok' if ok else 'BAD', text))
                bad += not ok
        problems = ['missing field %s' % k for k in EXPECTED if k not in fields]
        name = os.path.basename(path).upper()
        checks = [
            ('CompanyName',      fields.get('CompanyName') == v['SNAPWIN_AUTHOR']),
            ('FileVersion',      fields.get('FileVersion') == v['SNAPWIN_VERSION']),
            ('ProductVersion',   fields.get('ProductVersion') == v['SNAPWIN_VERSION']),
            ('LegalCopyright',   fields.get('LegalCopyright', '').startswith(v['SNAPWIN_COPYRIGHT'])),
            ('Comments',         fields.get('Comments', '').endswith(v['SNAPWIN_URL'])),
            ('InternalName',     fields.get('InternalName') == name),
            ('OriginalFilename', fields.get('OriginalFilename') == name),
        ]
        problems += ['%s does not match version.h / file name' % k for k, ok in checks if not ok]
        for msg in problems:
            print('  PROBLEM: ' + msg)
        bad += len(problems)
    print('PASS' if not bad else 'FAIL: %d problem(s)' % bad)
    return 1 if bad else 0

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
