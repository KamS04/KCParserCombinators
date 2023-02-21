$win = "$pwd\build.bat $pwd\dist\parselib-win\libparselib.a"
$winDebug = "$pwd\dbuild.bat $pwd\dist\parselib-win\libdparselib.a"



Remove-Item "$pwd/dist/parselib-win/*"
Remove-Item "$pwd/dist/parselib-linux/*"

iex $win
iex $winDebug

wsl -- "./fbuild.sh"