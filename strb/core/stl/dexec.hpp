#pragma once

#ifdef NDEBUG
#define dexec(statement)
#else
#define dexec(statement) statement
#endif
