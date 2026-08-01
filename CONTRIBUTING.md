# Contributing to Open-Carbon-Tablet

Thank you for your interest in contributing to OSICT! This document describes the contribution process and standards.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [How Can I Contribute?](#how-can-i-contribute)
- [Development Workflow](#development-workflow)
- [Style Guidelines](#style-guidelines)
- [Reporting Bugs](#reporting-bugs)
- [Suggesting Enhancements](#suggesting-enhancements)
- [Pull Request Process](#pull-request-process)

## Code of Conduct

Be respectful and constructive. Harassment or discrimination of any kind will not be tolerated.

## How Can I Contribute?

- **Bug Reports:** Found a bug? Open an issue with reproduction steps.
- **Feature Requests:** Have an idea? Open an issue with the "enhancement" label.
- **Documentation:** Improvements to docs are always welcome.
- **Firmware:** Help improve scanning algorithms, mux control, or protocol efficiency.
- **Hardware:** Suggest circuit improvements, schematic reviews, or PCB layouts.
- **Software:** Enhance the Python receiver, visualizer, or biometric analysis modules.

## Development Workflow

1. **Fork** the repository.
2. **Clone** your fork: `git clone https://github.com/<your-username>/Open-Carbon-Tablet.git`
3. **Create a branch**: `git checkout -b feature/my-new-feature`
4. **Commit** your changes: `git commit -m "Add: description of my feature"`
5. **Push** to your fork: `git push origin feature/my-new-feature`
6. **Open a Pull Request** against the `main` branch.

### Commit Message Convention

We use a simplified convention:

```
<type>: <short description>

<optional body>
```

**Types:**
- `Add:` New feature or file
- `Fix:` Bug fix
- `Update:` Modification to existing feature
- `Docs:` Documentation only
- `Refactor:` Code restructuring without behavior change
- `Test:` Adding or updating tests

## Style Guidelines

### Arduino/C++ (Firmware)

- Use `camelCase` for variables and functions.
- Use `PascalCase` for classes and structs.
- Keep functions small and focused (max ~50 lines).
- Comment complex logic; avoid commenting obvious code.
- Use `#ifndef` include guards in header files.

### Python (Software)

- Follow [PEP 8](https://peps.python.org/pep-0008/) style guidelines.
- Use `snake_case` for functions and variables.
- Use `PascalCase` for classes.
- Add docstrings to all public functions and classes.
- Use `type hints` where practical.

## Reporting Bugs

When reporting a bug, please include:

1. **Title:** Clear, concise description of the problem.
2. **Environment:** OS, Arduino IDE version, board type, Python version.
3. **Steps to reproduce:** Detailed, numbered steps.
4. **Expected behavior:** What you expected to happen.
5. **Actual behavior:** What actually happened.
6. **Logs/Screenshots:** Serial output, error messages, or photos of the hardware.

## Suggesting Enhancements

When suggesting an enhancement, please include:

1. **Problem statement:** What problem does this solve?
2. **Proposed solution:** How would you solve it?
3. **Alternatives considered:** Any other approaches?
4. **Additional context:** Screenshots, mockups, or references.

## Pull Request Process

1. Ensure your code compiles/runs without errors.
2. Report CHANGELOG.md under the [Unreleased] section.
3. If adding a new feature, update relevant documentation.
4. If adding a dependency, update `requirements.txt` or library dependencies.
5. Ensure your PR addresses only one feature/fix - keep it focused.
6. Request a review from the maintainer.

### PR Template

```markdown
## Description
Brief description of changes.

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Documentation update
- [ ] Refactor
- [ ] Breaking change

## Testing
Describe how you tested the changes.

## Checklist
- [ ] Code follows style guidelines
- [ ] Self-review completed
- [ ] Documentation updated
- [ ] CHANGELOG.md updated
```

## Questions?

Feel free to [open an issue](https://github.com/walidddhony-rgb/Open-Carbon-Tablet/issues) with the "question" label.
