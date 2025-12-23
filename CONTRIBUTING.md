# Contributing to Adaptive Hybrid Consensus Engine for V2X Blockchain

Thank you for your interest in contributing! This document provides guidelines for contributing to the project.

## Getting Started

1. Fork the repository
2. Clone your fork locally
3. Create a feature branch: `git checkout -b feature/your-feature-name`
4. Make your changes
5. Test your changes
6. Commit with clear messages
7. Push to your fork
8. Submit a pull request

## Development Setup

### Prerequisites

- NS-3 3.35 or later
- C++17 compatible compiler
- Python 3.6+
- Git

### Building from Source

```bash
# In NS-3 root directory
./ns3 configure --enable-examples --enable-tests
./ns3 build
```

### Running Tests

```bash
# Run a quick test simulation
./ns3 run v2x-urban-sim --duration=10 --nNodes=10

# Verify output
python analysis/analyze_results.py v2x-urban-metrics.csv
```

## Code Style

### C++ Style

- Follow NS-3 coding style guide
- Use GNU indentation style (2 spaces)
- Include doxygen comments for public methods
- Use meaningful variable names

Example:
```cpp
/**
 * \brief Calculate informational entropy
 * \return Entropy value
 */
double
CalculateInformationalEntropy ()
{
  // Implementation
}
```

### Python Style

- Follow PEP 8
- Use 4 spaces for indentation
- Include docstrings for functions
- Use type hints where appropriate

## Commit Messages

Use clear, descriptive commit messages:

```
Add feature: entropy-based mode switching

- Implement informational entropy calculation
- Add spatial entropy calculation
- Update control loop to switch modes based on thresholds
```

## Pull Request Process

1. Ensure all tests pass
2. Update documentation if needed
3. Add a clear description of changes
4. Reference any related issues
5. Wait for review from maintainers

## Bug Reports

When reporting bugs, please include:

- NS-3 version
- Operating system
- Steps to reproduce
- Expected vs actual behavior
- Relevant log output or screenshots

## Feature Requests

For new features:

- Describe the use case
- Explain why it would be valuable
- Provide examples if possible
- Consider implementation approach

## Areas for Contribution

- **Consensus mechanisms**: Add new consensus algorithms
- **Mobility models**: Implement realistic vehicle mobility patterns
- **Energy models**: Improve energy consumption accuracy
- **Analysis tools**: Create new visualization scripts
- **Documentation**: Improve guides and examples
- **Testing**: Add unit tests and integration tests

## Code Review

All submissions require review. We aim to:

- Respond to PRs within 3-5 business days
- Provide constructive feedback
- Maintain code quality standards
- Ensure compatibility with NS-3

## License

By contributing, you agree that your contributions will be licensed under the GNU General Public License v2.

## Questions?

Open an issue or contact the maintainers if you have questions.

Thank you for contributing!
