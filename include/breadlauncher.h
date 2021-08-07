/**
 * \file breadlauncher.h
 * \author MobSlicer152 (brambleclaw1414@gmail.com)
 * \brief General definitions
 * \version 0.1
 * \date 02-08-2021
 * 
 * \copyright Copyright 2021 MobSlicer152
 */

#pragma once

#ifndef BREADLAUNCHER_H
#define BREADLAUNCHER_H 1

/* Given that the names inside these namespaces are also long this gets to be a bit much */
#include <chrono>
#include <filesystem>
namespace chrono = std::chrono;
using namespace std::literals;
namespace fs = std::filesystem;

/* Debug console log */
#ifndef BREAD_CONFIG_DIST
#include <fmt/color.h>
#include <fmt/chrono.h>
#include <fmt/format.h>
#define BREAD_LOG(...) fmt::print(__VA_ARGS__)
#define BREAD_LOG2(verbose, ...)                 \
	{                                        \
		if (verbose)                     \
			fmt::print(__VA_ARGS__); \
	}
#else
#define BREAD_LOG(...)
#define BREAD_LOG2(...)
#endif

/* OS name for checking the libraries in the version definition */
#if BREAD_WIN32
#define BREAD_OS_NAME "windows"
#elif BREAD_MACOS
#define BREAD_OS_NAME "osx"
#elif BREAD_LINUX
#define BREAD_OS_NAME "linux"
#endif

/* Convert time (imperfect for non-C++20 way, but good enough) */
#ifdef _MSC_VER /* Oddly enough, MSVC is the first implementation to support this so far */
#define CONVERT_TIME(time, source_clock, target_clock) (chrono::clock_cast<target_clock, source_clock>(time))
#else
#define CONVERT_TIME(time, source_clock, target_clock) (chrono::time_point_cast<target_clock::duration>(time - source_clock::now() + target_clock::now()))
#endif

#endif /* !BREADLAUNCHER_H */
