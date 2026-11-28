
# Setup
Install Conan dependencies requiring a C++23 target:

```
conan install . --build=missing --settings=build_type=Debug --settings=compiler.cppstd=23
```
