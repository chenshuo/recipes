#!/usr/bin/env python3

import collections, os, sys

SOURCE_EXTS = set([".cc", ".c", ".h", ".S"])

class CodeSize:
    def __init__(self):
        self.self_bytes = 0
        self.self_lines = 0
        self.self_files = 0
        self.all_bytes = 0
        self.all_lines = 0
        self.all_files = 0

    def __repr__(self):
        if self.is_file():
            return "%d" % self.self_lines
        if self.self_lines == self.all_lines:
            return "%d %d" % (self.all_files, self.all_lines)
        return "%d %d %d" % (self.all_files, self.all_lines, self.self_lines)

    def is_file(self):
        return self.all_files == 0

    def is_dir(self):
        return self.all_files > 0

sizes = collections.defaultdict(CodeSize)

def accumulate(fullname, content):
    parent = ''
    lines = content.count('\n')
    sizes[parent].all_lines += lines
    sizes[parent].all_files += 1
    # a/b/c -> a/b, c
    for part in os.path.split(fullname):
        parent = os.path.join(parent, part)
        sizes[parent].self_bytes += len(content)
        sizes[parent].self_lines += lines
        sizes[parent].self_files += 1
    # a/b/c -> a, b, c
    parent = ''
    for part in fullname.split(os.path.sep)[:-1]:
        parent = os.path.join(parent, part)
        sizes[parent].all_bytes += len(content)
        sizes[parent].all_lines += lines
        sizes[parent].all_files += 1

def walk(path):
    for root, dirs, files in os.walk(path):
        # skip hidden directories
        dirs[:] = [x for x in dirs if not x.startswith('.')]

        for filename in files:
            if os.path.splitext(filename)[1] in SOURCE_EXTS:
                fullname = os.path.join(root, filename)
                with open(fullname) as f:
                    try:
                        content = f.read()
                        accumulate(fullname, content)
                    except:
                        print("ERROR:", fullname)

class Directory:
    def __init__(self, name, size: CodeSize):
        self.name = name
        self.size = size
        self.dirs = []
        self.files = []

    def __repr__(self):
        return "%s/ %s" % (self.name, self.size)

    def AddDir(self, d):
        self.dirs.append(d)

    def AddFile(self, name, size: CodeSize):
        self.files.append((os.path.basename(name), size.self_lines))

    def Print(self, indent):
        print("%*s%s/ %d" % (indent*2, "", self.name, self.size.all_lines))

        for f in self.files:
            print("%*s%s %d" % (indent*2 + 2, "", f[0], f[1]))
        for d in sorted(self.dirs, key=lambda d: d.size.all_lines, reverse=True):
            d.Print(indent + 1)

    def PrintHTML(self, out, indent):
        out.write("%*s<li>%s/ [%s]<ul>\n" % (indent*2, "", self.name, '{:,}'.format(self.size.all_lines)))
        for d in sorted(self.dirs, key=lambda d: d.size.all_lines, reverse=True):
            d.PrintHTML(out, indent + 1)
        for f in self.files:
            out.write("""%*s<li data-jstree='{"type":"file"}'>%s [%s]</li>\n""" %
                      (indent*2 + 2, "", f[0], '{:,}'.format(f[1])))
        out.write("%*s</ul></li>\n" % (indent*2, ""))

def main(argv):
    if len(argv) > 1:
        for path in argv[1:]:
            walk(path)
    else:
        walk(".")

    '''
    for k in sorted(sizes):
        if sizes[k].is_dir() > 0:
            print(k, sizes[k])
    print('==========')
    '''

    dirs = {}
    #root = Directory('', CodeSize())
    #dirs[''] = root
    for k in sorted(sizes):
        v = sizes[k]
        if v.is_dir():
            dirs[k] = Directory(os.path.basename(k), v)
        if not k:
            continue
        parent = os.path.split(k)[0]

        if v.is_dir():
            dirs[parent].AddDir(dirs[k])
        else:
            dirs[parent].AddFile(k, v)

    # dirs[''].Print(0)

    with open('call_tree.html', 'w') as out:
        out.write('''<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>jsTree test</title>
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/jstree/3.2.1/themes/default/style.min.css" />
</head>
<body>
  <input type="text" value="" id="demo_q" style="width: 450px;" placeholder="Search" />
  <div id="jstree">
    <ul>
''')

        dirs[''].PrintHTML(out, 0)

        out.write('''
    </ul>
  </div>

  <script src="https://cdnjs.cloudflare.com/ajax/libs/jquery/1.12.1/jquery.min.js"></script>
  <script src="https://cdnjs.cloudflare.com/ajax/libs/jstree/3.2.1/jstree.min.js"></script>
  <script>
  $(function () {
    var to = false;

    $('#demo_q').keyup(function () {
      if(to) { clearTimeout(to); }
      to = setTimeout(function () {
        var v = $('#demo_q').val();
        $('#jstree').jstree(true).search(v);
      }, 250);
    });

    $('#jstree').jstree({
      "types": {
        "default": { "icon": "jstree-folder" },
        "file": { "icon": "jstree-file" }
      },
      "plugins" : ["checkbox", "search", "types"]
    });
  });
  </script>
</body>
</html>''');

if __name__ == '__main__':
    main(sys.argv)
