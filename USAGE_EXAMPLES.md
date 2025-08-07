# UTHelper Plugin Usage Examples

The UTHelper plugin transforms C++ code to improve testability. By default, all transformations are enabled.

## Important: base-folder is MANDATORY

The `base-folder` parameter must always be provided. This ensures transformations are only applied to files within your project directory.

```bash
clang++ -Xclang -load -Xclang path/to/UTHelperPlugin.so \
        -Xclang -plugin -Xclang uthelper \
        -Xclang -plugin-arg-uthelper -Xclang "base-folder=/path/to/project" \
        -fsyntax-only input.cpp > output.cpp
```

## Default Behavior (All Features Enabled)

By default, the plugin applies ALL transformations:
1. Removes `final` keywords from classes and methods
2. Makes methods virtual (where C++ allows)
3. Adds friend declarations to classes/structs

```bash
# All transformations are applied
clang++ -Xclang -load -Xclang path/to/UTHelperPlugin.so \
        -Xclang -plugin -Xclang uthelper \
        -Xclang -plugin-arg-uthelper -Xclang "base-folder=/path/to/project" \
        -fsyntax-only input.cpp > output.cpp
```

## Disabling Specific Features

Use disable flags to turn off specific transformations:

### Disable Remove Final
```bash
clang++ -Xclang -load -Xclang path/to/UTHelperPlugin.so \
        -Xclang -plugin -Xclang uthelper \
        -Xclang -plugin-arg-uthelper -Xclang "base-folder=/path/to/project" \
        -Xclang -plugin-arg-uthelper -Xclang disable-remove-final \
        -fsyntax-only input.cpp > output.cpp
```

### Disable Make Virtual
```bash
clang++ -Xclang -load -Xclang path/to/UTHelperPlugin.so \
        -Xclang -plugin -Xclang uthelper \
        -Xclang -plugin-arg-uthelper -Xclang "base-folder=/path/to/project" \
        -Xclang -plugin-arg-uthelper -Xclang disable-make-virtual \
        -fsyntax-only input.cpp > output.cpp
```

### Disable Add Friend
```bash
clang++ -Xclang -load -Xclang path/to/UTHelperPlugin.so \
        -Xclang -plugin -Xclang uthelper \
        -Xclang -plugin-arg-uthelper -Xclang "base-folder=/path/to/project" \
        -Xclang -plugin-arg-uthelper -Xclang disable-add-friend \
        -fsyntax-only input.cpp > output.cpp
```

### Multiple Disable Flags
```bash
# Only remove-final will be active
clang++ -Xclang -load -Xclang path/to/UTHelperPlugin.so \
        -Xclang -plugin -Xclang uthelper \
        -Xclang -plugin-arg-uthelper -Xclang "base-folder=/path/to/project" \
        -Xclang -plugin-arg-uthelper -Xclang disable-make-virtual \
        -Xclang -plugin-arg-uthelper -Xclang disable-add-friend \
        -fsyntax-only input.cpp > output.cpp
```

## Custom Friend Classes

When add-friend is enabled (default), you can specify custom friend templates:

```bash
clang++ -Xclang -load -Xclang path/to/UTHelperPlugin.so \
        -Xclang -plugin -Xclang uthelper \
        -Xclang -plugin-arg-uthelper -Xclang "base-folder=/path/to/project" \
        -Xclang -plugin-arg-uthelper -Xclang "custom-friends=class {namespace}::{class-name}Mock;class testing::TestFor{class-name}" \
        -fsyntax-only input.cpp > output.cpp
```

Default friends added to each class/struct:
- `friend class {ClassName}Test;`
- `template<typename T> friend class {ClassName}Friend;`

Template variables in custom friends:
- `{namespace}`: Replaced with the class's namespace
- `{class-name}`: Replaced with the class name

## Pointcut Mode (Function Wrapping)

Pointcut mode is separate from the default transformations. When a pointcut file is specified, ONLY function wrapping is performed:

```bash
clang++ -Xclang -load -Xclang path/to/UTHelperPlugin.so \
        -Xclang -plugin -Xclang uthelper \
        -Xclang -plugin-arg-uthelper -Xclang "base-folder=/path/to/project" \
        -Xclang -plugin-arg-uthelper -Xclang "pointcut=pointcut.pc" \
        -fsyntax-only input.cpp > output.cpp
```

## Complete Arguments Reference

### Mandatory:
- `base-folder=<path>`: Base directory for transformations (REQUIRED)

### Optional:
- `disable-remove-final`: Disable final keyword removal
- `disable-make-virtual`: Disable making methods virtual
- `disable-add-friend`: Disable adding friend declarations
- `custom-friends=<list>`: Semicolon-separated list of friend templates
- `pointcut=<file>`: Switch to pointcut mode (disables other transformations)

## Examples

### For Unit Testing with Google Mock
```bash
# Default: All transformations for maximum mockability
clang++ -Xclang -load -Xclang UTHelperPlugin.so \
        -Xclang -plugin -Xclang uthelper \
        -Xclang -plugin-arg-uthelper -Xclang "base-folder=/my/project" \
        -fsyntax-only MyClass.cpp > MyClass_testable.cpp
```

### For Minimal Changes
```bash
# Only remove final keywords
clang++ -Xclang -load -Xclang UTHelperPlugin.so \
        -Xclang -plugin -Xclang uthelper \
        -Xclang -plugin-arg-uthelper -Xclang "base-folder=/my/project" \
        -Xclang -plugin-arg-uthelper -Xclang disable-make-virtual \
        -Xclang -plugin-arg-uthelper -Xclang disable-add-friend \
        -fsyntax-only MyClass.cpp > MyClass_nofinal.cpp
```

## Notes

- The plugin outputs transformed code to stdout
- Use `-fsyntax-only` to avoid compilation, just transformation
- The base-folder ensures only your project files are modified (not system headers)
- All features are enabled by default for maximum test coverage capability