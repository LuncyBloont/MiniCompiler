set file="./PL0_code-funs.in"
set in0="%file%"
set out0="%file%.out"
..\PL0-pro\x64\Debug\PL0-pro.exe 0 < %in0% > %out0%

if not %errorlevel% == 0 exit

set out10="%file%.chk.html"
..\PL0-pro\x64\Debug\PL0-pro.exe 10 < %out0% > %out10%

if not %errorlevel% == 0 exit

set out1="%file%.chk.tree"
..\PL0-pro\x64\Debug\PL0-pro.exe 1 < %out0% > %out1%

if not %errorlevel% == 0 exit

set out2="%file%.pl0exc"
..\PL0-pro\x64\Debug\PL0-pro.exe 2 < %out1% > %out2%

if not %errorlevel% == 0 exit


echo "done"