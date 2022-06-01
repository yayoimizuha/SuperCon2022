Param([parameter(mandatory = $true)][Int]$ProblemNumber,
        [parameter(mandatory = $true)][Int]$SolverNumber)

#Please set build type. (content_root\cmake-build\$build_type)
$build_type = 'release'

$exec_name = ".\cmake-build-" + $build_type + "\sc" + $ProblemNumber + "\SuperCon2022-ProblemGenerator-Pre" + $ProblemNumber + ".exe 12345"
Write-Host $exec_name
$input_array = Invoke-Expression $exec_name
Write-Host $input_array
$exec_name = "Write-Output $input_array | .\cmake-build-" + $build_type + "\SuperCon2022-Pre" + $SolverNumber + ".exe"
Invoke-Expression $exec_name