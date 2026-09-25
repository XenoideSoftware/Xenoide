
#ifndef XENOIDE_PREDEF_HPP
#define XENOIDE_PREDEF_HPP

#if defined(XENOIDE_DYNAMIC_LINK)
#if defined(_WINDOWS)
#error "Explicit dynamic linking on Windows isn't supported yet"
#define XENOIDE_API_EXPORT __declspec(dllexport)
#define XENOIDE_API_IMPORT __declspec(dllimport)
#else
#define XENOIDE_API_EXPORT
#define XENOIDE_API_IMPORT
#endif
#else
#define XENOIDE_API_EXPORT
#define XENOIDE_API_IMPORT
#endif

#if defined(XENOIDE_BUILD)
#define XENOIDE_API XENOIDE_API_EXPORT
#else
#define XENOIDE_API XENOIDE_API_IMPORT
#endif

#endif
