# UTHelper Plugin Usage Examples

The UTHelper plugin provides multiple features for C++ code transformation.

## 1. Remove Final Keywords

Remove `final` keywords from classes and methods within a specified directory:

```bash
clang++ -Xclang -load -Xclang path/to/UTHelperPlugin.so \
        -Xclang -plugin -Xclang uthelper \
        -Xclang -plugin-arg-uthelper -Xclang remove-final \
        -Xclang -plugin-arg-uthelper -Xclang "base-folder=/path/to/project" \
        -fsyntax-only input.cpp > output.cpp
```

This will:
- Remove `final` from all classes and structs
- Remove `final` from virtual methods
- Only process files under `/path/to/project`

## 2. Make Methods Virtual

Convert non-virtual methods to virtual (where C++ grammar allows):

```bash
clang++ -Xclang -load -Xclang path/to/UTHelperPlugin.so \
        -Xclang -plugin -Xclang uthelper \
        -Xclang -plugin-arg-uthelper -Xclang make-virtual \
        -Xclang -plugin-arg-uthelper -Xclang "base-folder=/path/to/project" \
        -fsyntax-only input.cpp > output.cpp
```

This will:
- Add `virtual` keyword to non-static member functions
- Skip constructors (cannot be virtual)
- Handle destructors appropriately
- Skip static methods and friend functions

## 3. Combined Mode for Unit Testing

Remove final and make methods virtual in one pass (ideal for creating mockable classes):

```bash
clang++ -Xclang -load -Xclang path/to/UTHelperPlugin.so \
        -Xclang -plugin -Xclang uthelper \
        -Xclang -plugin-arg-uthelper -Xclang remove-final \
        -Xclang -plugin-arg-uthelper -Xclang make-virtual \
        -Xclang -plugin-arg-uthelper -Xclang "base-folder=/path/to/project" \
        -fsyntax-only input.cpp > output.cpp
```

This enables mocking of previously final classes with Google Mock or other mocking frameworks.

## 4. Function Wrapping with Pointcuts

Wrap functions based on pointcut definitions:

```bash
clang++ -Xclang -load -Xclang path/to/UTHelperPlugin.so \
        -Xclang -plugin -Xclang uthelper \
        -Xclang -plugin-arg-uthelper -Xclang "pointcut=pointcut.pc" \
        -Xclang -plugin-arg-uthelper -Xclang "base-folder=/path/to/project" \
        -fsyntax-only input.cpp > output.cpp
```

Example pointcut file (`pointcut.pc`):
```
run_pointcut funcDecl = pragma_clang(text, data) || annotation(wrap);
```

## 5. Arguments

- `remove-final`: Enable final keyword removal mode
- `make-virtual`: Convert methods to virtual where possible
- `pointcut=<file>`: Path to pointcut definition file (for function wrapping)
- `base-folder=<path>`: Only transform files under this directory (optional)

Multiple operations can be combined (e.g., `remove-final` + `make-virtual`)

## Notes

- The plugin outputs transformed code to stdout
- Use `-fsyntax-only` to avoid compilation, just transformation
- The `base-folder` parameter accepts both absolute and relative paths
- When no `base-folder` is specified, all files are processed