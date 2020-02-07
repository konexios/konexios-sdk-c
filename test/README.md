# Unit Tests

These unit tests use the [Ceedling](http://www.throwtheswitch.org/ceedling) framework. Installation instructions are given below; see the official docs for more detail.

### Installing Ceedling

Ceedling requires Ruby and is available as a ruby gem. On Ubuntu or similar, this will install ruby and Ceedling:

```shell
sudo apt install ruby
sudo gem install ceedling
```

This should provide a `ceedling` binary in `/usr/local/bin/`.

### private.h

If you get errors related to `private.h` file missing, see the documentation at the project root level on the subject. A quick fix, from this test directory:

```shell
touch ../private.h
```

These tests mock out the interaction with Arrow Connect *(TODO: is this correct?)*, so the credentials normally stored in this file aren't used anyway.

### Running Tests

The simplest invocation is (from the test directory, e.g. this directory):

```shell
ceedling test:all
```

Ceedling provides flexibility in how tests are run; see `ceedling --help` or the official docs for more detail.

### Status

At the time of this writing, there are 96 tests and all are passing. Excluding compilation, these tests run in 0.13 seconds on an Intel i7-7700HQ.

The tests only seem to cover positive cases (e.g. "happy-path" responses from Arrow Connect, etc.) and don't cover how the SDK handles invalid input. In my experience with this SDK, these are where most of the problems occur, and this type of test coverage is at least as important.

### TODO

The following are some ways in which these unit tests could be made more useful.

* Integration with Eclipse / other IDEs
* Ability to target actual hardware (e.g. STM32)
* Cover invalid input, error responses, empty responses, etc.
* Integrate into main build (again, the tests run quickly)

