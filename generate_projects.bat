pushd .

cd /D "%~dp0"

sharpmake-scripts\sharpmake-binaries\Sharpmake.Application.exe /sources("sharpmake-scripts/Program.cs")

popd

if "%1" NEQ "" (
	goto nopause
)

pause
	
:nopause