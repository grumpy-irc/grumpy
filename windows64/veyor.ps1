# This is a powershell script that prepare grumpy project so that it can be built

#  Copyright (c) 2015, Petr Bena
#  All rights reserved.

#  Redistribution and use in source and binary forms, with
#  or without modification, are permitted provided that
#  the following conditions are met:

#  1. Redistributions of source code must retain
#     the above copyright notice, this list 
#     of conditions and the following disclaimer.

#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in
#     the documentation and/or other materials provided with the distribution.

#  3. Neither the name of Huggle nor the names of its contributors may be used
#     to endorse or promote products derived from this software without specific
#     prior written permission.

#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
#  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
#  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
#  THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
#  OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
#  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
#  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

param
(
    [string]$msbuild_path = "C:\Program Files (x86)\MSBuild\14.0\Bin\MSBuild.exe",
    [string]$root_path = $PWD,
    [string]$qt5_path = "C:\Qt\5.8\msvc2015_64\",
    #[string]$qt5_path = "D:\libs\Qt\5.9.1\msvc2015_64\",
    [string]$openssl_path = "C:\OpenSSL-Win64",
    [string]$cmake_generator = "Visual Studio 14 2015 Win64",
    [bool]$mingw = $false,
    [string]$mingw_path = "C:\Qt\Tools\mingw491_32",
    [bool]$python = $false,
    [string]$cmake_param = "",
    [string]$vcinstall_path = "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\"
)

$ErrorActionPreference = "Stop"

function PackageTest
{
    param
    (
        [parameter(Mandatory=$true)]
        [ValidateNotNullOrEmpty()]
        [string] $PackageName,

        [parameter(Mandatory=$true)]      
        [ValidateNotNullOrEmpty()]
        [string] $PackageUrl,

        [parameter(Mandatory=$true)]
        [string] $VariableName
    )

    Write-Host "Looking for $PackageName...    " -NoNewLine
    if (!(Test-Path $PackageUrl))
    {
        echo "ERROR"
        echo "Unable to find $PackageName at $PackageUrl, you can set the alternative path using -$VariableName=path"
        exit 1
    }
    echo ("OK");
}

function local_wget
{
    param
    (
        [parameter(Mandatory=$true)]
        [ValidateNotNullOrEmpty()]
        [string] $url,
        [parameter(Mandatory=$true)]
        [ValidateNotNullOrEmpty()]
        [string] $output
    )
    $wc = New-Object System.Net.WebClient
    $wc.DownloadFile($url, $output)
}

Write-Host "Checking paths...     " -NoNewline
cp ../LICENSE.txt gpl.txt

if (!(Test-Path "gpl.txt"))
{
    echo "ERROR"
    echo "Unable to find license, are you in right folder?"
    exit 1
}

if (!(Test-Path "../huggle/configure.ps1"))
{
    echo "ERROR"
    echo "This isn't a huggle windows folder, you need to run this script from within the ROOT/windows folder"
    exit 1
}

if ((Test-Path ".\build"))
{
    echo "ERROR"
    echo "The build folder is already present, please remove it first"
    exit 1
}

if ((Test-Path ".\release"))
{
    echo "ERROR"
    echo "The release folder is already present, please remove it first"
    exit 1
}

echo "OK"

if ($mingw)
{
    PackageTest "MingW" "$mingw_path" "mingw_path"
} else
{
    PackageTest "MSBuild" "$msbuild_path" "msbuild_path"
}
PackageTest "Qt5" "$qt5_path" "qt5_path"
#PackageTest "OpenSSL" "$openssl_path" "openssl_path"
$git_enabled = $true
Write-Host "Looking for git...    " -NoNewline
if (!(Get-Command git -errorAction SilentlyContinue))
{
    echo "NOT FOUND"
    $git_enabled = $false
    exit 1
}
echo "OK"
Write-Host "Looking for cmake...    " -NoNewline
if (!(Get-Command cmake -errorAction SilentlyContinue))
{
    echo "ERROR"
    echo "Unable to find cmake powershell snippet in this system"
    exit 1
}
echo "OK"

echo "Configuring the project..."

#let's try to invoke cmake now
cd $root_path
echo "Running cmake"
mkdir build | Out-Null
cd build

# Hack to fix crappy new cmake
$ErrorActionPreference = "Continue"

if ($python)
{
    cmake ..\..\src\ -G "$cmake_generator" -DCMAKE_PREFIX_PATH:STRING=$qt5_path -Wno-dev -DQT5_BUILD=true $cmake_param
} else
{
    cmake ..\..\src\ -G "$cmake_generator" -DCMAKE_PREFIX_PATH:STRING=$qt5_path -Wno-dev -DQT5_BUILD=true $cmake_param
}

$ErrorActionPreference = "Stop"

if ($mingw)
{
    & mingw32-make.exe
} else
{
    & $msbuild_path "GrumpyIRC.sln" "/p:Configuration=Release" "/v:minimal"
}
cd $root_path
echo "Preparing the package structure"
mkdir release | Out-Null
mkdir release\platforms | Out-Null
mkdir release\extensions | Out-Null
cp .\build\bin\Release\*.dll release
#cp .\build\Release\extensions\*.dll release\extensions
cp .\build\bin\Release\*.exe release
# get the qt
cp ..\src\GrumpyIRC\img\grumpy_logo.ico grumpy.ico
cp ..\src\GrumpyIRC\img\grumpy_logo.ico release\grumpy.ico
cp $openssl_path\bin\ssleay32.dll release
cp $openssl_path\bin\libeay32.dll release
# get openssl packs
local_wget "http://petr.insw.cz/devel/ssl/ssleay32.dll" "ssleay32.dll"
local_wget "http://petr.insw.cz/devel/ssl/libeay32.dll" "libeay32.dll"

# get the qt
cp ..\src\GrumpyIRC\img\grumpy_logo.ico grumpy.ico
cp ..\src\GrumpyIRC\img\grumpy_logo.ico release\grumpy.ico
cp ssleay32.dll release
cp libeay32.dll release

# Set the environment variable needed by windeployqt, todo: check if it's already set
$env:VCINSTALLDIR = $vcinstall_path

Invoke-Expression "$qt5_path\bin\windeployqt.exe release\grumpy.exe"

