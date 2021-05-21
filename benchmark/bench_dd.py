#!/usr/bin/python3

import re, subprocess

bs = 1
count = 1024 * 1024
while bs <= 1024 * 1024 * 8:
    args = ['dd', 'if=/dev/zero', 'of=/dev/null', 'bs=%d' % bs, 'count=%d' % count]
    result = subprocess.run(args, capture_output=True)
    m = re.search('copied, (.*?) s, ', str(result.stderr))
    seconds = float(m.group(1))
    print('bs=%7d count=%7d %6.3fs %7.3fus/record %9.3fMB/s' %
            (bs, count, seconds, seconds * 1e6 / count, bs * count / 1e6 / seconds))

    bs *= 2
    if seconds > 1:
        count /= 2
