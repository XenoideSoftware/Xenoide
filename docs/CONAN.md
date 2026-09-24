# CONAN guidelines

## Custom Conan Packages (`../conan/recipes/`)

Xenoide maintains custom Conan recipes inside `../conan/recipes/`. Several of these packages contain custom build logic, patches, or source code.

## Exporting Local Recipes
Whenever changes are made to any local recipe or in-tree package source (such as `glazer` or `glazed`), the recipes must be re-exported to the local Conan cache before installing dependencies:
```bash
mise run export-recipes
```
This script iterates through each directory in `../conan/recipes/` and executes `conan export`.
