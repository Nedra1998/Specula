# Changelog

[TOC]

All notable changes to this project will be documented in this file.

## [unreleased]

### Bug Fixes

- Completely disable tracy when not profiling ([832a501](https://github.com/Nedra1998/Specula/commit/832a50189a3ccf2b46e54ce80ac0a74573d97e01))

### Documentation

- Created readme ([1d38eaf](https://github.com/Nedra1998/Specula/commit/1d38eaf1bede1e2c168e05a1ed89d978e92267b1))
- Added license file ([8e13902](https://github.com/Nedra1998/Specula/commit/8e139027fdc1bd71d96cfde39040ec7fd64fb89c))
- Added changelog with git-cliff ([0d88635](https://github.com/Nedra1998/Specula/commit/0d886359401050494be98670f54dad206453003c))
- Added doc strings to some existing methods ([fdc5cb5](https://github.com/Nedra1998/Specula/commit/fdc5cb56a0988b731ae955e9879d27c95f1f64a2))
- Updated doxgyen configuration ([3b4db73](https://github.com/Nedra1998/Specula/commit/3b4db73b30f23cc90c2433e08a77708d7c162f2e))
- Added docstrings to float methods ([e6f8923](https://github.com/Nedra1998/Specula/commit/e6f8923ccfe95d77c8bf0c1a4d83c7f89ef1d130))
- Renamed license to markdown ([0a6d7be](https://github.com/Nedra1998/Specula/commit/0a6d7bea042dee4052d1ca4b584d700317127478))
- Updated custom css themes ([9b6d457](https://github.com/Nedra1998/Specula/commit/9b6d4578f926693bc38fad90eca5cd3c97441925))
- Update override css colors ([774bf46](https://github.com/Nedra1998/Specula/commit/774bf46a0d79d238e065b3b7aac1759a9f4d91bd))
- Added documentation for pstd array and complex ([417928d](https://github.com/Nedra1998/Specula/commit/417928d5f356d5da011d60cf509041b747143dac))

### Features

- Added logging and tracing support ([eb7fe96](https://github.com/Nedra1998/Specula/commit/eb7fe96e52ec73e03f83d4fe2e68cc3af286f780))
- Added catch2 test framework ([debad21](https://github.com/Nedra1998/Specula/commit/debad21ab7b57724e0a90ef6883460274160664c))
- Implemented doxygen documentation generation ([fcc6e64](https://github.com/Nedra1998/Specula/commit/fcc6e64d7a254793b44f1d75a0e7a7c07565f8e0))
- Added github actions CI ([df03ece](https://github.com/Nedra1998/Specula/commit/df03ecef80c9711b041004c0eb2867dc475727a2))
- Floating point utility methods ([84a3150](https://github.com/Nedra1998/Specula/commit/84a3150b32096c1b2650bcd8b7f18789053fbc05))
- Added debug assertion helper ([68af863](https://github.com/Nedra1998/Specula/commit/68af863f9ffdec73170ee7f844b3de27366f57ea))
- Initial statistics aggregation system ([715886c](https://github.com/Nedra1998/Specula/commit/715886c6c6545cc8f31bf0ed38d86122be0eb5a2))
- Implemented RARE_CHECK assertion ([0ae9b9d](https://github.com/Nedra1998/Specula/commit/0ae9b9df39dfd05d584fd4411b30f0ae01841ba1))
- Rich help text formatter ([79ea813](https://github.com/Nedra1998/Specula/commit/79ea8138ef4dab95365cda24f96cb249f8953a82))
- Added check callback and stacktrace support ([37f5163](https://github.com/Nedra1998/Specula/commit/37f5163d84a0738ccf0f50f1dd5e5d1590fd674a))
- Implemented hashing methods ([16fb893](https://github.com/Nedra1998/Specula/commit/16fb8932ac5454cdeca38b0525ec1abc76f82e4b))
- PCG pseudo-random number generator implementation ([b910bdf](https://github.com/Nedra1998/Specula/commit/b910bdf928fa794cc5daa1c1c9619c6e3a9ddb14))
- ProgressReporter implementation and bar renderer ([7f65ffa](https://github.com/Nedra1998/Specula/commit/7f65ffa36c1b0930023f73607959ab2abf2da980))
- Precomputed table of 1000 primes ([613edfb](https://github.com/Nedra1998/Specula/commit/613edfb8e48a69e302b54e00d69cf5a897794827))
- Implemented optional value ([bbf1c10](https://github.com/Nedra1998/Specula/commit/bbf1c10520734edcc8a9234d8f78a8a63a9e5e5f))
- Implemented pmr memory allocators ([729d1b4](https://github.com/Nedra1998/Specula/commit/729d1b4adfea08964216558758e81c1eb8927acd))
- Basic array implementation ([8006d1a](https://github.com/Nedra1998/Specula/commit/8006d1a8fd5a2bcf2d3eb25eeb5af2409a152bbd))
- Implemented tuple2 classes ([5363fdd](https://github.com/Nedra1998/Specula/commit/5363fddd64d399d85f7c3929400157ada3ef2d48))
- Implemented tuple3 classes ([d21022e](https://github.com/Nedra1998/Specula/commit/d21022eb02576152b804ae12b51517ce848e192f))
- Implemented span, tuple, vector, and complex ([04b200c](https://github.com/Nedra1998/Specula/commit/04b200c656db9cd1642fa21a32f7b7af0d1b3d2a))
- Introduce rgb color class ([03f2027](https://github.com/Nedra1998/Specula/commit/03f2027e4538b4fa30f3549b5e42fd23475d66d3))
- Add new math utilities and compensated sum/float types ([27390b3](https://github.com/Nedra1998/Specula/commit/27390b3178657b01a74a2dc9405a0c884cac6ff3))

### Miscellaneous Tasks

- Updated dependency versions ([4be0946](https://github.com/Nedra1998/Specula/commit/4be0946de0b6e74219240f35530b6a54ad920f6a))
- Bump dependency versions ([c1ac454](https://github.com/Nedra1998/Specula/commit/c1ac45449a275c314cb5233fc294080a9ce23618))

### Testing

- Tests for floating point utilities ([691303e](https://github.com/Nedra1998/Specula/commit/691303e15f26d61caa1637fa2bdc238fc8f6c0f1))
- Added basic pstd optional tests ([3ab3ed6](https://github.com/Nedra1998/Specula/commit/3ab3ed6dddeb61cc0f603e498d4fa7b14691fd3b))
- Add unit-tests for pstd array and complex ([1544dde](https://github.com/Nedra1998/Specula/commit/1544dde4a16cc4529a8d98bf47c75e35368e9d24))

### Build

- Implemented main cmake build system ([5b9369a](https://github.com/Nedra1998/Specula/commit/5b9369a7e4fb32789c191be66ab87c45e06c601c))
- Removed auto linters in build ([4505434](https://github.com/Nedra1998/Specula/commit/4505434bcd21edee9e7c921b92ba409b19acb933))
- Refactored cmake configuration ([8d732c9](https://github.com/Nedra1998/Specula/commit/8d732c9c23b5c1beb184aa069a7f12e61ea2d962))

### Ci

- Remove test reporting ([84307c3](https://github.com/Nedra1998/Specula/commit/84307c3dfad3db7578142b4a1ef6c610e6337a7b))
- Disable docs target during ([5d0e4d0](https://github.com/Nedra1998/Specula/commit/5d0e4d0a3a813fb0fce5b01ed2b4ff83fcdc772d))
- Update checkout action version ([8361b8b](https://github.com/Nedra1998/Specula/commit/8361b8be6a9c9c5db8884dc944fb34f0acc87c14))

<!-- generated by git-cliff -->
