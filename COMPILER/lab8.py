#!/bin/python3

def get_use_def(CFG):
    use_def = {}

    for key in CFG.keys():
        use = set()
        define = set()
        stmts = CFG[key]['stmts']
        for stmt in stmts:
            if '=' in stmt:
                left = stmt.split('=')[0].strip()
                right = stmt.split('=')[1].strip()

                define.add(left)
                if '+' in right:
                    l = right.split('+')[0].strip()
                    r = right.split('+')[1].strip()

                    if not l.isnumeric():
                        use.add(l)
                    if not r.isnumeric():
                        use.add(r)
                elif not right.isnumeric():
                    use.add(right)

            elif "return" in stmt:
                use.add(stmt[6:].strip())

            use_def[key] = { 'use': use, 'def': define}

    return use_def

def live_var_analysis(CFG, use_def):
    IN = {b: set() for b in CFG.keys() }
    OUT = {b: set() for b in CFG.keys() }

    changed = True
    while (changed):
        changed = False

        for key in reversed(CFG.keys()):
            old_in = IN[key].copy()
            old_out = OUT[key].copy()

            OUT[key] = set()
            for succ in CFG[key]['succ']:
                OUT[key] |= IN[succ]

            IN[key] = use_def[key]['use'] | (OUT[key] - use_def[key]['def'])

            changed |= (old_in != IN[key] or old_out != OUT[key])
    return IN, OUT

CFG = {
    'B1': { 'stmts': ["a = 5", "b = 10"], 'succ': ["B2"]},
    'B2': { 'stmts': ["c = a + b"], 'succ': ["B3"]},
    'B3': { 'stmts': ["d = c + 2"], 'succ': ["B4"]},
    'B4': { 'stmts': ["e = d + 1"], 'succ': ["B5"]},
    'B5': { 'stmts': ["return c"], 'succ': []},
}

use_def = get_use_def(CFG)
print(use_def)
IN, OUT = live_var_analysis(CFG, use_def)
print(IN)
print(OUT)
