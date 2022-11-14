#ifndef SOFTEQ_COMMON_SERIALIZATION_TESTS_TEST_STRUCTURE_H
#define SOFTEQ_COMMON_SERIALIZATION_TESTS_TEST_STRUCTURE_H

struct TestStructure
{
    int a;
    double b;
};

bool operator==(const TestStructure &lhs, const TestStructure &rhs);

#endif // SOFTEQ_COMMON_SERIALIZATION_TESTS_TEST_STRUCTURE_H
