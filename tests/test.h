#ifndef _H_TEST
#define _H_TEST

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "zapp.h"

#define ASSERT_EQ(x, y) (assert((x) == (y)))
#define ASSERT_NEQ(x, y) (assert((x) != (y)))
#define ASSERT_LT(x, y) (assert((x) < (y)))
#define ASSERT_GT(x, y) (assert((x) > (y)))
#define ASSERT_LE(x, y) (assert((x) <= (y)))
#define ASSERT_GE(x, y) (assert((x) >= (y)))

#endif // _H_TEST
