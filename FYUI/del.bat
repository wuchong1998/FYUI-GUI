
del *.db /f /s /q /a
del *.pdb /f /s /q /a
del *.ipdb /f /s /q /a
del *.iobj /f /s /q /a
del *.obj /f /s /q /a
del *.pch /f /s /q /a
del *.exp /f /s /q /a

for /f "delims=" %%i in ('dir /ad /b /s "Release"') do (
   rd /s /q "%%i")

for /f "delims=" %%i in ('dir /ad /b /s "Debug"') do (
   rd /s /q "%%i")

for /f "delims=" %%i in ('dir /ad /b /s "ipch"') do (
   rd /s /q "%%i")

for /f "delims=" %%i in ('dir /ad /b /s "Temp"') do (
   rd /s /q "%%i")


