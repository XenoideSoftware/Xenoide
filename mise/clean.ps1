$ErrorActionPreference = "Stop"

Remove-Item -Recurse -Force "build","build-*" -ErrorAction SilentlyContinue