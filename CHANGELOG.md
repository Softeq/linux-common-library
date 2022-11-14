# linux-common-library
All notable changes to this project will be documented in this file.

The format of the file is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)
Please use [Release workflow](https://portal.softeq.com/display/SDD/Git+flow#Gitflow-Releaseworkflow) and [Commit messages style guide](https://portal.softeq.com/display/SDD/Git+flow#Gitflow-Commitmessagesstyleguide)

See [README.md](README.md) for more details.

## [0.4.0] - 2022-10-31
### Added
- Added system time change monitoring functionality in DefTimeProvider
- TimeProvider tests.
- add xml serializer example
- Add migration component

### Changed
- New serializer approach
- Code duplications removed
- Change places of serializer and desirializer parameters. Add default nullptr for serializer parameter.
- Reject short component include pathes.
- Prevent export of subcomponents
- Update README.md
- Project moved to component based files structure
- Bug fixes

### Removed
- Removed SystemError. use std::system_error instead

## [0.3.0] - 2022-02-22
### Added
- add sysV service test case
- Add exceptions test suite
- Adding tests for sdbus module
- Implemented passing permissions for the new directories to 'mkdirs' method
- support DELETE method in http requests
- Add unit tests for HTTP Range functionality
- Support of PUT requests

### Changed
- cover with tests setHeartbeatPeriod(), uptime(), setError(), handleSignal() functions

[0.4.0]: https://stash.softeq.com/projects/emblab/repos/linux-common-library/browse?at=3dd2a09
[0.3.0]: https://stash.softeq.com/projects/emblab/repos/linux-common-library/browse?at=refs/tags/0.3.0
