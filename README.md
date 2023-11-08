# [![N|Solid](https://naivesystems.com/_next/static/media/logo.25b8e43f.png)](https://naivesystems.com/)

*Static analysis for code security and compliance*

NaiveSystems Analyze helps you identify issues in your code early and ensure
compliance with functional safety and coding standards. Get started for free,
and scale up as needed.

## Coding Standards

NaiveSystems Analyze checks code for compliance with a variety of functional
safety, security, and coding standards, including

- [Motor Industry Software Reliability Association (MISRA)](https://misra.org.uk/):
Supports both MISRA C:2012 and MISRA C++:2008, enforce MISRA compliance.
- [AUTOSAR (AUTomotive Open System ARchitecture)](https://www.autosar.org/):
Ensures compilance with the AUTOSAR C++14 coding standard.
- [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html):
Check your code against Google C++ Style Guide.

To specify checking rules based on these coding standards, create a
`.naivesystems/check_rules` file in your project root directory. Use the
following format for rule definitions:

- MISRA C:2012: misra_c_2012/dir_1_1 or misra_c_2012/rule_1_1
- MISRA C++:2008: misra_cpp_2008/rule_0_1_1
- AUTOSAR: autosar/rule_A0_1_1 or autosar/rule_M0_1_1
- Google C++ Style Guide: googlecpp/g1149. A table documenting the
correspondence between the rule IDs and the actual rule texts can be found
[here](https://github.com/naivesystems/googlecpp/blob/main/google_cpp.check_rules.md).

For an example of a `.naivesystems/check_rules` file, refer to
[here](https://github.com/naivesystems/googlecpp-image/blob/main/google_cpp.check_rules.txt).

## How to Use

After setting up your `.naivesystems/check_rules` file, you can use
NaiveSystems Analyze in the following ways:

### Command Line Interface

In the root of your project, use the following commands for projects using
Makefile or CMakeLists.txt:

- For projects using Makefile:
```sh
mkdir -p output && \
podman run -v $PWD:/src:O -v $PWD/.naivesystems:/config:Z \
  -v $PWD/output:/output:Z \
  ccr.ccs.tencentyun.com/naivesystems/analyze:2022.1.0.515002 \
  /opt/naivesystems/misra_analyzer -show_results
```

- For projects using CMakeLists.txt:
```sh
mkdir -p output && \
podman run -v $PWD:/src:O -v $PWD/.naivesystems:/config:Z \
  -v $PWD/output:/output:Z \
  ccr.ccs.tencentyun.com/naivesystems/analyze:2022.1.0.515002 \
  /opt/naivesystems/misra_analyzer -show_results -project_type=cmake
```

You can start with the [demo](https://github.com/naivesystems/analyze-demo) as
a reference.

### Github Actions

NaiveSystems Analyze also support [Github Actions](https://docs.github.com/en/actions)
for checking your code against Google C++ Style. For more information, please
refer to the [Demo of Google C++ Style Analysis](https://github.com/naivesystems/googlecpp-demo).


## How to Build

NaiveSystems Analyze can be built on [Fedora 36](https://docs.fedoraproject.org/en-US/releases/f36/).

1. Install dependencies
```sh
dnf update -y && dnf install -y \
autoconf \
automake \
clang \
cmake \
libtool \
lld \
make \
npm \
patch \
python3 \
python3-devel \
wget \
which \
xz \
zip
```
You also need to install [Bazel](https://bazel.build/install?hl=en) and
[Golang](https://go.dev/).

2. Build the project
```sh
make
```

3. Build the image

```sh
cd podman_image && make build-en
```
It will build an image named `naive.systems/analyzer/misra:dev_en` to check
rules of MISRA C:2012. You can specify other targets if needed.


## License

The project is licensed under GNU GENERAL PUBLIC LICENSE.
