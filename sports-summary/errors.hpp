// Copyright 2020 Jeremy Sorensen
// This code is subject to the GPL3 license. See LICENSE file for details.
// Errors used by this software. Currently there isn't much error handling.
// Most of these erros are unrecoverable, such as library init errors,
// not being able to load local files, and bugs. One recoverable error
// would be the Download error, but currently it is not handled.
// There are helper functions to throw the errors.

#pragma once

#include <exception>
#include <iostream>

class FileError : public std::exception {
public:
	FileError(const char* message): exception(message) { }
};

class DownloadError : public std::exception {
public:
	DownloadError(const char* message) : exception(message) { }
};

class UiError : public std::exception {
public:
	UiError(const char* message) : exception(message) { }
};

class LibError : public std::exception {
public:
	LibError(const char* message): exception(message) { }
};

class LogicError : public std::exception {
public:
	LogicError(const char* message) : exception(message) { }
};

static void file_error(const char* message, const char* file, int line) {
	// replace with a real logger
	std::cerr << "File Error (" << file << ":" << line << ") - " << message << "\n";
	throw FileError(message);
}

static void download_error(const char* message, const char* file, int line) {
	// replace with a real logger
	std::cerr << "Download Error (" << file << ":" << line << ") - " << message << "\n";
	throw FileError(message);
}

static void ui_error(const char* message, const char* file, int line) {
	// replace with a real logger
	std::cerr << "UI Error (" << file << ":" << line << ") - " << message << "\n";
	throw FileError(message);
}

static void lib_error(const char* message, const char* file, int line) {
	// replace with a real logger
	std::cerr << "Library Error (" << file << ":" << line << ") - " << message << "\n";
	throw FileError(message);
}

static void logic_error(const char* message, const char* file, int line) {
	// replace with a real logger
	std::cerr << "!!BUG!! (" << file << ":" << line << ") - " << message << "\n";
	throw LogicError(message);
}
static void assert(bool condition, const char* message, const char* file, int line) {
	if (!condition) { logic_error(message, file, line); }
}


