set file="..\\..\\PL0_code2022\\err0.in"
set in0="%file%"
set out0="%file%.out"
..\x64\Debug\PL0-pro.exe 0 < %in0% > %out0%

pause
if not %errorlevel% == 0 exit

set out10="%file%.chk.html"
..\x64\Debug\PL0-pro.exe 10 < %out0% > %out10%

pause
if not %errorlevel% == 0 exit

set out1="%file%.chk.tree"
..\x64\Debug\PL0-pro.exe 1 < %out0% > %out1%

pause
if not %errorlevel% == 0 exit

set out2="%file%.pl0exc"
..\x64\Debug\PL0-pro.exe 2 < %out1% > %out2%

pause
if not %errorlevel% == 0 exit

echo "done"