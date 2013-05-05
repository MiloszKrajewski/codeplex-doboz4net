Properties {
	$release = "1.0.0.0"
	$src = "..\source"
	$sln = "$src\Doboz.sln"
	$snk = "$src\Doboz.snk"
	$libz = "$src\packages\LibZ.Bootstrap.1.0.3.3\tools\libz.exe"
}

Include ".\common.ps1"

FormatTaskName (("-"*79) + "`n`n    {0}`n`n" + ("-"*79))

Task default -depends LibZ

Task LibZ -depends Release {
	Create-Folder libz
	
	copy-item any\*.dll libz\
	
	exec { cmd /c $libz inject-dll -a libz\Doboz.dll -i libz\*.dll -e Doboz.dll "--move" -k $snk }
}

Task Release -depends Rebuild {
	Create-Folder x86
	Create-Folder x64
	Create-Folder any

	copy-item "$src\Doboz\bin\Release\Doboz.dll" any\
	copy-item "$src\DobozN\bin\Release\DobozN.dll" any\
	copy-item "$src\DobozS\bin\Release\DobozS.dll" any\

	copy-item "$src\bin\Win32\Release\*.dll" x86\
	copy-item "$src\bin\x64\Release\*.dll" x64\

	copy-item x86\DobozMM.dll any\DobozMM.x86.dll
	copy-item x64\DobozMM.dll any\DobozMM.x64.dll
}

Task Version {
	Update-AssemblyVersion $src $release 'Tests'
}

Task Rebuild -depends VsVars,Clean,KeyGen,Version {
	Build-Solution $sln "Any CPU"
	Build-Solution $sln x86
	Build-Solution $sln x64
}

Task KeyGen -depends VsVars -precondition { return !(test-path $snk) } {
	exec { cmd /c sn -k $snk }
}

Task Clean {
	Clean-BinObj $src
	remove-item * -recurse -force -include x86,x64,any,libz
}

Task VsVars {
	Set-VsVars
}
