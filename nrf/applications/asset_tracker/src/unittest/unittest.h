#ifndef _IOTEX_UNITTEST_H_
#define _IOTEX_UNITTEST_H_

#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define UNITTEST_ASSERT_EQ(a,b)	(assert((a) == (b)))
#define UNITTEST_ASSERT_NE(a,b)	(assert((a) != (b)))
#define UNITTEST_ASSERT_GT(a,b) (assert((a) > (b)))
#define UNITTEST_ASSERT_LT(a,b) (assert((a) < (b)))
#define UNITTEST_ASSERT_STR_EQ(a, b, l) (assert(memcmp(a, b, l) == 0))

#define UNITTEST_GET_ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define UNITTEST_PASS(x)	fprintf(stdout, "[\033[0;32mPass\033[0m] %s %s\n", __FILE__, x);
#define UNITTEST_FAIL(x)	fprintf(stdout, "[\033[0;31mPass\033[0m] %s %s\n", __FILE__, x);
#define UNITTEST_AUTO_PASS()	UNITTEST_PASS(__func__)
#define UNITTEST_AUTO_FAIL()	UNITTEST_FAIL(__func__)
#define UNITTEST_AUOT_FPASS()   UNITTEST_PASS(__FILE__)
#define UNITTEST_AUTO_TRUE(x)   do { if (x) {UNITTEST_PASS(__func__)} else {UNITTEST_FAIL(__func__)}} while (0)


#endif /* _IOTEX_UNITTEST_H_ */
