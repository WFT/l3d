import sys

if len(sys.argv) > 1:
    with open(sys.argv[1]) as f:
        i = 1
        y = -1
        for l in f:
            i += 1
            l = l.strip()
            if l == 'MAN DOWN':
                break
            elif l == 'drew triangle':
                y = -1
                continue
            elif l[0:2] != 'x1':
                continue
            newy = int(l.split(':')[-1])
            if y == -1:
                y = newy
            else:
                if newy != y - 1:
                    if y == newy:
                        print 'eq',
                    else:
                        print 'nequals',
                    print 'found on line ', i

print 'done;'
