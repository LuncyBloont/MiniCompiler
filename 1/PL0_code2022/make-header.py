
inputname = 'Grammar.txt'
filename = 'grammar.h'
varname = 'Grammar_0'

print('Input file(%s):' % (inputname))
ins = input()
inputname = ins if len(ins) > 0 else inputname

print('Output file(%s):' % (filename))
ins = input()
filename = ins if len(ins) > 0 else filename

print('Class name(%s):' % (varname))
ins = input()
varname = ins if len(ins) > 0 else varname

print('Make a header file(%s) with a string variable %s from %s' % (filename, varname, inputname))

headblock = filename.upper()
headblock = headblock.replace('.', '_')
print(headblock)

with open(inputname, 'r', encoding='utf8') as f:
	s = f.read()
	lines = s.split('\n')
	out = '#pragma once\n#ifndef %s\n#define %s\nconst char %s[] = \"\\\n' % (headblock, headblock, varname)
	for line in lines:
		out += line + '\\n\\\n'
	out += '";\n#endif // %s' % (headblock)
	print(out)

	with open(filename, 'w', encoding='utf8') as wf:
		wf.write(out)

print('Done')
input()