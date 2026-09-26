# Automated Testing Guidelines

This guide complements @CPP.md with additional guidelines for creating unit tests.

## Glossary
- **SUT**: Subject Under Test

## Core Testing Principles

### 0. Respect

Treat Testing Code with the same respect as feature code. Pass it to format, linters, and any other declared tool that is used to improve the code quality.

### 0. Test Public Interfaces

Choose classes that are intended to be used in the public interface of a library / component: They tend to be more stable than code reused privately in that component. The whole purpose of Testing is to ensure that code still works after a **refactor** operation after all.

### 1. High-Performance testing 
- Prefer generating input data and SUTs in memory over filesystems. 
- **AVOID** dependency on external resources such as databases, APIs, etc.

### 1. Prefer Property-Based Testing over Example-Based Testing

We need to make tests resistant against changes to the input data:

- **Dynamic Data Generators**: Input data must be generated using custom, parametric synthetic random data generator, according to the domain of the SUT. Use the Builder Pattern to make explicit the parameters being used  with the form `generate([Entity]Builder().param1().param2()....build())`. 
- **Property Assertions**: Create utility assertions with the form `requireEntityProperty`, which must check that the entity have the desired state.

### 2. Determinism, Independence & Zero Regressions

- **Determinism**: Synthetic random data generator must use Catch'2 current execution seed, in order to have reproducible tests.
- **Zero Test Coupling:** Every test must run independently in any execution order and concurrently without shared mutable state.
- **Zero Resource Leaks:** Any temporary file, buffer, or OS handle created during a test must be freed immediately upon test completion.
- **Non-Regression:** When adding or refactoring tests, verify that all existing tests in the suite continue to pass.

### 3. Don't reinvent the wheel.
- **Reuse existing facilities whenever possible**: When creating new generators and property assertions, always put them in a common translation or library corresponding to the Project, in order to be reused across tests and reduce repetitive code.
