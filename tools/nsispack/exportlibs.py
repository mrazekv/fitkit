# -*- coding: utf-8 -*-
# vim: set ts=4 et enc=utf-8:

'''
 ***************************************************************************
 *   Copyright (C) 2008 Brno University of Technology,                     *
 *   Faculty of Information Technology                                     *
 *   Author(s): Zdenek Vasicek   <vasicek  AT fit.vutbr.cz>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************
'''

from distutils.core import setup
import py2exe
import sys
import os

from py2exe.build_exe import py2exe

class build_installer(py2exe):
    # This class first builds the exe file(s), then creates a Windows installer.
    # You need InnoSetup for it.
    def run(self):
        # First, let py2exe do it's work.
        py2exe.run(self)

        #self.clean(os.path.abspath("xxx"))
        #print self.collect_dir
        self.clean("pythonlib")
        os.rename(self.collect_dir,"pythonlib")
        self.clean("build")
        for fn in os.listdir("dist"):
            if (not fn.endswith(".pyd")): continue
            os.rename(os.path.join("dist",fn),os.path.join("pythonlib",fn))
        self.clean("dist")

    def clean(self,topdir):
        for root, dirs, files in os.walk(topdir, topdown=False):
            for name in files:
                #print "remove fil:",os.path.join(root, name)
                os.remove(os.path.join(root, name))
            for name in dirs:
                #print "remove dri:",os.path.join(root, name)
                os.rmdir(os.path.join(root, name))
        try:
            os.rmdir(topdir)
        except:
            pass
        
opts = {
        "py2exe": {
        "ascii": 1,
        "bundle_files": 3,
#        "xref" : True,
        "excludes" : ["Tkinter","pydoc","distutils"],
        "includes" : ["codecs", "struct", "md5", 
                      "encodings.utf_16","encodings.utf_16_be","encodings.utf_16_le","encodings.utf_32","encodings.utf_32_be","encodings.utf_32_le","encodings.utf_8",
                      "encodings.latin_1","encodings.iso8859_2","encodings.cp1250","encodings.cp852","encodings.mbcs", "encodings.ascii",
#                      "docutils","docutils.core","docutils.languages.cs","docutils.readers.standalone","docutils.writers.null","docutils.writers.html4css1","docutils.parsers.rst","docutils.parsers.rst.directives.images","docutils.parsers.rst.directives.tables","docutils.parsers.rst.directives.admonitions","docutils.parsers.rst.directives.parts","docutils.parsers.rst.directives.body"
                     ],
        "packages": ['docutils']
 }

}

open("_export.py","w").write("""#!/usr/bin/python
# -*- coding: utf-8 -*-
# vim: set ts=4 et enc=utf-8:

import site
import os.path
import codecs
import urllib
import socket
import urllib2
import xml.dom.minidom
#import docutils
#from docutils.writers import html4css1

print u"test string čeština"
""")

if (len(sys.argv) == 1):
    sys.argv.append("py2exe")


setup(
    console = [{"script": "_export.py"}], 
    options=opts, 
    cmdclass = {"py2exe": build_installer},
)


from docutils.writers import html4css1
pth = html4css1.Writer.default_stylesheet_path
open('pythonlib/docutils/writers/html4css1/'+os.path.basename(pth),'wb').write( open(pth,'rb').read())

pth = html4css1.Writer.default_template_path
open('pythonlib/docutils/writers/html4css1/'+os.path.basename(pth),'wb').write( open(pth,'rb').read())

os.remove("_export.py")
