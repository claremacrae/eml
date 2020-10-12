# Embedded ML
[![Build Status](https://travis-ci.org/LesleyLai/eml.svg?branch=master)](https://travis-ci.org/LesleyLai/eml)
[![Build status](https://ci.appveyor.com/api/projects/status/rf9t1bnli5rpj5r0/branch/master?svg=true)](https://ci.appveyor.com/project/LesleyLai/eml/branch/master)
[![codecov](https://codecov.io/gh/LesleyLai/eml/branch/master/graph/badge.svg)](https://codecov.io/gh/LesleyLai/eml)

Embedded ML aims to be a static-typed scripting language that gets performance and type safety of a static-typed language, while still remains most of the convenience of a dynamic-typed language. This project is in a very early stage, so I will make breaking changes frequently.


## Quick Start
See how to build the project [here](docs/build.md).

## Acknowledgements

Thanks to Sy Brand for their [tl::expected](https://github.com/TartanLlama/expected).

## Examples

<!-- snippet: integration_test.Integration_Test.approved.docs.txt -->
<a id='snippet-integration_test.Integration_Test.approved.docs.txt'></a>
```txt
Input:
if (5 > 1) {2 + 3} else {4 - 6}

Executes to:
5: Number

```
<sup><a href='/tests/integration_test.Integration_Test.approved.docs.txt#L1-L6' title='Snippet source file'>snippet source</a> | <a href='#snippet-integration_test.Integration_Test.approved.docs.txt' title='Start of snippet'>anchor</a></sup>
<!-- endSnippet -->
<!-- snippet: example_from_code -->
<a id='snippet-example_from_code'></a>
```cpp
const std::string s = "if (5 > 1) {2 + 3} else {4 - 6}";
```
<sup><a href='/tests/integration_test.cpp#L70-L72' title='Snippet source file'>snippet source</a> | <a href='#snippet-example_from_code' title='Start of snippet'>anchor</a></sup>
<!-- endSnippet -->
