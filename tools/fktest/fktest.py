#!/usr/bin/env python
# -*- coding: UTF-8 -*-
# vim: set ts=4 et:
import sys, os, re

#sys.argv.append('D:\FITkit\svn\\apps\inp\segmentace\\build\\fpga\med_filtr.par.par')
#sys.argv.append("""WARNING.*design did not meet timing""")

if len(sys.argv) != 3:
    print >>sys.stderr, "Usage:"
    print >>sys.stderr,"  fktest file_name regexp"
    sys.exit(0)

else:
    fname, regexp = sys.argv[1:3]
    if not os.path.isfile(fname):
       print >>sys.stderr,"File '%s' not found" % fname
       sys.exit(0)
    
    try:
        m = re.search(regexp, open(fname,'rb').read())
        if m:
            print '='*len(m.group(0))
            print m.group(0)
            print '='*len(m.group(0))
            sys.exit(1)

        sys.exit(0)

    except Exception, e:
       print >>sys.stderr,"Invalid arguments"
       sys.exit(0)      
