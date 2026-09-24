# CONAN guidelines

## Custom Conan Packages (`conan/packages/`)

Xenoide maintains custom Conan recipes inside `conan/packages/`. Several of these packages contain custom build logic, patches, or source code.

## Exporting Local Recipes
Whenever changes are made to any local recipe or in-tree package source (such as `glazer` or `glazed`), the recipes must be re-exported to the local Conan cache before installing dependencies:
```bash
mise run setup:export-recipes
```
This script iterates through each directory in `conan/packages/` and executes `conan export`.
