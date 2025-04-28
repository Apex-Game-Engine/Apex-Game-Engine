#pragma once

#define XSTR(x)					#x
#define STR(x)					XSTR(x)

#define XCONCAT(x1,x2)			x1##x2
#define CONCAT(x1,x2)			XCONCAT(x1,x2)
#define CONCAT3(x1,x2,x3)		CONCAT(CONCAT(x1,x2),x3)
#define CONCAT4(x1,x2,x3,x4)	CONCAT3(CONCAT(x1,x2),x3,x4)

#define NON_COPYABLE(type)		type(const type&) = delete;		type& operator=(const type&) = delete
#define NON_MOVABLE(type)		type(type&&) = delete;			type& operator=(type&&) = delete
