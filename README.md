# Qualix Tokenizer

A production-grade, Unicode-first, grapheme-aware tokenizer and trainer written entirely in modern C++20.

The goal of Qualix Tokenizer is to provide a high-performance tokenizer library for training and inference of modern Large Language Models (LLMs) without depending on external tokenizer frameworks.

---

# Features

- Modern C++20
- Unicode First
- Grapheme First
- UTF-8 Native
- High Performance
- Cross Platform
- Modular Architecture
- Production Ready
- Extensive Testing

---

# Project Structure

```
Qualix_Tokenizer/

include/
src/
tests/
docs/
examples/
benchmarks/
cmake/

CMakeLists.txt
README.md
LICENSE
.gitignore
```

---

# Core Modules

## Types

Location

```
include/core/types.hpp
```

Purpose

- Fixed-width integer types
- Common aliases
- Platform-independent primitive types

Examples

- i8
- u8
- i16
- u16
- i32
- u32
- i64
- u64
- usize

---

## Error

Location

```
include/core/error.hpp
```

Purpose

Centralized error definitions.

Responsibilities

- File errors
- Unicode errors
- Vocabulary errors
- Dataset errors
- Internal errors

---

## Status

Location

```
include/core/status.hpp
```

Purpose

Represents success or failure of an operation.

Responsibilities

- Success state
- Failure state
- Error retrieval
- Human-readable messages

---

## Result

Location

```
include/core/result.hpp
```

Purpose

Stores both a value and operation status.

Responsibilities

- Safe return values
- Error propagation
- Generic template container

---

## Config

Location

```
include/core/config.hpp
```

Purpose

Stores runtime configuration.

Responsibilities

- Thread count
- Chunk size
- Logging
- Reports
- Runtime settings

---

## Version

Location

```
include/core/version.hpp
```

Purpose

Library version information.

Responsibilities

- Major
- Minor
- Patch
- Version string
- Library name

---

## Logger

Location

```
include/core/logger.hpp
```

Purpose

Thread-safe logging system.

Responsibilities

- Trace
- Debug
- Info
- Warning
- Error
- Fatal

---

## Platform

Location

```
include/core/platform.hpp
```

Purpose

Detect compiler and operating system.

Responsibilities

- Platform detection
- Compiler detection
- Platform names
- Compiler names

---

# Unicode Modules

Purpose

Complete Unicode processing.

Modules

- UTF-8
- UTF-16
- UTF-32
- Code Points
- Grapheme Clusters
- Unicode Normalization

Responsibilities

- Unicode validation
- Character iteration
- Encoding conversion
- Normalization
- Grapheme segmentation

---

# Vocabulary Modules

Purpose

Vocabulary management.

Modules

- Token
- Vocabulary
- Merges
- Special Tokens

Responsibilities

- Vocabulary loading
- Vocabulary saving
- Token lookup
- Merge lookup

---

# Normalizer

Purpose

Normalize text before tokenization.

Responsibilities

- Unicode normalization
- Lowercase
- Whitespace normalization
- Character cleanup

---

# Pretokenizer

Purpose

Split raw text into initial pieces.

Responsibilities

- Word boundaries
- Numbers
- URLs
- Emails
- Dates
- Symbols
- Emoji
- Mixed-language text

---

# Tokenizer

Purpose

Convert text into tokens.

Responsibilities

- Encode
- Decode
- Token IDs
- Attention masks
- Offset mapping

---

# Trainer

Purpose

Train tokenizer vocabulary.

Supported Algorithms

- BPE
- WordPiece
- Unigram

Responsibilities

- Vocabulary creation
- Merge generation
- Statistics
- Training pipeline

---

# Dataset

Purpose

Dataset processing.

Responsibilities

- Streaming
- Multi-file datasets
- Compression
- Sampling
- Statistics

---

# IO

Purpose

Input and output.

Responsibilities

- Reader
- Writer
- Binary files
- JSON
- Configuration

---

# CLI

Purpose

Command-line interface.

Responsibilities

- Train tokenizer
- Encode text
- Decode tokens
- Vocabulary inspection
- Statistics

---

# Testing

Every module has dedicated unit tests.

Current Status

- test_types
- test_error
- test_status
- test_result
- test_config
- test_version
- test_logger
- test_platform

---

# Build

```
mkdir build
cd build

cmake ..

cmake --build .

ctest
```

---

# License

MIT License
