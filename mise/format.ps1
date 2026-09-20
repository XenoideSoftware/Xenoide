$ErrorActionPreference = "Stop"

Get-ChildItem -Path "src/" -Recurse -File -Include "*.cpp","*.h","*.hpp","*.c","*.cc","*.cxx" | ForEach-Object {
    clang-format -i $_.FullName
}