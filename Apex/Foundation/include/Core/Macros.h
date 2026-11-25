#pragma once

// Config macros

#ifndef APEX_DEBUG
#define APEX_DEBUG (APEX_CONFIG_DEBUG || APEX_CONFIG_DEBUGGAME || APEX_CONFIG_TOOLDEBUG)
#endif

#if APEX_DEBUG
#  define DEBUG_ONLY(...)				__VA_ARGS__
#  define DEBUG_IF_ELSE(_if_,_else_)	_if_
#else
#  define DEBUG_ONLY(...)
#  define DEBUG_IF_ELSE(_if_,_else_)	_else_
#endif // APEX_DEBUG

#ifndef APEX_TOOl
#define APEX_TOOL (APEX_CONFIG_TOOLDEBUG || APEX_CONFIG_TOOLRELEASE)
#endif

#if APEX_TOOL
#  define TOOL_ONLY(...)				__VA_ARGS__
#  define TOOK_IF_ELSE(_if_,_else_)		_if_
#else
#  define TOOL_ONLY(...)
#  define TOOK_IF_ELSE(_if_,_else_)		_else_
#endif // APEX_TOOL

#define APEX_API     __cdecl
#if APEX_EXPORTS
#  define APEX_EXPORT  __declspec(dllexport)
#elif APEX_IMPORTS
#  define APEX_EXPORT  __declspec(dllimport)
#else
#  define APEX_EXPORT
#endif


// Utility macros

#define XSTR(x)								#x
#define STR(x)								XSTR(x)

#define XCONCAT(x1,x2)						x1##x2
#define CONCAT(x1,x2)						XCONCAT(x1,x2)
#define CONCAT3(x1,x2,x3)					CONCAT(CONCAT(x1,x2),x3)
#define CONCAT4(x1,x2,x3,x4)				CONCAT3(CONCAT(x1,x2),x3,x4)

#define NON_COPY_ASSIGNABLE(type)			type& operator=(const type&) = delete
#define DEFAULT_COPY_ASSIGNABLE(type)		type& operator=(const type&) = default
#define NON_COPYABLE(type)					type(const type&) = delete;		NON_COPY_ASSIGNABLE(type)
#define DEFAULT_COPYABLE(type)				type(const type&) = default;	DEFAULT_COPY_ASSIGNABLE(type)

#define NON_MOVE_ASSIGNABLE(type)			type& operator=(type&&) noexcept = delete
#define DEFAULT_MOVE_ASSIGNABLE(type)		type& operator=(type&&) noexcept = default
#define NON_MOVABLE(type)					type(type&&) = delete;			NON_MOVE_ASSIGNABLE(type)
#define DEFAULT_MOVABLE(type)				type(type&&) = default;			DEFAULT_MOVE_ASSIGNABLE(type)

#define apex_alloca(type, count)			static_cast<type*>(_alloca(sizeof(type) * (count)))
#define apex_alloca_zero(type, count)		static_cast<type*>(memset(_alloca(sizeof(type) * (count)), 0, sizeof(type) * (count)))
#define apex_alloca_new(type, count)		::new(_alloca(sizeof(type) * (count))) type[(count)]