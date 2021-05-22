#!/usr/bin/python3

import re, subprocess

bs = 1
count = 1024 * 1024
while bs <= 1024 * 1024 * 8:
    args = ['dd', 'if=/dev/zero', 'of=/dev/null', 'bs=%d' % bs, 'count=%d' % count]
    result = subprocess.run(args, capture_output=True)
    seconds = 0
    message = str(result.stderr)
    if m := re.search('copied, (.*?) s, ', message):
        seconds = float(m.group(1))
    elif m := re.search('bytes transferred in (.*?) secs', message):
        seconds = float(m.group(1))
    else:
        print('Unable to parse dd output:\n%s' % message)
        break
    print('bs=%7d count=%7d %6.3fs %8.3fus/record %9.3fMB/s' %
            (bs, count, seconds, seconds * 1e6 / count, bs * count / 1e6 / seconds))

    bs *= 2
    if seconds > 1:
        count /= 2
