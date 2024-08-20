# -*- coding: utf-8 -*-
'''
@Author   : xy_cloud
@IDE      : PyCharm
@Project  : Python Project
@File     : main.py
@Time     : 2022/12/20 10:22
'''

import os, json, re, sys
print(str(len(sys.argv)))
if len(sys.argv) != 3:
    print("2 arg only! 1. OutputFile 2. Source Dir")
    print("Example: \"python3 ./scripts/localization-parser xxx.json ./src/game/server/ \"")
    sys.exit()
OutputFile = sys.argv[1]

def jsonio(path):
    global data
    print(path)
    with open(path, 'r', encoding='utf-8') as f:
        text = f.read()
    print(text)
    all_list = re.findall(r', _\(\".*?\"\)', text)
    print(all_list)
    output_list = []
    for i in all_list:
        output_list.append(i[5:-2])
    print(output_list)
    for i in output_list:
        if i != '':
            if not {
                       'key': i,
                       'value': i
                   } in data['translation']:
                data['translation'].append({
                    'key': i,
                    'value': i
                })


def run_tree(filepath='./'):
    filetree = os.listdir(filepath)
    print(filetree)
    for i in filetree:
        # print(i[-2:], i[-4:])
        if not '.' in i:
            run_tree(filepath + i + '/')
        if i[-2:] == '.h' or i[-4:] == '.cpp':
            jsonio(filepath + i)


b = input('1.覆盖 2.承接:')
data = {
    'translation': [

    ]
}
if b == '2':
    with open(OutputFile, 'r', encoding='utf-8') as f:
        data = json.loads(f.read())
run_tree(sys.argv[2])
with open(OutputFile, 'w', encoding='utf-8') as f:
    f.write(json.dumps(data, indent=1, ensure_ascii=False))
